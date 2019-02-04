#!/bin/bash
gcc ./inf136714_s.c -o ./server_program
gcc ./inf136714_c.c -o ./client_program
gnome-terminal -e "./server_program"
if [ "$#" -ne 0 ] ; then
for i in $(seq 1 $1); do gnome-terminal -e "./client_program"; done
fi
