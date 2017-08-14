#!/bin/bash

dmesg | grep AVG | tail -79 | awk -F":" '{print $2 $3 $4}' | awk '{printf("%d\t%d\n",$1, $7)}'
