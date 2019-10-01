#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>

#include "input_processing.c"
#include "history.c"
#include "execution.c"
#include "get_line.c"

static char *FIFO_NAME;

void my_system(char *line)
{
    // if (line[0] == EOF){
    //     exit(EXIT_SUCCESS);0515102
    // } //TODO
    write_history(line);

    execute_command(line);
}

int main(int argc, char *argv[])
{
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