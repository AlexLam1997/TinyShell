#define BUFF_SIZE 64

char** string_split(char* line, char* delim)
{
    // init to 64, realloc later
    int current_buffer_size = BUFF_SIZE;
    char **tokenArray = malloc(BUFF_SIZE * sizeof(char *));
    //TODO error check
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
            //TODO error check
        }
        token = strtok(NULL, delim);
    }
    tokenArray[position] = NULL;
    return tokenArray;
}