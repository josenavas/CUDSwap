obj-m += mod_hack_brk.o
mod_hack_brk-objs := hack_brk.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
