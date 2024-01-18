#!/bin/bash

timeout=100

for instance in resources/scenarios/*
do
	for alg in sat-mks sat-soc
	do
		for strategy in b m p c
		do
			for path in biased random cross time
			do
				./build/subgraph_framework -i $instance -s $strategy -b $alg -p $path -t $timeout -a 5 -k 5
			done
		done
	done
done
