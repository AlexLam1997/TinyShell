#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/resource.h> 
#include <sys/time.h> 

#define DELIM "  \t\r\n\a"
#define BUFF_SIZE 64
#define MAX_HISTORY 100

static char *history[MAX_HISTORY];
static unsigned history_count = 0;

char *get_a_line()
{
    char *line = NULL;
    size_t bufferSize = 0;
    ssize_t nread;

    // Getline allocates a buffer an assigns it to line if line is null, *bufferSize is updated with size of buffer.
    // it is the responsibility of the caller to free this buffer in this case.
    nread = getline(&line, &bufferSize, stdin);

    return line;
}

char **string_split(char *line)
{
    // init to 64, realloc later
    int current_buffer_size = BUFF_SIZE;
    char **tokenArray = malloc(BUFF_SIZE * sizeof(char *));
    //TODO error check
    int position = 0;

    char *token = strtok(line, DELIM);
    while (token != NULL)
    {
        tokenArray[position] = token;
        position++;

        if (position >= BUFF_SIZE)
        {
            current_buffer_size += BUFF_SIZE;
            tokenArray = realloc(tokenArray, current_buffer_size * sizeof(char *));
            //TODO error check
        }
        token = strtok(NULL, DELIM);
    }
    tokenArray[position] = NULL;
    return tokenArray;
}

void print_history(void)
{
    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s", i, history[i]);
    }
}

void my_system(char *line)
{
    //History:
    if (history_count >= MAX_HISTORY)
    {
        free(history[0]);
        for (unsigned i = 1; i < MAX_HISTORY; i++)
        {
            history[i - 1] = history[i];
        }
        history[MAX_HISTORY - 1] = strdup(line);
    }
    else
    {
        history[history_count++] = strdup(line);
    }

    // Function arguments
    char **tokens = string_split(line);

    //change dir command
    if (strcmp(tokens[0], "cd") == 0)
    {
        chdir(tokens[1]);
    }
    else if (strcmp(tokens[0], "history") == 0)
    {
        print_history();
    }
    else if (strcmp(tokens[0], "limit") == 0)
    {
        struct rlimit rl; 
        getrlimit (RLIMIT_CPU, &rl); 
        printf("\n Default value is : %lld\n", (long long int)rl.rlim_cur); 
        rl.rlim_cur = atoi(tokens[1]); 
        
        // Now call setrlimit() to set the  
        // changed value. 
        setrlimit (RLIMIT_CPU, &rl); 
        
        // Again get the limit and check 
        getrlimit (RLIMIT_CPU, &rl); 
        
        printf("\n Default value now is : %lld\n", (long long int)rl.rlim_cur); 
    }
    else
    {
        // fork curent process to exexute a command
        pid_t pid = fork();
        if (pid == 0)
        {
            //In the child
            // if(execvp(tokens[0], tokens) == -1){
            if (execvp(tokens[0], tokens) == -1)
            {
                // the Exec command isnt supposed to return if no error happened
                perror("Something went wrong executing the child process program.");
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0)
        {
            // In the parent
            int status;
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        else
        {
            // Creation of the process failed
            perror("Something went wrong forking the process.");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
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