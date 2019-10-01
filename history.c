#define MAX_HISTORY 100
static char *history[MAX_HISTORY];
static unsigned history_count = 0;

void print_history(void)
{
    for (int i = 0; i < history_count; i++)
    {
        printf("%d %s", i, history[i]);
    }
}

void write_history(char* line){
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