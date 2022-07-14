#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define READ_END 0
#define WRITE_END 1


// author @ Cavin Gada


int main(int argc, char *argv[]) {

    // keep track of children in an array
    pid_t ret[2];

    // set up both pipes
    int pfind_sort[2];
    int sort_p[2];

    pipe(pfind_sort);
    pipe(sort_p);

    // fork first child
    ret[0] = fork();

    // if the fork of the first child is negative, the fork failed 
    if (ret[0] < 0 ) {
        fprintf(stderr, "Error: first fork failed. %s.\n", strerror(errno));
        return 1;
    }

    
    // if child:
    if (ret[0] == 0) {

        close(pfind_sort[0]); // close the read end of first pipe
        if (dup2(pfind_sort[1], 1) == -1) {
            fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
        }; // write to write end of first pipe

        // CLOSE PIPE 2. we aren't using it. 
        close(sort_p[0]);
        close (sort_p[1]); 

        // got help from here: https://stackoverflow.com/questions/55743496/difference-between-exec-execvp-execl-execv
        /* "In your case the arguments will be entered by the user and you have to construct a vector of arguments at run time, 
        so you should use one of the execv* functions.""
        */
        int e = execv("pfind", argv);

        if (e == -1) {
            fprintf(stderr, "Error: pfind failed. %s.\n", strerror(errno));
            return 1;
        }

    }

    // fork second child
    ret[1] = fork();

    // if the value is negative, we have an error in fork
    if (ret[1] < 0 ) {
        fprintf(stderr, "Error: second fork failed. %s.\n", strerror(errno));
        return 1;
    }
    // if child:
    if (ret[1] == 0) {

        close(pfind_sort[1]); // close the write end of the first pipe
        if (dup2(pfind_sort[0], 0) == -1) {
            fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
        }
     // read from the read end of the first pipe

        // help from here on which exec to use: https://stackoverflow.com/questions/8465026/using-exec-to-sort-a-text-file-in-c
        // syntax help from here: https://stackoverflow.com/questions/21558937/i-do-not-understand-how-execlp-works-in-linux

        close(sort_p[0]); // close the read end of the second pipe

        if (dup2(sort_p[1],1) == -1) {
            fprintf(stderr, "Error: dup2 failed. %s.\n", strerror(errno));
        } // write to the write end of the second pipe

        int e2 = execlp("sort", "sort", (char *)NULL); // execute the sort

        if (e2 == -1) {
            fprintf(stderr, "Error: sort failed. %s.\n",strerror(errno));
            return 1;
        }

    }

    // parent! 

    /* Your code here */
    close(pfind_sort[0]); // close pipe 1 in, we dont need it
    close(pfind_sort[1]); // close pipe 1 out, we dont need it
    close(sort_p[1]);    // close pipe 2 out, we only need to read from it. 

    // wait for the poor children to die, if an error occurs in wait fail. 
     while(wait(NULL)>0){
        if (wait(NULL) == -1) {
            fprintf(stderr, "Error: wait failed. %s.\n", strerror(errno));
            return 1;
        }
    }

    char c; // keep track of current char
    int count = 0; // keep track of number of files found
    int bytes;     // keep track of the # of bytes read (which should be 1 tbh)

    while(true) {           
        bytes = read(sort_p[0], &c, 1);     // read from pipe 2's in and dispose of the value in c
        if (bytes == 0) {                   // if we're done lets just exit (no bytes read)
        printf("Matches found: %d\n", count);     // print the total number of matches found
            break;
        }
        if (bytes == -1) {                  // if an error is found while reading show it!
            fprintf(stderr,"Error: read failed. %s.\n", strerror(errno));
            return 1;
        }
        printf("%c", c);                    // print the current character
        if (c == '\n') {                    // if the current character is a new line, we know we found a match
            count++;                        
        }
    }
    close(sort_p[0]);                       // close the pipe 2's read. (last fd to close)
    

    return 0;                               // successful program baby
}