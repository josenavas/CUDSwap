#!/usr/bin/env python
from optparse import make_option, OptionParser
from numpy.linalg import solve
from numpy import array

__author__ = "Jose Antonio Navas Molina"
__copyright__ = ""
__credits__ = ["Jose Antonio Navas Molina"]
__license__ = "GPL"
__version__ = "0.1"
__maintainer__ = "Jose Antonio NAvas Molina"
__email__ = "josenavasmolina@gmail.com"
__status__ = "Development"


def create_dict(cpu, mem, stg, price):
    data = {}
    data['cpu'] = float(cpu)
    data['mem'] = float(mem)
    data['stg'] = float(stg)
    data['price'] = float(price)
    return data


def eq_file_parser(lines):
    list_config = []
    for line in lines:
        if line[0] == '#':
            continue
        cpu, mem, stg, price = line.split()
        data = create_dict(cpu, mem, stg, price)
        list_config.append(data)
    return list_config


def setup_system(conf1, conf2, conf3):
    coef = array([
        [conf1['cpu'], conf1['mem'], conf1['stg']],
        [conf2['cpu'], conf2['mem'], conf2['stg']],
        [conf3['cpu'], conf3['mem'], conf3['stg']]
        ])
    dep = array([conf1['price'], conf2['price'], conf3['price']])
    return coef, dep


options = [make_option('--eq_file', dest='eq_file', default=None)]


if __name__ == '__main__':
    parser = OptionParser(option_list=options)
    opts, args = parser.parse_args()

    if not opts.eq_file:
        parser.error("No equations file specified")

    eq_fp = opts.eq_file
    eqf = open(eq_fp, 'U')
    l_conf = eq_file_parser(eqf)

    num_results = 0
    cum_cpu = 0
    cum_mem = 0
    cum_stg = 0
    for i in range(len(l_conf)):
        for j in range(i + 1, len(l_conf)):
            for k in range(j + 1, len(l_conf)):
                coef, dep = setup_system(l_conf[i], l_conf[j], l_conf[k])
                try:
                    result = solve(coef, dep)
                    if result[0] >= 0 and result[1] >= 0 and result[2] >= 0:
                        num_results += 1
                        cum_cpu += result[0]
                        cum_mem += result[1]
                        cum_stg += result[2]
                except Exception, e:
                    pass
    cpu_unit = cum_cpu / num_results
    mem_unit = cum_mem / num_results
    stg_unit = cum_stg / num_results
    print cpu_unit, mem_unit, stg_unit
    eqf.close()
