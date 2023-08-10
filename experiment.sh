#!/bin/bash

timeout=300
strategy=b


for instance in resources/scenarios/*
do
	for alg in makespan soc-jump soc-iter
	do
		./build/subgraph_framework -i $instance -s $strategy -b $alg -t $timeout
	done
done
