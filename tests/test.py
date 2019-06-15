#!/usr/bin/python3

import subprocess
import os
import sys
import collections

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
host_files = ["host1.txt", "host2.txt", "host3.txt", "host4.txt"]
hosts = []
def run():
    for host_file in host_files:
        my_port, neighbor_ports, input_string = read_host_file(procedure_directory + "/" + host_file)
        host = {'instance': subprocess.Popen([executable, my_port, neighbor_ports], stdout=subprocess.PIPE, stdin=subprocess.PIPE), \
                'input_string': input_string}
        hosts.append(host)

    for host in hosts:
        outs, err = host['instance'].communicate(host['input_string'].encode())
        #print(outs)
        if err != None:
            print(err)

def check():
    for host in hosts:
        host['instance'].wait()
        exit_status = host['instance'].poll()
        if exit_status != 0:
          print(exit_status)
          sys.exit(-1)

run()
check()
