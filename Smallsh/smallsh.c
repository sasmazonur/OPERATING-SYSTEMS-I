//Author: Onur Sasmaz
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LENGTH 2038
#define MAX_ARGUMENT 512
int z_value = 0;
int status_value = 0;
int background = 0;
int back_array[MAX_ARGUMENT];
int num_args = 0;
int num_process = 0;


//Source: https://brennan.io/2015/01/16/write-a-shell-in-c/
//Reading a line from stdin
//easy way with getline
char *read_user_input(void)
{
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    getline(&line, &bufsize, stdin);
    return line; //return to line from stdin
}
//tokenize the string using whitespace as delimiters
char **split_ars(char *line){
    int buffsize = 64;
    int index = 0;
    
    char **tokens = malloc(buffsize * sizeof(char*));
    char *token;
    
    //if malloc fails. Throw error message
    if (!tokens){
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    //start tokenizing
    token = strtok(line, " \t\r\n\a");
    
    // repeats until no token is returned by strtok
    while(token != NULL){
        tokens[index] = token;
        index++;
        
        //if needed, reallocate array of pointers
        if(index >= buffsize){
            buffsize += 64;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    num_args = index;
    tokens[index] = NULL;
    return tokens;
}

//Source: Chapter 3.4 Page 5
//Source:https://www.youtube.com/watch?v=l64ySYHmMmY
//Source: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/create.html
//source: https://www.geeksforgeeks.org/strdup-strdndup-functions-c/
//Credits: Justin Goins, Chris Pomeroy and Spencer Moss explained in Office hours.
//To contunie exec return 1
//launch a program and wait for it to terminate
int bash_launch(char **array){
    int status;
    pid_t pid;
    pid_t wait_pid;
    int r_arr = 0;
    int l_arr = 0;
    int fd = 0;
    int fd_val = 0;
    int in_var = -5;
    int out_var;
    int ampers;
    int background_fl = 0;
    int bg_exitStatus = -1; // execution exit status for BG
    int term_signal = -1;
    int background_val;
    
    int amper_flag;
    
    
    //scan if "$$" is in user input. if find it, change $$ with PID using buffers.
    int r;
    int p;
    for (r = 0; r < num_args; r++) //run number of arguments
    {
        if (strstr(array[r],"$$") != NULL) //scan string and look for $$
        {
            int pid_val = getpid(); //get PID val
            char dol_ar[100] = {0}; //buffer1
            char changed_str[100] = {0}; //buffer2
            snprintf(dol_ar,100, "%d",pid_val); //put pid_val in dol_arr
            for (p = 0; p < strlen(array[r]) - 2; p++) //$$ is 2 char so -2
            {
                changed_str[p] = array[r][p];
            }
            strncat(changed_str, dol_ar,100 ); //append doll_ar in changed_str
            array[r] = strdup(changed_str); //copy changed_str in user input
        }
    }
    
    //Source: Chapter 3.4 Page 5
    pid = fork();
    fflush(stdout);
    
    //Check num_args times, if ampersand exist in userinput
    //if exist set background flag 1
    int z;
    for(z = 0; z<num_args; z++){
        amper_flag = strcmp(array[z], "&");
        if (amper_flag == 0 && z_value == 0){
            background_fl = 1;
        }
    }
    
    //check if fork() had an error.
    if(pid < 0)
    {
        perror("Fork Failed.");
        exit(1);
    }
    
    //Start the child process
    else if(pid == 0){
        int i;
        //scan tru user input
        for(i = 0; i<num_args; i++){
             in_var = strcmp(array[i], "<");
             out_var = strcmp(array[i], ">");
             ampers = strcmp(array[i], "&");
            
            //check if < exist in user input
            if (in_var == 0 ){
                //check if there is & in user input
                if(amper_flag == 0)
                {
                    //standard input redirected from /dev/null if the user did not specify some other file to take standard input from.
                    fd = open("/dev/null", O_RDONLY);
                    if(fd<0)//throw error if file is not valid
                    {
                        perror("Input File is not valid.\n");
                        exit(2);
                    }
                    else //close the file
                    {
                        fcntl(fd, F_SETFD, FD_CLOEXEC);
                    }
                    fd_val=dup2(fd,0); //copy file to stdin
                    if(fd_val < 0) // if fails throw error message
                    {
                        perror("Input Status Fail dup2 (Background)\n");
                        exit(2);
                    }
                }
                //open < <input> and check if file exist. if not throw error
                if((l_arr = open(array[i+1], O_RDONLY, 0644)) < 0)
                {
                    perror("Input File is not valid.\n");
                    exit(1);
                }
                
                status = dup2(l_arr,0); //copy file to stdin
                array[i] = NULL; //set user input null
                
                if(status < 0) //check if fails throw error message
                {
                    perror("Input Status Fail dup2\n");
                    exit(2);
                }
            }
            //check if > exist in user input
            if (out_var == 0 ){
                //check if there is & in user input
                if(amper_flag == 0)
                {
                    //standard input redirected from /dev/null if the user did not specify some other file to take standard input from.
                    fd = open("/dev/null", O_WRONLY);
                    if(fd<0) //throw error if file is not valid
                    {
                        perror("Input File is not valid.\n");
                        exit(2);
                    }
                    else //close the file
                    {
                        fcntl(fd, F_SETFD, FD_CLOEXEC);
                    }
                    fd_val=dup2(fd,0);
                    if(fd_val < 0)
                    {
                        perror("Input Status Fail dup2 (Background)\n");
                        exit(2);
                    }
                }
                
                if((r_arr = open(array[i+1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0)
                {
                    perror("Input File is not valid.\n"); //throw error if file is not valid
                    exit(1);
                }
                status = dup2(r_arr,1);
                printf("status is: %d", status);
                array[i] = NULL;
                
                if(status < 0)
                {
                    perror("Output Status Fail dup2\n"); //throw error if file is not valid
                    exit(2);
                }
            }
            
            if(ampers == 0)
            {
                array[i] = NULL;
            }
            
        }//for loop
        int com_check;
        
        //check if command user input exist. If not throw error message
        com_check = execvp(array[0], array);
        if(com_check < 0)
        {
            fprintf(stderr, ": %s: no such file or directory\n", array[0]);
            fflush(stdout);
        }
        exit(1);
    }
    
    //Parent process
    else{
        if(!background_fl) //check if background is activated
        {
            waitpid(pid, &status_value, 0);
        }
        else{
            printf("Backgrounded pid is %i\n", pid); //print background PID on screen
            fflush(stdout);
            waitpid(pid, &status_value, WNOHANG);
        }
    }
    //Check if backgroudn process is done
    while ( (pid = waitpid(-1, &status_value, WNOHANG) ) > 0)
    {
        if(WIFSIGNALED(status_value))
        {
            term_signal = WTERMSIG(status_value);
            printf("background pid %d is done: terminated by signal %d\n", pid, term_signal);
        }
        
        if(WIFEXITED(status_value))
        {
            background_val = WEXITSTATUS(status_value);
            printf("background process %d is done: exit value %d \n", pid, status_value);
        }
    }
    
    
    return 1; //prompt input again
}


//Check if the user input is Build in command. If not call the exec
//source:https://www.gnu.org/software/libc/manual/html_node/Process-Completion-Status.html
//source:http://www.cplusplus.com/reference/cstdlib/getenv/
//source:https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
int check_arg(char** arg_point){
    if (arg_point[0] == NULL){ return 1; } //if no input do nothhing. Continue
    else if (strcmp(arg_point[0], "exit") == 0){ return 0; } //end function if user input exit
    else if (strcmp(arg_point[0], "cd") == 0) {
        //if user didnt enter anything after cd
        //if directory not exist throw error message
        if (arg_point[1]){
            if(chdir(arg_point[1]) == -1){
                printf("Directory not found.\n");
                fflush(stdout);
            }
        } else{
         //Go to home directory if user only enter cd
            chdir(getenv("HOME"));
        }
    }
    else if (strcmp(arg_point[0], "#") == 0) { return 1;} //check if # exist in user input. If it is contunie
    else if (strcmp(arg_point[0], "status") == 0){
        //Check if the child process terminated normally with exit
        if (WIFEXITED(status_value)){
            //If WIFEXITED is true of status, this macro returns the low-order 8 bits of the exit status value from the child process
            printf("exit value %d\n", WEXITSTATUS(status_value));
            fflush(stdout);
        }
        else{
            //If WIFSIGNALED is true of status, return the signal number of the signal that terminated the child process.
            printf("Terminated by signal %d\n",WTERMSIG(status_value));
            fflush(stdout);
        }
    }
    else {
        //if not build in functions.
        //call redirection function
        bash_launch(arg_point);
    }
    return 1;
}


void bash_loop(void){
    char *line;
    char **args;
    int status;
    //Get user input and start the function
    do{
        printf(": ");
        line = read_user_input();
        args = split_ars(line);
        status = check_arg(args);
    } while(status);
}


//source:https://stackoverflow.com/questions/4891214/how-to-handle-the-signal-ctrl-z-in-mini-shell-c
//source:http://www.yolinux.com/TUTORIALS/C++Signals.html
//Lecture 3.3 Page notes
//when the user hits Ctrl-Z, foreground-only
//mode is entered and background processes aren't allowed
//until Ctrl-Z is entered again.
//Can not use printf for signal handlers.
void catch_z(int sign_flag){
    char* active_m = " Foreground only mode activated. & is ignored.\n";
    char* deactive_m = " Foreground only mode deactivated.\n";
    if (!z_value){
        z_value = 1;
        write(STDOUT_FILENO, active_m, strlen(active_m));
        fflush(stdout);
    }
    else{
        z_value = 0;
        write(STDOUT_FILENO, deactive_m, strlen(deactive_m));
        fflush(stdout);
    }
}

//Dont need use to catch Ctrl+C. However neccerasy for printing signal message.
//source: Class notes 3.3 Advanced User Input with getline()
void catch_c(int sign_flag){
    char *m1 = " terminated by signal 2\n";
    write(STDOUT_FILENO, m1, strlen(m1));
}

//use SA_RESTART because of getline(). from lecture 3.3
int main(){
    
    //Completely initialize this complicated struct to be empty
        struct sigaction SIGINT_action = {0}, SIGZ_action = {0};
    
        //Block/delay Ctrl+C arriving while this mask is in place
        SIGINT_action.sa_handler = catch_c;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = SA_RESTART;
        sigaction(SIGINT, &SIGINT_action, NULL);
    
        //Block/delay Ctrl+Z arriving while this mask is in place
        SIGZ_action.sa_handler = catch_z;
        sigfillset(&SIGZ_action.sa_mask);
        SIGZ_action.sa_flags = SA_RESTART;
        sigaction(SIGTSTP, &SIGZ_action, NULL);
    //set the signals and start the
    bash_loop();
    return EXIT_SUCCESS; //successful execution of a program
}


