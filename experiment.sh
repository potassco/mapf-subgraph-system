#!/bin/bash

timeout=300

for instance in resources/scenarios/*
do
	for alg in asp-mks
	do
		for strategy in p
		do
			for path in biased
			do
				./build/subgraph_framework -i $instance -s $strategy -b $alg -p $path -t $timeout -a 5 -k 5
			done
		done
	done
done
