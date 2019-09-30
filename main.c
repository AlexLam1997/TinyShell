#include <stdio.h>
#include <string.h>

char* get_a_line(){
    char* line = NULL;
    size_t bufferSize = 0;
    ssize_t nread;

    // Getline allocates a buffer an assigns it to line if line is null, *bufferSize is updated with size of buffer. 
    // it is the responsibility of the caller to free this buffer in this case. 
    nread = getline(&line, &bufferSize, stdin);

    return line;
}

void my_system(char* line){
    printf("From my_system: %s", line);
}

int main(int argc, char* arv[] ) {
    char* line;
    while(1){
        line = get_a_line();
        if(strlen(line) > 1){
            my_system(line);
        }
    }
    return 0;
}