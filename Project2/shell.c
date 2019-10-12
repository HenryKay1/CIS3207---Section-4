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

	int i ;
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


// Function where the system command is executed
int myShellLaunch(char **args){
	pid_t pid, wpid;
	int status;
	pid = fork();
	if (pid == 0){
		// The Child Process
		if (execvp(args[0], args) == -1){
			perror("myShell: ");
		}
	exit(EXIT_FAILURE);
	} else if (pid < 0){
		//Forking Error
		perror("myShell: ");
	} else {
		// The Parent Process
	do {
      wpid = waitpid(pid, &status, WUNTRACED); //allow parent process to wait on the child process for execute
    } 
	while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
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

// Read and Parse from Config File
int readConfig(){
	FILE *fptr;
	char line[200];
	char **args;
	fptr = fopen("config.txt", "r");
	if (fptr == NULL){
		printf("Unable to find config file.\n");
		return 1;
	}else{
		while(fgets(line, sizeof(line), fptr) != NULL){
			printf("\n%s", line);
			args=splitLine(line);
			if(strcmp(args[0], "export")==0)
				strcpy(SHELL_NAME, args[1]);
		}
	}
	free(args);
	fclose(fptr);
	return 1;
}

// When myShell is called Interactively
int myShellInteract(){
	char *line;
	char **args;
	while(QUIT == 0){
		printf("%s> ", SHELL_NAME);
		line=readLine();
		args=splitLine(line);
		execShell(args);
		free(line);
		free(args);
	}
	return 1;
}

// When myShell is called with a Script as Argument
int myShellScript(char filename[100]){
	printf("Received Script. Opening %s", filename);
	FILE *fptr;
	char line[200];
	char **args;
	fptr = fopen(filename, "r");
	if (fptr == NULL){
		printf("\nUnable to open file.");
		return 1;
	} else {
		printf("\nFile Opened. Parsing. Parsed commands displayed first.");
		while(fgets(line, sizeof(line), fptr)!= NULL){
			printf("\n%s", line);
			args=splitLine(line);
			execShell(args);
		}
	}
	free(args);
	fclose(fptr);
	return 1;
}

int main(int argc, char **argv){
	// Read from myShell Configuration Files
	readConfig();
	// Parsing commands Interactive mode or Script Mode
	if (argc == 1)
		myShellInteract();
	else if (argc == 2)
		myShellScript(argv[1]);
	else
		printf("\nInvalid Number of Arguments");
	// Exit the Shell
	return EXIT_SUCCESS;
}
