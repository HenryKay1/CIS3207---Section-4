## PROGRAM DESCRIPTION

1. An brief introduction to my shell

In this assignment, I build the simple Linux shell, which is called "myshell". The shell is the heart of command line interface. It should operate in the basic way as when you type in a command, the shell will create a child process that executes the command when you entered and prompts then for more user input when it has finished. The purpose of this assignment is to familiarize with the Linux programming environment, learn how processes are created, destroyed and managed and gain some understanding of the necessary functionality in shells.

2. My overall program design

"myshell" is a Linux simple shell, which is designed to parse the input into commands, or input redirection, output redirection or the background execution processing. In fact, once the input is either parsed or processed, the output will execute the command specified on that line of the input and continue waiting for the command finish. This process will be repeated until the user types command "quit". This shell does not work well as many standard shell features, it does only support for the listed commands below:

2.1 the internal commands:

- cd: change the current default directory.
- clr: clear the screen.
- dir: list the contents of directory.
- environ: list all the environment strings.
- echo: display the comment on the display followed by a new line.
- help: display the user manual using the more filter.
- pause: pause the program until 'Enter' is pressed.
- quit: quit the shell.

2.2 input syntax formatting:

- command arg1 arg2 < inputfile > outputfile
- command arg1 arg2 < inputfile >> outputfile
- command arg1 arg 2 < inputfile | command arg1 arg2 > outputfile
- command arg1 &

3. A detailed description of how I tested my program

Since the shell is written by C programming, we have to compile the myshell.c file first in Linux environment by the command "gcc -o myshell myshell.c". It will automatically compile and run the program into "myshell" in the terminal of Linux environment. At this point, "myshell" is running and starts to accept commands. In general, there are two modes, which are interactive mode and batch mode allow for the user to use:

- Ineractive mode: by typing "./myshell" to allow the user typing commands directly. After that, the shell is ready to work with the internal commands such as cd, clr, dir, env, echo, help, pause and quit. Furthermore, if the user wants to take its command line input from a file, they can use some input syntax formatting examples above to type the command for I/O redirection or background execution processing.

- Batch mode: in this area, the shell is supported by reading input from a batch file, which is listed some commands that a developer want to print out the output automatically. By typing "./myshell batch.txt", the shell will parse on some commands based on the batch.txt file and then execute the program corresponding to the command or print out the output file.




