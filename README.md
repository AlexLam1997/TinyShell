# TinyShell

Implementation of a linux command line shell from scratch in C using POSIX compliant threads. 
Supports piping (assuming a pre-existing FIFO pipe has been previously created) and input/output redirection.

## Creating a named pipe 
1. To create a FIFO in linux, run "mkfifo [pathname]"
2. You can now hand this FIFO in to the tinyShell as a parameter in order to use command piping in the shell. 

## Compiling and running 
1. To compile: run "gcc -o tshell main.c"
2. To run: run command "./tshell" or "./tshell [name of named pipe]" if you wish to use command piping in the shell 
