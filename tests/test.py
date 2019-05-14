#!/usr/bin/python3

import subprocess
import os

def read_host_file(filename):
    fp = open(filename, 'r')
    first_line = True
    input_str = ""
    for line in fp:
       if first_line:
          l = line.split('|')
          my_port = l[0]
          neighbor_ports = l[1]
          first_line = False
       else:
          input_str += line
    fp.close()

    return my_port, neighbor_ports, input_str

executable = "../yela"
procedure_directory = "procedures"
hosts = []
def run():
    input_strs = []
    host_files = os.listdir(procedure_directory)
    for host_file in host_files:
        my_port, neighbor_ports, input_str = read_host_file(procedure_directory + "/" + host_file)
        host = subprocess.Popen([executable, my_port, neighbor_ports], stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        hosts.append(host)
        input_strs.append(input_str)

    for host, input_str in zip(hosts, input_strs):
        host.communicate(input_str.encode())

def check():
    for host in hosts:
        host.wait()
        assert(host.poll() == 0)

run()
check()
