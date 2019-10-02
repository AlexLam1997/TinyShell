#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shared.h"
#define DELIM "  \t\r\n\a"
#define PIPE_DELIM "|"

typedef enum
{
    false,
    true
} bool;

bool is_pipe(char *line)
{
    bool pipe = false;
    int i = 0;
    while (i < strlen(line) && !pipe)
    {
        pipe = strchr(PIPE_DELIM, line[i]) != NULL;
        i++;
    }
    return pipe;
}

void execute_command(char *line)
{
    char **tokens;
    bool isPipe = is_pipe(line);
    if(isPipe){
        if (FIFO_NAME == NULL)
        {
            printf("Unable to use pipes, no FIFO provided. \n");
            exit(EXIT_FAILURE);
        }
        tokens = string_split(line, PIPE_DELIM);
    }else{
        tokens = string_split(line, DELIM);
    }

    if (strcmp(tokens[0], "chdir") == 0)
    {
        //tokens 1 wil contain the path of the file to cd into
        chdir(tokens[1]);
    }
    else if (strcmp(tokens[0], "history") == 0)
    {
        print_history();
    }
    else if (strcmp(tokens[0], "limit") == 0)
    {
        struct rlimit rl;
        getrlimit(RLIMIT_CPU, &rl);
        printf("\n Default value is : %lld\n", (long long int)rl.rlim_cur);
        rl.rlim_cur = atoi(tokens[1]);
        setrlimit(RLIMIT_CPU, &rl);
        getrlimit(RLIMIT_CPU, &rl);
        printf("\n Default value now is : %lld\n", (long long int)rl.rlim_cur);
    }
    else
    {
        // fork curent process to execute a command
        pid_t pid = fork();

        if (pid < 0)
        {
            // Creation of the process failed
            perror("Error while forking");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            //In the child
            if(isPipe){
                // Piping command
                // Close the stdout, reroute to FIFO
                close(1);
                int fifo = open(FIFO_NAME, O_WRONLY);
                dup2(fifo, 1);
                if(fifo < 0){
                    perror("Invalid command");
                    exit(EXIT_FAILURE); 
                }

                // If this is a pipe command, then the tokens array has seperated the string using PIPE_DELIM
                // Split the first command of the pipe again
                char** pipe_tokens = string_split(tokens[0], DELIM);

                // run the first command, the output has been redirected to the fifo
                if (execvp(pipe_tokens[0], pipe_tokens) == -1)
                {
                    // the Exec command isnt supposed to return if no error happened
                    perror("Invalid command");
                    exit(EXIT_FAILURE);
                }
                close(fifo);
            }else{
                // there is no pipe operator in this command, the tokens array has seperated the string using DELIM
                if (execvp(tokens[0], tokens) == -1)
                {
                    // the Exec command isnt supposed to return if no error happened
                    perror("Invalid command");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (pid > 0)
        {
            // In the parent
            // Now for the 2nd process of the pipe
            if(isPipe){
                pid_t pid2 = fork();
                if(pid2 == 0){
                    // Pull from FIFO instead of stdin
                    // Close the stdin, reroute to FIFO
                    close(0);
                    int fifo = open(FIFO_NAME, O_RDONLY);
                    dup2(fifo, 0);
                    // If this is a pipe command, then the tokens array has seperated the string using PIPE_DELIM
                    // Split the first command of the pipe again
                    char** pipe_tokens = string_split(tokens[1], DELIM);

                    // run the first command, the output has been redirected to the fifo
                    if (execvp(pipe_tokens[0], pipe_tokens) == -1)
                    {
                        // the Exec command isnt supposed to return if no error happened
                        perror("Invalid command");
                        exit(EXIT_FAILURE);
                    }
                    close(fifo);
                }else if (pid2<0){
                    perror("Error while forking");
                    exit(EXIT_FAILURE);
                }
            }
            // wait for execution of task 1 to end before accepting another
            int status;
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}