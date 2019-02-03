#!/bin/bash
gcc ./server.c -o ./server_program
gcc ./client.c -o ./client_program
x-terminal-emulator -e "./server_program"
if [ "$#" -ne 0 ] ; then
for i in $(seq 1 $1); do x-terminal-emulator -e "./client_program"; done
fi
