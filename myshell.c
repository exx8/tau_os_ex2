#include <zconf.h>
#include <wait.h>
#include <stdbool.h>
#define NOT_FOUND -1

/**
 *
 * @param count num of elements
 * @param arglist the array of char to look in
 * @return the index of the first | or NOT_FOUND
 */
int find_first_vertical_bar(int count, char **arglist)
{
    for(int i=0;i<count;i++)
        if(arglist[i]=='|')
            return i;
        return NOT_FOUND;
}
// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char **arglist) {
    pid_t fork_id=fork();
    bool is_parent=fork_id;
    printf("%d \n", fork_id);
    return is_parent;
}

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void) {
    return 0;
}

int finalize(void) {
    return 0;
}