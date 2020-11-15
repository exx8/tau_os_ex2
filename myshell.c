#include <zconf.h>
#include <wait.h>
#include <stdbool.h>
#define NOT_FOUND -1
struct _args{
    int count;
    char **arglist;
} typedef args;
/**
 *
 * @param count num of elements
 * @param arglist the array of char to look in
 * @return the index of the first | or NOT_FOUND
 */
int find_first_vertical_bar(args userInput)
{
    for(int i=0;i<userInput.count;i++)
        if(userInput.arglist[i]=='|')
            return i;
        return NOT_FOUND;
}
void child_action(args userInput)
{

}
void parent_action(args userInput)
{

}
args convert_user_input_to_structure(int count, char **arglist)
{
    args userInput;
    userInput.count=count;
    userInput.arglist=arglist;
    return userInput;
}
// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char **arglist) {

    args user_input= convert_user_input_to_structure(count, arglist);
    pid_t fork_id=fork();
    bool is_parent=fork_id;
    if(is_parent)
    {
        parent_action(user_input);
    }

    else
        child_action(user_input);
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