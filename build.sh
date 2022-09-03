#!/bin/bash
cc -std=c99 -Wall -O3 main.c mpc.c -ledit -lm -o lisp.o
