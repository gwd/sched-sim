#!/bin/bash
sched=credit01
workload=N2s2
file_base=${sched}.${workload}
./simulator --scheduler=${sched} --workload=${workload} > /tmp/${file_base}.all
for C in credit runtime; do 
    for I in 0 1 2 3 4 ; do 
	grep -a "^$C v$I" /tmp/${file_base}.all | awk '{print $3 " " $4;}' > /tmp/${file_base}.$C.$I.csv 
    done
    ygraph -c /tmp/${file_base}.$C.*.csv & 
done
tail -25 /tmp/$file_base.all
