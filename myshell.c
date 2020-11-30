#include <zconf.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NOT_FOUND -1
#define END_OF_STRING (char *) NULL
/**
 * a structure for describing arguments
 */
struct _args {
    int count;
    char **arglist;
} typedef args;

/**
 * generic error handler for function of the project
 * @param status - the status which returns from the function
 * @param msg - a custom message to be printed into stderr
 */
void error_handler(int status, char *msg) {
    if (status < 0) {
        fprintf(stderr, "%s \n", msg);
        fprintf(stderr, "%s \n", strerror(errno));
        exit(1);
    }

}
/**
 * sets handler (empty func) as the zombie_reaper to prevent zombies
 */
void prepare_sigint(__sighandler_t sighandler) {
    struct sigaction cntrl_c_catcher;
    memset(&cntrl_c_catcher,0,sizeof(cntrl_c_catcher));
    cntrl_c_catcher.sa_flags = SA_RESTART;
    cntrl_c_catcher.sa_handler =sighandler ;
    int status = sigaction(SIGINT, &cntrl_c_catcher, NULL);
    error_handler(status, "couldn't set zombie handler");

}

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
/**
 * a convenient execution wrapper function
 * @param arglist- list of args to be executed
 */

void execute(char **arglist) {
    const char *file = arglist[0];
    char **argv = arglist;
    int status = execvp(file, argv);
    char *msg="execution  failed";
    error_handler(status, msg);
}
/**
 * returns the ampersand place
 * @param userInput -args structure
 * @return the correct place
 */
int get_ampersand_place(args *userInput) { return (*userInput).count - 1; }
/**
 * remove the apersand
 * @param userInput args structure
 * @return args with no apersand
 */
args remove_apersand(args *userInput) {
    const int ampersand_place = get_ampersand_place(userInput);
    if ((*userInput).arglist[ampersand_place][0] == '&') {
        (*userInput).arglist[ampersand_place] = END_OF_STRING;
        userInput->count--;
    }
    return (*userInput);
}
/**
 * error wrapper for pipe closure
 * @param status
 */
void validate_pipe_close(int status) {
    error_handler(status, "closing pipe failed");
}
/**
 * in case of a pipe,split into 2 process which communicate via a pipe
 * @param userInput args struct
 * @param bar_index the place of the pipe command
 */
void split_for_each_task(args *userInput, int bar_index) {

    int pipe_end[2];
    int create_pipe_status = pipe(pipe_end);
    error_handler(create_pipe_status, "pipe creation failed");
    int status;
    pid_t fork_id = fork();
    error_handler(fork_id, "forking failed");
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
/**
 * func to handle pipe command
 * @param userInput args struct
 * @param bar_index place of the pipe
 */
void bar_handler(args *userInput, int bar_index) {
    split_for_each_task(userInput, bar_index);
    execute(userInput->arglist);
}
/**
 * func for handle the forked process from the shell
 * @param userInput
 */
void child_action(args userInput) {
    prepare_sigint(SIG_DFL);
    userInput = remove_apersand(&userInput);
    const int bar_location = find_first_vertical_bar(userInput);
    if (bar_location == -1) {
        execute(userInput.arglist);
    } else
        bar_handler(&userInput, bar_location);
}
void wait_error_handler(int status)
{

   if(status<0) //DUP it's known, but prevents edge case as func may change errno for junk value on success
   {
       switch (errno) {
           case ECHILD:
           case EINTR:
               return;
       }
   }
    error_handler(status, "wait failed");

}
/**
 * func to handle shell ui after fork
 * @param userInput
 * @param pid
 */
void parent_action(args userInput, pid_t pid) {
    const int ampersand_place = get_ampersand_place(&userInput);
    const bool should_task_run_in_background = userInput.arglist[ampersand_place][0] == '&';
    if (!should_task_run_in_background) {
        int status = waitpid(pid, NULL, 0);
        wait_error_handler(status);
    }

}
/**
 * convert user input to structure
 * @param count
 * @param arglist
 * @return the args structure
 */
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
    error_handler(fork_id, "forking failed");
    bool is_parent = fork_id;
    if (is_parent) {
        parent_action(user_input, fork_id);
    } else
        child_action(user_input);


    return is_parent;
}


// prepare and finalize calls for initialization and destruction of anything required
int prepare(void) {
    prepare_sigint(SIG_IGN);
    return 0;
}

int finalize(void) {
    return 0;
}