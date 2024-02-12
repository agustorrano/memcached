#!/bin/bash

for ((i = 0; i < 1000; i++)); do
	./client PUT 1 1
	./client GET 1
done