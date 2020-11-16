#include <zconf.h>
#include <stdbool.h>
#include <sys/wait.h>

#define NOT_FOUND -1
#define END_OF_STRING (char *) NULL
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
error_handler(int status, char **msg) {
    if (status < 0) {
        printf("%s \n", msg);
        exit(1);
    }

}

int find_first_vertical_bar(args userInput) {
    for (int i = 0; i < userInput.count; i++)
        if (userInput.arglist[i][0] == '|')
            return i;
    return NOT_FOUND;
}

void execute(char **arglist) {
    const char *file = arglist[0];
    char **argv = arglist;
    int status = execvp(file, argv);
    char **msg;
    asprintf(&msg,"execution of %s failed", file);
    error_handler(status,msg);
    free(msg);
}

int get_ampersand_place(args *userInput) { return (*userInput).count - 1; }

args remove_apersand(args *userInput) {
    const int ampersand_place = get_ampersand_place(userInput);
    if ((*userInput).arglist[ampersand_place][0] == '&') {
        (*userInput).arglist[ampersand_place] = END_OF_STRING;
        userInput->count--;
    }
    return (*userInput);
}
void validate_pipe_close(int status)
{
    error_handler(status,"closing pipe failed");
}

void split_for_each_task(args *userInput, int bar_index) {
    pid_t fork_id = fork();
    error_handler(fork_id,"forking failed");
    int pipe_end[2];
    int create_pipe_status=pipe(pipe_end);
    error_handler(create_pipe_status,"pipe creation failed");
    int status;
    if (fork_id) {
        //parent
        userInput->count = bar_index;
        userInput->arglist[bar_index] = END_OF_STRING;
        status = dup2(pipe_end[1], STDOUT_FILENO);
        validate_pipe_close(close(pipe_end[0]));
        validate_pipe_close(close(pipe_end[1]));
    } else {
        //child
        userInput->count = userInput->count - bar_index;
        userInput->arglist = userInput->arglist + bar_index + 1;

        status = dup2(pipe_end[0], STDIN_FILENO);
        validate_pipe_close(close(pipe_end[0]));
        validate_pipe_close(close(pipe_end[1])); //keep
    }
    error_handler(status, "piping duping failed");
}

void bar_handler(args *userInput, int bar_index) {
    split_for_each_task(userInput, bar_index);
    execute(userInput->arglist);
}

void child_action(args userInput) {
    userInput = remove_apersand(&userInput);
    const int bar_location = find_first_vertical_bar(userInput);
    if (bar_location == -1) {
        execute(userInput.arglist);
    } else
        bar_handler(&userInput, bar_location);
}


void parent_action(args userInput, pid_t pid) {
    const int ampersand_place = get_ampersand_place(&userInput);
    const bool should_task_run_in_background = userInput.arglist[ampersand_place][0] == '&';
    if (!should_task_run_in_background) {
        int status=waitpid(pid, NULL, 0);
        error_handler(status,"wait failed");
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
    error_handler(fork_id,"forking failed");
    bool is_parent = fork_id;
    if (is_parent) {
        parent_action(user_input, fork_id);
    } else
        child_action(user_input);


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