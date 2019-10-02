#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h> 

#include "input_processing.c"
#include "history.c"
#include "execution.c"
#include "get_line.c"

void my_system(char *line)
{
    write_history(line);

    execute_command(line);
}

void sig_int_handler(int num){
    char reply[5];
    printf("\nWould you really like to terminate the program? (y/n) \n");
    fflush(stdout); 
    scanf("%s", reply);
    if(strchr(reply, 'y')){
        exit(EXIT_SUCCESS);
    }
}

void sig_tstp_handler(int num){
    //ignore signal
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_int_handler);
    signal(SIGTSTP, sig_tstp_handler);

    if (argv[1])
    {
        FIFO_NAME = argv[1];
    }

    char *line;
    while (1)
    {
        line = get_a_line();
        if (strlen(line) > 1)
        {
            my_system(line);
        }
    }
    return 0;
}