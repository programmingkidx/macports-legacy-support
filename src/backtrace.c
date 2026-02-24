// Description: Implement backtrace functions

// Note: backtrace() and other functions below were added to Mac OS 10.5.

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Call this function to print a stacktrace at the call location
int backtrace(void** array, int size)
{
    unsigned int level = 0;
    void *saved_ra = __builtin_return_address(0);
    void **fp = (void **)__builtin_frame_address(0);
    void *saved_fp = __builtin_frame_address(0);

    level++;
    fp = saved_fp;
    while (fp) {
        saved_fp = *fp;
        fp = saved_fp;
        if (*fp == NULL)
            break;
        saved_ra = *(fp + 2);
        array[level-1] = saved_ra;
        level++;
        if (level >= size)
            break;
    }
    return level;
}

// Takes the array returned by backtrace() and returns a list of functions
char **backtrace_symbols(void** array, int size)
{
    Dl_info info;
    char **functions;
    const int max_function_name_size = 256;
    char function_name[max_function_name_size];
    char symbol_name[max_function_name_size];

    // Allocate array of char*
    functions = malloc(size * sizeof(char *));
    if (!functions) {
        printf("Error: could not allocate memory for functions array\n");
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        // Allocate space for each string
        functions[i] = malloc(max_function_name_size);
        if (!functions[i]) {
            printf("Error: could not allocate memory for function name\n");
            // Free previously allocated strings
            for (int j = 0; j < i; j++)
                free(functions[j]);
            free(functions);
            return NULL;
        }

        int ok = dladdr(array[i], &info);
        const char *name = (ok && info.dli_sname) ? info.dli_sname : "?";

        // Function name: Append "()" if we have a real function name
        if (ok && info.dli_sname) {
            snprintf(function_name, max_function_name_size, "%s()", name);
        } else {
            snprintf(function_name, max_function_name_size, "%s", name);
        }
        
        // Symbol name
        sprintf(symbol_name, "%s", (ok) ? info.dli_fname : "?");
        
        // Create the final output for this function
        sprintf(functions[i], "#%-4d %s in %s, pc = 0x%x", i, function_name, symbol_name, array[i]);
    }
    return functions;
}

// Takes the array returned by backtrace() and writes function names to the fd
void backtrace_symbols_fd(void** array, int size, int fd)
{
    Dl_info info;
    char **functions;
    const int max_function_name_size = 256;
    char function_name[max_function_name_size];
    char symbol_name[max_function_name_size];
    char buffer[1000];

    // Allocate array of char*
    functions = malloc(size * sizeof(char *));
    if (!functions) {
        printf("Error: could not allocate memory for functions array\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        // Allocate space for each string
        functions[i] = malloc(max_function_name_size);
        if (!functions[i]) {
            printf("Error: could not allocate memory for function name\n");
            // Free previously allocated strings
            for (int j = 0; j < i; j++)
                free(functions[j]);
            free(functions);
            return;
        }

        int ok = dladdr(array[i], &info);
        const char *name = (ok && info.dli_sname) ? info.dli_sname : "?";
        // Function name: Append "()" if we have a real function name
        if (ok && info.dli_sname) {
            snprintf(function_name, max_function_name_size, "%s()", name);
        } else {
            snprintf(function_name, max_function_name_size, "%s", name);
        }
        
        // Symbol name
        sprintf(symbol_name, "%s", (ok) ? info.dli_fname : "?");
        
        // Create the final output for this function
        sprintf(buffer, "#%-4d %s in %s, pc = 0x%x", i, function_name, symbol_name, array[i]);
        ok = write(fd, buffer, strlen(buffer));
        if (ok == -1) {
            fprintf(stderr, "Error: backtrace_symbols_fd() could not write to fd %d\n", fd);
        }
    }
}
