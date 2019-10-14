## INSTRUCTION USER COMMANDS

In this part, I will instruct for a beginner working on Linux environment how to use the simple shell. I will instantly explain in 3 aspects below:

1. Explain the concepts necessary: includes I/O redirection, piping and background execution

- '>': is the output redirection operator (stdout) to file, creating if necessary.
- '>>': is very similar to '>', except appending output to an existing file.
- '<': is the input redirection operator (stdin). By default, standard input gets its contents from the keyboard directly.
- '|': is the pipe, which is a set of processes chained by the standard streams so that the output of the process to the left of the pipe (stdout) works directly as input (stdin) for processing to the right of pipe.
- '&': is the background execution, which is added at the end of the command line in order that the shell should return to the command line prompt immediately after launching the program.

2. Explain how to use the bash commands

- '>': command arg1 > outputfile.
For example: "ls > out.txt" is a command to list all the computer files into the out.txt
- '>>' command arg1 >> outputfile.
For example: "ls >> file.txt" is a command to appended the file.txt 
- '<' command ard1 < inputfile.
For example: "sort < file.txt" is a command to process the contents of file.txt. The results are output on the display since the standard output was not redirected.
- '|': command arg1 arg 2 < inputfile | command arg1 arg2 > outputfile.
For example: "cat text.txt | wc -l" is a command to read data from text.txt and then to print out the word count of lines number.
- '&': command arg1 &.
For example: "firefox &" is a command to run the Firefox.

4. Explain all of the built-in commands by describing 

4.1 Description of built-in commands

- 'cd': change the current default directory.
- 'clr': clear the screen.
- 'dir': list the contents of directory.
- 'environ': list all the environment strings.
- 'echo': display the comment on the display followed by a new line.
- 'help': display the user manual using the more filter.
- 'pause': pause the program until 'Enter' is pressed.
- 'quit': quit the shell.

4.2 How to use built-in commands: 

First, we open the terminal in Linux Environment by pressing "Ctrl+Alt+T" in Lubuntu. Then, we type the commands below by:

- 'cd': type "cd" then press "Enter".
- 'clr': type "clr" then press "Enter".
- 'dir': type "dir" then press "Enter".
- 'environ': type "env" then press "Enter".
- 'echo': type "echo + argument" where argument is comment you want to display by a new line. For example, type "echo + Hi" then press "Enter" so "Hi" will display in a new line.
- 'help': type "help" then press "Enter".
- 'pause': type "pause" then if you want to continue by pressing "Enter".
- 'quit': type "quit" then press "Enter" to return another execution.


