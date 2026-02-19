// Description: implement the function aligned_alloc()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Carbon/Carbon.h>

int (*debug_print)(const char * format, ...);

// prints anything from being printed
int do_nothing( const char * format, ...)
{
    return 0; 
}

// Sets debug printing
void init()
{
    // If the enviromental variable MallocDebugReport is set to "stderr", debug printing will happen
    char *value = getenv("MallocDebugReport");
    if (value != NULL && strcmp(value, "stderr") == 0)
    {
        //printf("Note: debug printing enabled\n");
        debug_print = printf;
    } 
    else
    {
        debug_print = do_nothing;
    }
}

void *aligned_alloc(size_t requested_alignment, size_t size)
{
    // only need to run init once in a program's life 
    static bool init_ran = false;
    if (init_ran == false)
    {
        init_ran = true;
        init();
    }
     // check that alignment is a power of 2
    if ((requested_alignment != 0) && (requested_alignment & (requested_alignment - 1)))
    {
        debug_print("Failure reason: alignment value is not a power of 2\n");
        errno = EINVAL;
        return NULL;
    }
    
    // check that size is an integral multiple of alignment
    if (size % requested_alignment != 0)
    {
        debug_print("Failure reason: size %% requested_alignment != 0\n");
        errno = EINVAL;
        return NULL;
    }
    
    // check that alignment is at least as large as sizeof(void *)
    if (requested_alignment < sizeof(void *))
    {
        debug_print("Failure reason: requested_alignment < sizeof(void *)\n");
        errno = EINVAL;
        return NULL;
    }
    
    void *ptr;
    ptr = malloc(size);
    if (ptr == NULL)
    {
        debug_print("Failure reason: ptr == NULL\n");
        errno = ENOMEM;
    }
    
    return ptr;
}
