#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "dictionary.h"

/*Define variables*/
pthread_t threadPool[NUM_WORKER_THREADS], logThread; // declare global thread pool to use as worker threads
pthread_mutex_t job_mutex, log_mutex; // declare mutexes to use for job and log buffers
pthread_cond_t job_cv_cs, job_cv_pd; // delcare condition variables for job buffer
pthread_cond_t log_cv_cs, log_cv_pd; // delcare condition varialbes for log buffer
int jobBuff[JOB_BUF_LEN]; // array of ints that represent the socket for a client trying to connect
char logBuff[LOG_BUF_LEN][PHRASE_SIZE]; // array of phrases to be written to log file
int jobLen = JOB_BUF_LEN; // capacity of job buffer
int logLen = LOG_BUF_LEN; // capacity of log buffer
int jobCount, logCount = 0; // number of items in each buffer
int jobFront = -1; // used to keep track of front of job buffer
int jobRear = 0; // used to keep track of rear of job buffer
int logFront = -1; // used to keep track of front of log buffer
int logRear = 0; // used to keep track of rear of log buffer

int connectionPort = 0; // declare global connectionPort to be used
char* dictionaryName = ""; // declare global dictionaryName to be used
char dictionary[DICTIONARY_SIZE][MAX_WORD_SIZE]; // dictionary to store words from dictionaryName file in
int wordsInDictionary = 0; // global var to keep track of word count of dictionary - to be used when searching

struct sockaddr_in client;
int clientLen = sizeof(client);
int connectionSocket, clientSocket;
//char recvBuffer[MAX_WORD_SIZE];
char* clientMessage = "Hello! You're connected to the server. Send the server a word to spell check!\n";
char* msgRequest = "Send me another word to spell check! Or, enter the escape key and hit enter to quit this connection..\n";
char* msgClose = "Goodbye!\n";
FILE* logFile_ptr; // log file pointer to open log file with

/* Returns a char** to all of the words in the dictionary file. This opens the 
    designated file the user puts in or the default, which is dictionary.txt and 
    coopies the list for comparsion.  (simple fgets from lap01)*/
char **open_dictionary(char *filename) {
    FILE *fd;
    char **output = malloc(DICTIONARY_LENGTH *sizeof(char *) + 1);
    char line [BUFF_SIZE];
    int index = 0;

    /* this is a fault if file is entered wrong */
    fd = fopen(filename, "r");
    if(fd == NULL) {
      printf("Couldn't open file dictionary file.\n");
      exit(1);
    }

    while((fgets(line, BUFF_SIZE, fd)) != NULL) {
      output[index] = (char *) malloc(strlen(line) *sizeof(char *) + 1);
      int temp = strlen(line) - 2;
      line[temp] = '\0';
      strcpy(output[index], line);
      index++;
    }
    fclose(fd);
    return output;
}

/* This section was taken from the slides provided and nearly word from 
    word from the book. Essentially, it creates the listener file descriptor, 
    opens the socket descriptor, sets the socketopt. */
int open_listenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serverAddress;

    /* creates a socket descriptor from connection*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
      return -1;
    }

    /* gets rid of the "Address already in use" error */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
      return -1;
    }

    /* Reset the serverAddress struct, setting all of it's bytes to zero.
        bind() is then called, associating the port number with the socket fd. */
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
      return -1;
    }

    /* Prepare the socket to allow accept() calls. Twenty is the backlog accoring
        to the book, this is the max num of connection. Only that max will be placed 
        on queue. Accept() then will be called after that max is met. */
    if (listen(listenfd, 20) < 0) {
        return -1;
    }
    return listenfd;
}

/* create and return the fifo queue struct (lab01 learned stuff) */
Queue *createQueue() {
    Queue *temp = (Queue *)malloc(sizeof(Queue));
    temp->front = NULL;
    temp->queue_size = 0;
    return temp;
}

/* makes a Node struct and also returns it which is used in the 
    queue struct, which is also part of the fifo queue
    previously explained */
Node *createNode(struct sockaddr_in client, char *word, int socket) {
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->client = client;
    if(word == NULL) {
      temp->word = word;
    } else {
      temp->word = malloc(sizeof(char *) *strlen(word) + 1);
      if(temp->word == NULL){
        printf("Unable to allocate memory for Node.\n");
        exit(1);
      }
      strcpy(temp->word, word);
    }
    temp->next = NULL;
    temp->client_socket = socket;
    return temp;
}

/* linked list esk push function that pushes the node onto the queue struct 
    which again was used in lab01 */
void push(Queue *queue, struct sockaddr_in client, char *word, int socket) {
    Node *temp = createNode(client, word, socket);

    /* if the node is empty, simply place at the end of the queue */
    if (queue->queue_size == 0) {
      queue->front = temp;
    } else {
        Node *head = queue->front;
        while(head->next != NULL) {
            head = head->next;
        }
        head->next = temp;
      }
    queue->queue_size++;
    return;
}

/* simple pop function, pops the first node off the queue (again from lab01) */
Node *pop(Queue *queue) {
    /* simply returns NULL, should the queue be empty */
    if (queue->front == NULL) {
      queue->queue_size = 0;
      return NULL;
    }

    /* simply moves the next node front as the orignal front node was popped */
    Node *temp = queue->front;
    queue->front = queue->front->next;
    queue->queue_size--;
    free(queue->front); /* gotta free! */
    return temp;
}

