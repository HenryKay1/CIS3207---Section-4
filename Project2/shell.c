#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strtok() and strcmp()
#include <unistd.h> // For fork(), pid_t
#include <sys/wait.h> // For waitpid() and associated macros
#include <dirent.h>
#include <sys/types.h> //for dir
#include <limits.h>
#include <fcntl.h> //for open(), close(), 
#include <sys/stat.h> //for userid
#include <pwd.h> //for username

char SHELL_NAME[50] = "myShell";
int QUIT = 0;

// Function Declarations for built-in commands
int myShell_cd(char **args);
int myShell_clr(char **args);
int myShell_dir(char **args);
int myShell_environ(char **args);
int myShell_echo(char **args);
int myShell_help(char **args);
int myShell_pause(char **args);
int myShell_quit();

//List of I/O redirection commands
char *redirection[] = {
	"<", //input redir
	">", //output redir
	">>", //output redir, appends not truncates
	"|", //pipe
	"&" //background
}; 

// Definitions
char *builtin_cmd[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit"};

// Array of function pointers for call from execShell
int (*builtin_func[]) (char **) = {&myShell_cd, &myShell_clr, &myShell_dir, &myShell_environ, &myShell_echo, &myShell_help, &myShell_pause, &myShell_quit}; 

// Function to return number of builtin commands
int numBuiltin(){ 
	return sizeof(builtin_cmd)/sizeof(char *);
}

//size of char **foo, includes null ptr
int argsLength(char **args) {
	int i = 0;
	int len = 0;
	while(args[i] != NULL) {
		len++;
		i++;
  	}
  	return len;
}

//***************************************************************************************************************************
//STARTS BUILD-IN COMMANDS

//Change the default current directory to <directory>, change PWD environ variable
//if <directory> isnt present in arg1, report current directory
//if directory doesnt exist, report error
int myShell_cd(char **args){
	if (args[1] == NULL) {
		//no argument, report current directory
		char buff[PATH_MAX + 1];
		if(getcwd(buff, PATH_MAX + 1) == NULL) {
    		fprintf(stderr, "cwd error in cd\n");
      		exit(EXIT_FAILURE);
    	}
		printf("no <directory> argument found.\nCurrent directory is: %s\n", buff);
		return 1;
  	} else if (chdir(args[1]) != 0) {
      	perror("chdir");
    } else {
      	setenv("PWD",args[1],1); //set environment variable
    }
  	}
  	return 1;
}

//Clear the screen
int myShell_clr(char **args){
	printf("\033[H\033[2J");
}

//List the contents of current directory
//if no <directory> arg passed, returns "no directory entered"
int myShell_dir(char **args){
	int len = 0;
	int saved_stdout;
	int i = 0;
	int io_output = 0;
	int io_output2 = 0;
	int out;
	
	for(i=0;i<argsLength(args);i++) {
		if(strcmp(args[i], redirection[1]) == 0) {
			io_output=i; 
		} else if(strcmp(args[i], redirection[2]) == 0) { 
			io_output2=i; 
		}
	}
	if(io_output > 0) {
		//Save current stdout for use later
		saved_stdout = dup(STDOUT_FILENO);
		//open file for output redirection
		out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	} else if (io_output2 > 0) {
		//open file for output redirection
		out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	}
	
	struct dirent *s;
	DIR *dir = opendir(".");
	if (dir == NULL){  // opendir returns NULL if couldn't open directory 
        printf("Could not open current directory" );  
    } 
    // for readdir() 
    while ((s = readdir(dir)) != NULL) {
            printf("%s\n", s->d_name); 
	}
	//If there was I/O redirection, redirect back to stdout
	if(io_output > 0 || io_output2 > 0) {
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
  	}
  	return 1;
}

//List all the environment strings
int myShell_environ(char **args){
	int len = 0;
	int saved_stdout;
	int i = 0;
	int io_output = 0;
	int io_output2 = 0;
	int out;
	
	for(i=0;i<argsLength(args);i++) {
		if(strcmp(args[i], redirection[1]) == 0) {
			io_output=i; 
		} else if(strcmp(args[i], redirection[2]) == 0) { 
			io_output2=i; 
		}
	}
	if(io_output > 0) {
		//Save current stdout for use later
		saved_stdout = dup(STDOUT_FILENO);
		//open file for output redirection
		out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	} else if (io_output2 > 0) {
		//open file for output redirection
		out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	}
	
	while (environ[i]){
        printf("%s\n",envp[i]);// print all info
		i++:
    }

	//If there was I/O redirection, redirect back to stdout
	if(io_output > 0 || io_output2 > 0) {
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
  	}
  	return 1;
    
}

//Display <comment> on the display followed by a new line
int myShell_echo(char **args){
	int len = 0;
	int saved_stdout;
	int i = 0;
	int io_output = 0;
	int io_output2 = 0;
	int out;
	
	for(i=0;i<argsLength(args);i++) {
		if(strcmp(args[i], redirection[1]) == 0) {
			io_output=i; 
		} else if(strcmp(args[i], redirection[2]) == 0) { 
			io_output2=i; 
		}
	}
	if(io_output > 0) {
		//Save current stdout for use later
		saved_stdout = dup(STDOUT_FILENO);
		//open file for output redirection
		out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	} else if (io_output2 > 0) {
		//open file for output redirection
		out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	}

	i = 2;
	if (args[1] != NULL) {
		printf("%s", args[1]);
	} else {
		printf("\n");
		return 1;
	}
	while(args[i] != NULL) {
		printf(" %s", args[i]);
		i++;
	}
	printf("\n");

	//If there was I/O redirection, redirect back to stdout
	if(io_output > 0 || io_output2 > 0) {
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
  	}
  	return 1;
}

//Display the user manual using more filter
int myShell_help(char **args){
	int len = 0;
	int saved_stdout;
	int i = 0;
	int io_output = 0;
	int io_output2 = 0;
	int out;
	
	for(i=0;i<argsLength(args);i++) {
		if(strcmp(args[i], redirection[1]) == 0) {
			io_output=i; 
		} else if(strcmp(args[i], redirection[2]) == 0) { 
			io_output2=i; 
		}
	}
	if(io_output > 0) {
		//Save current stdout for use later
		saved_stdout = dup(STDOUT_FILENO);
		//open file for output redirection
		out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	} else if (io_output2 > 0) {
		//open file for output redirection
		out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		//replace standard output with output file
		dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
		close(out);
	}

	int i;
	puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Tuyen Pham"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:");

	for (i = 0; i < numBuiltin(); i++) {
		printf("  %s\n", builtin_cmd[i]);
  	}

	printf("Use the man command for information on other programs.\n");
	
	//If there was I/O redirection, redirect back to stdout
	if(io_output > 0 || io_output2 > 0) {
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
  	}
  	return 1;
}

//Pause program
int myShell_pause(char **args){
	printf("Press [Enter] to unpause shell");
    getchar();
    return 1;
}
 
//Quit the shell
int myShell_quit(){
	QUIT=0;
	return 0;
}

//ENDS BUILD-IN COMMANDS
//**************************************************************************************************************************

// Function where the system command is executed
// input is null terminated list of args
// launches non builtin cmd, handles io_redirection commands
// assumes program and program args come before (to the left) of any io redirection
// sets any index containing io redirection to null, so io redirection isnt passed to exec()
// always returns 1 to continue execution loop  
int myshell_launch(char **args){
  pid_t pid, wpid;
  int status; 
  int len = 0;
  int i = 0; //counter
  int io_counter = 0;
  int io_output = 0; // '>'
  int io_input = 0; // '<'
  int io_amper = 0; // '&'
  int io_pipe = 0; // '|'
  int io_output2 = 0; // '>>'
  int in, out;
 
  //iterate through args array, determine which io case(s) are needed
  //if io case found, variable set to equal the position of the redir symbol in args[]
  for(i=0;i<argsLength(args);i++) {
    if(strcmp(args[i], redirection[0]) == 0) { 
      io_input=i; 
    } else if(strcmp(args[i], redirection[1]) == 0) { 
      io_output=i; 
    } else if(strcmp(args[i], redirection[2]) == 0) { 
      io_output2=i; 
    } else if(strcmp(args[i], redirection[3]) == 0) { 
      io_pipe=i; 
    } else if(strcmp(args[i], redirection[4]) == 0) { 
        io_amper=i; 
    }
  }
 
  //begins large list of if else 
  //each one represents actions to take for a particular set of io commands
  if (io_output == 0 && io_input == 0 && io_output2 == 0 && io_pipe == 0){ //if no redirection
    if( (pid = fork()) < 0){ //forking error
      perror("fork");
    } else if (pid == 0){ //child process
        if(io_amper > 0){
          args[io_amper] = NULL; 
        }
        if(execvp(args[0], args) == -1) {
          perror("execvp");
          exit(EXIT_FAILURE);
        }
    } else { //parent process
        if(io_amper > 0) {
          ;
        } else {
          do {
            wpid = waitpid(pid, &status, WUNTRACED); //allow parent process to wait on the child process for execute
          } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
        }
    }
  } 
  else if (io_output > 0 && io_input == 0 && io_output2 == 0 && io_pipe == 0){ //if '>' standard output redirection
    if( (pid = fork()) < 0) { //forking error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        args[io_output] = NULL; //assumes command and cmd args come before redirection
        //open file for output redirection
        out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent process
        if(io_amper > 0) { // background execution
          ;
        } else {
          do {
            wpid = waitpid(pid, &status, WUNTRACED);  //allow parent process to wait on the child process for execute
          } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
        }
    }
  }
  else if (io_output == 0 && io_input > 0 && io_output2 == 0 && io_pipe == 0){ //if '<' standard input redirection
    if( (pid = fork()) < 0) { //forking error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        args[io_input] = NULL; //assumes command and cmd args come before redirection
        //open file for input redirection
        in = open(args[io_input+1], O_RDONLY);
         //replace standard input with input file
        dup2(in, STDIN_FILENO); //STDIN_FILENO is 0
        close(in); //might need to also close in? not sure...
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent process
      if(io_amper > 0) { //background execution
        ;
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);  //allow parent process to wait on the child process for execute
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_output == 0 && io_input == 0 && io_output2 > 0 && io_pipe == 0){ //if '>>' standard output redirection
    if( (pid = fork()) < 0) { //forking error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output2] = NULL; //assumes command and cmd args come before redirection        
        //opens for writing, appending, creating if necessary
        out = open(args[io_output2+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard input with input file
        dup2(out, STDOUT_FILENO); //STOUT_FILENO is 1
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent process
      if(io_amper > 0) {
        ; //background, exit without waiting
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);  //allow parent process to wait on the child process for execute
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }    
    }
  }
  else if (io_output > 0 && io_input > 0 && io_output2 == 0 && io_pipe == 0){ //if '<' and '>' standard input & output redirection
    if( (pid = fork()) < 0) { //forking error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output] = NULL; //assumes command and cmd args come before redirection
        args[io_input] = NULL;
        //open file for input redirection
        in = open(args[io_input +1], O_RDONLY);
        //open file for output redirection
        out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        //replaces standard input with input file
        dup2(in, STDIN_FILENO);
        //closing unused file descriptors
        close(in);
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent process
      if(io_amper > 0) {
        ; //background, exit without waiting
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);  //allow parent process to wait on the child process for execute
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_output == 0 && io_input > 0 && io_output2 > 0 && io_pipe == 0){ //if '<' and '>>' standard input and output redirection
    if( (pid = fork()) < 0) { //forking error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output2] = NULL; //assumes command and cmd args come before redirection
        args[io_input] = NULL;
        //open file for input redirection
        in = open(args[io_input +1], O_RDONLY);
        //open file for output redirection
        out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        //replaces standard input with input file
        dup2(in, STDIN_FILENO);
        //closing unused file descriptors
        close(in);
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent process
      if(io_amper > 0) {
        ; //background, dont wait for child process to finish
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED); //allow parent process to wait on the child process for execute
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_pipe > 0){ //if single '|'
    int j = 0; //counter
    int x = 0; //counter
    int z = 1; //counter
    int status;
    int pipefd[2]; 
     
    char **args2 = malloc((len - io_pipe) * sizeof(char *)); //total length of args (including null ptr) - where the pipe is (so, )
    if (!args2) { //if malloc error
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
    do { //puts to the right of pipe into args2
      args2[j] = args[io_pipe+z];
      j++;
      z++;
    } while (j<len-1-io_pipe); //args len - args null ptr io-pipe position
    //placing null ptrs
    args2[j] = NULL;
    args[io_pipe] = NULL;
     
    pipe(pipefd);
     
    runleft(pipefd, args);
    runright(pipefd, args2);
    close(pipefd[0]);
    close(pipefd[1]); //closed both file descriptors on pipe
 
    while ((pid = wait(&status)) != -1) { //collects any child processes that have exit()ed
      fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
    }
  }
  //going back to loop, no quit
  return 1;
}

//left of pipe
void runleft(int pfd[], char **args) {
  int pid, wpid;
  int status;
  int len = 0;
  int i = 0;
  int io_input = 0;
  int in;
 
  for(i=0;i<argsLength(args);i++) { //if any args are io_input, set index
    if(strcmp(args[i], redirection[0]) == 0) { 
      io_input=i;  
    }
  }
 
  if( (pid = fork()) < 0) { //forking error 
    perror("runsource fork");
  } else if (pid == 0) { //child, handle redirection in here
  
    if(io_input > 0) {
      in = open(args[io_input+1], O_RDONLY);
      dup2(in, STDIN_FILENO); //STDIN_FILENO is 0
      close(in);
      args[io_input] = NULL;
    }
  
    dup2(pfd[1],STDOUT_FILENO); //end of this pipe becomes stdout
    if(execvp(args[0],args) == -1) {
      perror("execvp");
    }
  } else { //parent process
    close(pfd[1]); //close input side of pipe
    do {
      wpid = waitpid(pid, &status, WUNTRACED);//allow parent process to wait on the child process for execute
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
  }
}

//right of pipe
void runright(int pfd[], char **args2){
  int pid, wpid;
  int len = 0;
  int i = 0;
  int io_output = 0;
  int io_output2 = 0;
  int io_amper = 0;
  int out;
  int status;
 
  for(i=0;i<argsLength(args2);i++) { //iterates through args, if any are redirect, set var to index 
    if(strcmp(args2[i],redirection[1]) == 0) { 
      io_output=i; 
    } else if(strcmp(args2[i], redirection[2]) == 0) { 
        io_output2=i; 
    } else if(strcmp(args2[i], redirection[4]) == 0) { 
        io_amper=i; 
    }
  }
  if( (pid = fork()) < 0) { //forking error 
    perror("runsource fork");
  } else if (pid == 0) { //child, handle redirection in here
    if(io_output > 0) {
      out = open(args2[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      //replace standard output with output file
      dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
      close(out); 
      args2[io_output] = NULL;
    } else if(io_output2 > 0) {
      out = open(args2[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      //replace standard output with output file
      dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
      close(out); 
      args2[io_output2] = NULL;
    }
    dup2(pfd[0], STDIN_FILENO); //end of this pipe becomes stdin
    if(execvp(args2[0],args2) == -1) {
      perror("execvp");
    }
  } else { //parent process
      do {
        pid = waitpid(pid, &status, WUNTRACED);//allow parent process to wait on the child process for execute
      } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
  }
}

// Function to read a line from command into the buffer
char *readLine(){
	char *line = (char *)malloc(sizeof(char) * 1024); // Dynamically Allocate Buffer
	char c;
	int pos = 0, bufsize = 1024;
	if (!line){ // Buffer Allocation Failed
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	while(1){
		c=getchar();
		if (c == EOF || c == '\n'){ // If End of File or New line, replace with Null character
			line[pos] = '\0';
			return line;
		} else {
			line[pos] = c;
		}
		pos ++;
		// If we have exceeded the buffer
		if (pos >= bufsize){
			bufsize += 1024;
			line = realloc(line, sizeof(char) * bufsize);
			if (!line){ // Buffer Allocation Failed
				printf("\nBuffer Allocation Error.");
				exit(EXIT_FAILURE);
			}
		}
	}
}

// Function to split a line into constituent commands
char **splitLine(char *line){
	char **tokens = (char **)malloc(sizeof(char *) * 64);
	char *token;
	char delim[10] = " \t\n\r\a";
	int pos = 0, bufsize = 64;
	if (!tokens){
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, delim);
	while (token != NULL){
		tokens[pos] = token;
		pos ++;
		if (pos >= bufsize){
			bufsize += 64;
			line = realloc(line, bufsize * sizeof(char *));
			if (!line){ // Buffer Allocation Failed
				printf("\nBuffer Allocation Error.");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, delim);
	}
	tokens[pos] = NULL;
	return tokens;
}

// Function to execute command from terminal
int execShell(char **args){
	int ret;
	if (args[0] == NULL){
		// Empty command
		return 1;
	}
	// Loop to check for builtin functions
	for (int i=0; i< numBuiltin(); i++){ // numBuiltin() returns the number of builtin functions
		if(strcmp(args[0], builtin_cmd[i])==0) // Check if user function matches builtin function name
			return (*builtin_func[i])(args); // Call respective builtin function with arguments
	}
	ret = myShellLaunch(args);
	return ret;
}

//main: handles batchfile
//passes batchfile lines to execute(), exits when complete
//if no batchfile, enters cmd loop accepting user input
int main(int argc, char **argv){
  char *line;
  char **args;
  int status;
  char curDir[PATH_MAX + 1]; 
  //if there is a batchfile:
  //batchfile is split into lines 
  //lines are processed through the exit function
  //function then exits
  if (argc > 1) {
    int i; //counter
    FILE *fp;
    char **batch_lines;
    char *token;
    
    char *filename = argv[1];
    if((fp = fopen(filename, "r")) == NULL) { //open batchfile for reading
      printf("fileopen error\n");
      return -1;
    }
 
    fseek(fp, 0, SEEK_END); //go to end of batchfile
    int inputsize = ftell(fp); //this is size of batchfile
    char *batchfile = malloc(inputsize * sizeof(char)); //batchfile char array is now correct size
    if (!batchfile) { //err check
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
 
    fseek(fp,0,SEEK_SET); //go back to beginning of file
 
    for (i = 0; i < inputsize; i++) { //fill *batchfile
      fscanf(fp, "%c", &batchfile[i]);
    }
    batchfile[i] = EOF;
    fclose(fp); //done with file
    int buffsize = MYSHELL_TOKEN_SIZE;
    int position = 0;
    char **buffertokens = malloc(buffsize * sizeof(char*));
 
    if (!buffertokens) { //err check
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
 
    token = strtok(batchfile, BATCH_TOK_DELIM);
     
    while (token != NULL) {
      buffertokens[position] = token;
      position++;
      if (position >= buffsize) { //if reallocation is needed
        buffsize += MYSHELL_TOKEN_SIZE;
        buffertokens = realloc(buffertokens, buffsize * sizeof(char*));
        if(!buffertokens) {
          fprintf(stderr, "allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
      token = strtok(NULL, BATCH_TOK_DELIM);
    }
    buffertokens[position] = NULL;
     
    for(i = 0; i < (position-1); i++) {
      batch_lines = splitLine(buffertokens[i]);
      status = execShell(batch_lines);
      if (status < 1) {
        //freeing memory before shutdown
        free(batch_lines);
        free(buffertokens);
        return EXIT_SUCCESS;
      } 
    }

  } else {
    //Loop getting input and executing.
    // No batchfile, run command loop accepting user input from stdin
     printf("\nWELCOME TO MY SHELL!!!\n");
    //if no batchfile, shell loop begins
    do {
      if(getcwd(curDir, PATH_MAX + 1) == NULL) {
          fprintf(stderr, "cwd error\n");
          exit(EXIT_FAILURE);
      }
      struct passwd *pws; //for getting id
      pws = getpwuid(geteuid());
      printf("%s@%s$ ", pws->pw_name, curDir); //prompt
      line = readLine(); //get line from user
      args = splitLine(line);  //split line into tokens
      status = execShell(args); //run tokenized line, lots of work in this function
      free(line);
      free(args);
    } while (status); //continue while execute returns positive status
  }
  
  //shutdown
  return EXIT_SUCCESS;
}
