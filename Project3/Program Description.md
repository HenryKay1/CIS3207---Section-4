## PROGRAM DESCRIPTION

In this part, I will describe briefly in detail how I designed and implemented my project and how to test my program.

1. Program design.

When designing this program, it is important to break it into smalls part in order to easily code and test the program. My program is split into three main parts, which are the main thread, the worker thread and the log thread. Firstly, the main thread is responsible for initializing a mutex and condition variables for each buffer, creating the thread pool of worker threads and a log thread, processing the user's arguments, loading the dictionary file into memory to use, getting a connection with a client and inserting that socket descriptor describing that connection into the job buffer using the job buffer mutex and the job buffer condition variables. Secondly, the worker thread is created to call the start routines which making a thread funcition for called workerThreadFunc(). This function is in an inifinite loop, removes the clients socket descriptor to be serviced from the job buffer using mutual exclusion and condition variables to signal, services that client's socket descriptor by receiving words from the user until the user enters "Esc+Enter" to stop connecting. Thirdly, the log thread is created the start routine which is a thread function I made, creatively called logThreadFunc(). This log thread function sits in an infinite loop that safety removes a phrase from the log buffer using mutual exclusion and condition variables and appends that phrase to the log file after opening it, closes the log file, releases the mutex and signals that the log buffer is not empty because we would safely removed from it. 

2. Test my program.

To test my program, I divided it into many small parts:

- Start by testing the functionality of my searchForWordInDict(). It can be done to keep track of words in dictionary by iterating through the dictionary and increment of it.
- Test thread created successfully with if blocks (pthread_create() returns 0 on success).
- Test the connection success after inserting client socket descriptor into job buffer. It will print out the connection success every time a connection is made.
- Test the workerThreadFunc() by showing removing job buffer successfully. It print out the job buffer count after removing, receiving word from client, searching dictionary for word to spell correct or not, inserting the phrase into log buffer.
- Test the logThreadFunc() by taking phrase out of log buffer, shows that the phrase is the same as the one that was inserted. We have to append the phrase to the log file and show log file after entering words on client.