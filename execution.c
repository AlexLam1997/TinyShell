#define DELIM "  \t\r\n\a"
#define PIPE_DELIM "<>|"

// ls -l | wc
// ls -l    wc
void execute_pipe(char * line){
    char** tokens = string_split(line, PIPE_DELIM);
    close(stdout); 
    printf("piped");
}

void execute_regular(char* line){
    char** tokens = string_split(line, DELIM);
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
        // fork curent process to exexute a command
        pid_t pid = fork();
        if (pid == 0)
        {
            //In the child
            // if(execvp(tokens[0], tokens) == -1){
            if (execvp(tokens[0], tokens) == -1)
            {
                // the Exec command isnt supposed to return if no error happened
                perror("Invalid command");
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

void execute_command(char* line)
{
    //Line contains a pipe argument
    if (strstr(line, PIPE_DELIM) != NULL){
        execute_pipe(line);
    }else{
        // Regular command without pipes
        execute_regular(line);
    }
}