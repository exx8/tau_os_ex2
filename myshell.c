#include <zconf.h>
#include <stdbool.h>
#include <sys/wait.h>

#define NOT_FOUND -1
struct _args {
    int count;
    char **arglist;
} typedef args;

/**
 *
 * @param count num of elements
 * @param arglist the array of char to look in
 * @return the index of the first | or NOT_FOUND
 */
int find_first_vertical_bar(args userInput) {
    for (int i = 0; i < userInput.count; i++)
        if (userInput.arglist[i][0] == '|')
            return i;
    return NOT_FOUND;
}

void child_action(args userInput) {
    const int bar_location = find_first_vertical_bar(userInput);
    if(bar_location==-1) {
        const char* file=userInput.arglist[0];
        char **argv=userInput.arglist;
        execvp(file,argv);
    }
}

void parent_action(args userInput, pid_t pid) {
    const int ampersand_place = userInput.count - 1;
    const bool should_task_run_in_background = userInput.arglist[ampersand_place][0] == '&';
    if (!should_task_run_in_background) {
        waitpid(pid, NULL, 0);
    }

}

args convert_user_input_to_structure(int count, char **arglist) {
    args userInput;
    userInput.count = count;
    userInput.arglist = arglist;
    return userInput;
}

// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise
int process_arglist(int count, char **arglist) {

    args user_input = convert_user_input_to_structure(count, arglist);
    pid_t fork_id = fork();
    bool is_parent = fork_id;
    if (is_parent) {
        parent_action(user_input, fork_id);
    } else
        child_action(user_input);


    //printf("%d \n", fork_id);
    return is_parent;
}

//@todo remove reaped
void zombie_reaper() {
    printf("reaped");
}

/**
 * sets handler (empty func) as the zombie_reaper to prevent zombies
 */
void prepare_sigint() {
    sigaction(SIGINT, &(struct sigaction) {zombie_reaper}, NULL);

}

// prepare and finalize calls for initialization and destruction of anything required
int prepare(void) {
    prepare_sigint();
    return 0;
}

int finalize(void) {
    return 0;
}