#!/bin/bash

timeout=100

for instance in resources/instances/*
do
	for strategy in b m p c
	do
		for solver in asp sat
		do
			./models/corridor_framework -i $instance -s $strategy -b $solver -t $timeout
		done
	done
done