#!/bin/bash

timeout=100

for instance in resources/instances/scenarios/*
do
	for strategy in b m p c
	do
		for solver in asp sat
		do
			./build/bin/corridor_framework -i $instance -s $strategy -b $solver -t $timeout
		done
	done
done
