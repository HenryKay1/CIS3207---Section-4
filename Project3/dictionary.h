#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_LOG_FILE "logFile.txt"
#define DEFAULT_PORT 8010
#define DICTIONARY_LENGTH 99171 /* how many lines are in provided dictionary */
#define EXIT_NUM -1
#define MAX_SIZE 1000
#define NUM_WORKERS 5

/* defining the double pointer for the dictionary of words */
char **dictionary_words;

/****** pThread Locks and Condition variables ******/
/* lock and release the threads */
pthread_mutex_t job_queue_lock;
pthread_mutex_t log_queue_lock;
pthread_mutex_t log_lock;

/* signals where the thread is locked/unlocked 
  prevents deadlocks with the threads */
pthread_cond_t condition1;
pthread_cond_t condition2;
pthread_cond_t condition3;
pthread_cond_t condition4;

#endif
