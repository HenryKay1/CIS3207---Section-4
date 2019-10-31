#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h> // for working with threads, mutexes, and condition variables
#include <string.h> // used for comparing args at start ie. checking for port number vs. dictionary file
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // need for sockaddr_in struct to use with accept()
#include <unistd.h>

// Defined constants
#define BUFF_SIZE 256
#define DEFAULT_DICTIONARY "dictionary.txt" // to be used if user does not specify a dictionary to use
#define DEFAULT_LOG_FILE "logFile.txt"
#define DEFAULT_PORT 8888 // to be used if no port specified by  user
#define DICTIONARY_LENGTH 99171 /* how many lines are in provided dictionary */
#define JOB_BUF_LEN 100 // size of job buffer which holds socket descriptors
#define LOG_BUF_LEN 100 // size of log buffer
#define MAX_SIZE 1000 // max word size possible in dictionary
#define NUM_WORKERS 5 // number of worker threads to be created at start
#define EXIT_NUM -1

/********* Function Declartion ***********/
//These define the FIFO queues and the creating nodes, the pushing, and pulling of said queues.
Queue *createQueue();
Node *createNode(struct sockaddr_in, char *, int);
void push(Queue *, struct sockaddr_in, char *, int);
Node *pop(Queue *);

/* read the dictionary function */
char **open_dictionary(char *);

//file descriptor for opening the listener 
int open_listenfd(int);

/* both the worker thread function that prompts the user for their input,
  and checks the word against the dictionary. The log thread which is used
  to log all the activity from the clients requests to the server */
void *worker_thread(void *);
void *logger_thread(void *);

//search words from Dictionary.txt
int searchForWordInDict(char list_of_words[][MAX_WORD_SIZE], char* wordToFind);

#endif
