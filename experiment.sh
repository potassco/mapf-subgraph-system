#!/bin/bash

timeout=100

for instance in resources/scenarios/*
do
	for alg in asp-mks asp-soc asp-inc-mks asp-inc-soc
	do
		for strategy in b m p c
		do
			for path in biased
			do
				./build/subgraph_framework -i $instance -s $strategy -b $alg -p $path -t $timeout -a 5 -k 5
			done
		done
	done
done
