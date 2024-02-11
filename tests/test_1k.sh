#!/bin/bash

K=$1
N=0
while :; do
	echo "PUT $K $N"
	# sleep 0.0001
	echo "GET $K"
	N=$((N+1))
done | nc localhost 8888
