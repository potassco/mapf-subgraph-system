#!/bin/bash

timeout=300

for instance in resources/instances/scenarios/*
do
	for strategy in b m p c
	do
		for path in single all random diverse
		do
			./build/bin/subgraph_framework -i $instance -s $strategy -b asp-teg -t $timeout -p $path
		done
	done
done
