#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h> 
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_HISTORY 100
#define BUFF_SIZE 64
#define DELIM "  \t\r\n\a"
#define PIPE_DELIM "|"

// Static variables and definitions
typedef enum
{
    false,
    true
} bool;

//Holding the name of the FIFO for use in piping
static char* FIFO_NAME = NULL;

// Array holding the history commands
static char *history[MAX_HISTORY];
static unsigned history_count = 0;

// Functions definitions
void sig_int_handler(int num);
void sig_tstp_handler(int num);
char* get_a_line();
void my_system(char *line);
char** string_split(char* line, char* delim);
void write_history(char* line);
void print_history(void);
bool is_pipe(char *line);
void execute_command(char *line);

//Function bodies:
int main(int argc, char *argv[])
{
    // Handle both signals
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

void sig_int_handler(int num){
    char reply[1];

    // Reset the handler so it can handle the next signal too
    signal(SIGINT, sig_int_handler);

    printf("\nWould you really like to terminate the program? (y/n) \n");
    char f = getchar();

    if(f == 'y'){
        exit(EXIT_SUCCESS);
    }
}

void sig_tstp_handler(int num){
    //ignore signal
}

char* get_a_line()
{
    char *line = NULL;
    size_t bufferSize = 0;
    ssize_t nread;

    // Getline allocates a buffer an assigns it to line if line is null, *bufferSize is updated with size of buffer.
    // it is the responsibility of the caller to free this buffer in this case.
    nread = getline(&line, &bufferSize, stdin);

    return line;
}

void my_system(char *line)
{
    write_history(line);

    execute_command(line);
}

char** string_split(char* line, char* delim)
{
    // init to 64, realloc later
    int current_buffer_size = BUFF_SIZE;
    char **tokenArray = malloc(BUFF_SIZE * sizeof(char *));

    if(tokenArray == NULL){
        printf("Error allocating memory");
        exit(0);
    }

    int position = 0;

    char *token = strtok(line, delim);
    while (token != NULL)
    {
        tokenArray[position] = token;
        position++;

        if (position >= BUFF_SIZE)
        {
            current_buffer_size += BUFF_SIZE;
            tokenArray = realloc(tokenArray, current_buffer_size * sizeof(char *));
            if(tokenArray == NULL){
                printf("Error allocating memory");
                exit(0);
            }
        }
        token = strtok(NULL, delim);
    }
    tokenArray[position] = NULL;
    return tokenArray;
}

void write_history(char* line)
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
}

void print_history(void)
{
    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s", i, history[i]);
    }
}

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

