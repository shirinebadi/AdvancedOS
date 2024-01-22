#!/usr/bin/env python3

import sys

pid = int(sys.argv[1])
search_string = str(sys.argv[2])
write_string = str(sys.argv[3])

maps_file = open('/proc/{}/maps'.format(pid), 'r')

for line in maps_file:
    sline = line.split(' ')

    if sline[-1][:-1] != "[heap]":
        continue

    addrress = sline[0]
    addrress = addrress.split("-")
    start = int(addrress[0], 16)
    end = int(addrress[1], 16)

    try:
        mem_file = open('/proc/{}/mem'.format(pid), 'rb+', 0)
    except:
        print("Couldn't open the file")
    
    mem_file.seek(start)
    heap = mem_file.read(end - start)

    i = heap.index(bytes(search_string, "ASCII"))
    mem_file.seek(start + i)
    mem_file.write(bytes(write_string, "ASCII"))

    # close files
    maps_file.close()
    mem_file.close()

    break

    


