#!/bin/bash

timeout=300

for instance in resources/instances/scenarios/*
do
	for strategy in b m p c
	do
		for solver in asp asp-teg sat
		do
			./build/bin/subgraph_framework -i $instance -s $strategy -b $solver -t $timeout
		done
	done
done
