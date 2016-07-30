#include<linux/module.h>
#include<linux/version.h>
#include<linux/kernel.h>
#include<linux/kprobes.h>
#include<linux/kallsyms.h>
#include<linux/mm.h>
#include<linux/swap.h>
#include<linux/mmzone.h>
#include<linux/security.h>
#include<linux/proc_fs.h>
#include<linux/mutex.h>

#define MAX_PIDS 20

static DEFINE_MUTEX(CUDSwap_mutex);

static int pids[MAX_PIDS];
static int i_pid = -1;

static struct jprobe jp;

static struct proc_dir_entry *proc_entry;

static long threshold = 0;
static int swap_creator_pid = 0;

module_param(threshold, long, 0);
MODULE_PARM_DESC(threshold, "The system memory threshold");

module_param(swap_creator_pid, int, 0);
MODULE_PARM_DESC(swap_creator_pid, "The pid of the process which will create the swap space");

void send_signal(int signal, int pid){
	int ret;
	struct siginfo info;
	struct pid *ret_pid;
	struct task_struct *t;
	// Get the memory needed for the siginfo structure
	memset(&info, 0, sizeof(struct siginfo));
	// Set values in the siginfo structure
	info.si_signo = signal;
	info.si_code = SI_KERNEL;
	info.si_int = signal;
	// Find the task struct of the process 'pid'
	rcu_read_lock();
	ret_pid = find_get_pid(pid);
	t = pid_task(ret_pid, PIDTYPE_PID);
	rcu_read_unlock();
	if(t == NULL){
		printk("CUDSwap: process %d do not exists\n", pid);
		return;
	}
	// Send the signal
	ret = send_sig_info(signal, &info, t);
	if(ret < 0){
		printk("CUDSwap: signal to process %d failed\n", pid);
		return;
	}
}

void create_swap_space(void){
	// Check that current process is not the swap creator one
	// Neither its child (mkswap)
	if(current->pid == swap_creator_pid)
		return;
	if(current->parent != NULL)
		if(current->parent->pid == swap_creator_pid)
			return;

	// Wake up swap creator process
	send_signal(SIGUSR1, swap_creator_pid);

	// If there is enough space in the pids array
	// store current pid and send a signal to
	// himself to stop the process
	// don't do anything otherwise
	mutex_lock_interruptible(&CUDSwap_mutex);
	if(i_pid < MAX_PIDS - 1){
		i_pid++;
		pids[i_pid] = current->pid;
		mutex_unlock(&CUDSwap_mutex);
		send_signal(SIGSTOP, current->pid);
	}else{
		mutex_unlock(&CUDSwap_mutex);
	}

}

struct pglist_data *get_next_pgdat(struct pglist_data *pgdat){
	// Code from mm/mmzone.c - next_online_pgdat
	int nid = next_online_node(pgdat->node_id);
	if(nid == MAX_NUMNODES)
		return NULL;
	return NODE_DATA(nid);
}

unsigned long compute_reserved_pages(void){
	// Code from mm/page_alloc.c - calculate_totalreserve_pages
	struct pglist_data *pgdat;
	unsigned long reserved = 0;
	enum zone_type i, j;
	for(pgdat = NODE_DATA(first_online_node); pgdat; pgdat = get_next_pgdat(pgdat)){
		for(i = 0; i < MAX_NR_ZONES; i++){
			struct zone *zone = pgdat->node_zones + i;
			unsigned long max = 0;
			for(j = i; j < MAX_NR_ZONES; j++){
				if(zone->lowmem_reserve[j] > max)
					max = zone->lowmem_reserve[j];
			}
			reserved += max;
		}
	}
	return reserved;
}

int check_enough_memory(unsigned long addr, unsigned long len){
	unsigned long free, reserved;
	struct sysinfo mem_info;
	long pages;
	len = PAGE_ALIGN(len);
	if(!len){
		// No changes in memory usage
		return 0;
	}
	// Number of pages needed by the process
	pages = len >> PAGE_SHIFT;
	// Compute the number of free pages in the system
	free = global_page_state(NR_FREE_PAGES);
	free += global_page_state(NR_FILE_PAGES);
	free -= global_page_state(NR_SHMEM);
	si_swapinfo(&mem_info);
	si_meminfo(&mem_info);
	free += mem_info.freeswap;
	free += global_page_state(NR_SLAB_RECLAIMABLE);
	reserved = compute_reserved_pages();
	// Return -1 if the unmber of free pages is less than the number
	// system reserved pages
	if(free <= reserved){
		return -1;
	}	
	else{
		free -= reserved;
	}
	free -= free / 32;
	free -= pages;
	// Check if the number of free pages after allocation
	// will be greater than the treshold
	if(free > threshold){
		return 0;
	}
	// There isn't enough pages - return -1
	return -1;
}

unsigned long my_brk_handler(unsigned long addr, unsigned long len){
	if(check_enough_memory(addr, len)){
		create_swap_space();
	}
	jprobe_return();
	return 0;
}

int cudswap_write(struct file* filp, const char __user *buff, unsigned long len, void *data){
	char c;
	if(copy_from_user(&c, buff, 1)){
		return -EFAULT;
	}
	if(c != '1')
		return -1;
	// The swap space has been created
	mutex_lock_interruptible(&CUDSwap_mutex);
	while(i_pid >= 0){
		send_signal(SIGCONT, pids[i_pid]);
		--i_pid;
	}
	mutex_unlock(&CUDSwap_mutex);
	return len;
}

int my_init(void){
	int ret;

	if(threshold == 0){
		printk("CUDSwap: registration failed - didn't get the system memory threshold\n");
		return -1;
	}

	if(swap_creator_pid == 0){
		printk("CUDSwap: registration failed - didn't get the swap creator pid\n");
		return -1;
	}

	jp.kp.addr = (kprobe_opcode_t *) kallsyms_lookup_name("do_brk");
	jp.entry = (kprobe_opcode_t *) my_brk_handler;
	ret = register_jprobe(&jp);
	if(ret < 0) {
		printk("CUDSwap: registration failed: %d\n", ret);
		return -1;
	}

	proc_entry = create_proc_entry("CUDSwap", 0200, NULL);
	if(proc_entry == NULL){
		printk("CUDSwap: Proc entry creation failed\n");
		return -1;
	}
	proc_entry->write_proc = cudswap_write;

	threshold += 2500; // Increase the threshold to allow the creation of mkswap

	printk("CUDSwap: jprobe registered successfully. swap creator pid: %d\n", swap_creator_pid);
	return 0;
}

void my_exit(void){
	unregister_jprobe(&jp);
	remove_proc_entry("CUDSwap", NULL);
	printk("CUDSwap: jprobe unregistered successfully.\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jose Antonio Navas Molina");
MODULE_DESCRIPTION("CUDSwap");
MODULE_LICENSE("GPL");
