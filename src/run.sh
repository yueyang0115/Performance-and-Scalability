#!/bin/bash
make clean
make
taskset --cpu-list 0-1 ./server 0
