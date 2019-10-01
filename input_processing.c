#define DELIM "  \t\r\n\a"
#define BUFF_SIZE 64

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