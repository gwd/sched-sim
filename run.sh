#!/bin/bash
./simulator --scheduler=credit01 --workload=Sx3 > /tmp/1
for C in credit runtime; do 
    for I in 0 1 2 ; do 
	grep -a "^$C v$I" /tmp/1 | awk '{print $3 " " $4;}' > /tmp/1.$C.$I.csv 
    done
    ygraph -c /tmp/1.$C.*.csv & 
done
