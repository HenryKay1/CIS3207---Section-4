## INTRODUCE PROGRAM TO POTENTIAL USER

In this part, I will instruct for a potential user working on Linux environment how to use the network spell checker. I will instantly explain in 2 aspects below:

1. Program overview

In this assignment, I build the Network Spell checker in Linux environment by using the socket.h and pthread.h in C libraries with other libraries. This project starts a server which uses multithreading and synchronization to connect with multiple clients who can send words to server. The server will search a dictionary word and then respond to the client if that word is spelled correctly or not. As it responds to the client, it also writes the phrase to a log file for the clients to view all words they sent and whether or not they were correct. In general, this program works around some key topics such as sockets, multithreading, concurrency and synchonization.

2. Explain how to use this program:

- First of all, we have to open the terminal and call the server as follows: 

    ./server portNumber textFileName
    
the server takes two arguments, a portNumber is default_port or any portNumber you want and an optional text file, which is dictionary.txt for spell checking.

- Next, open another terminal for the user (clients) to connect the server as follows:

    ipAddress portNumber
    
where an ipAddress here always use "nc 127.0.0.1" and the same portNumber that we called in server.

- Then, we begin entering words into the user terminal window.

- To exit, enter "Esc+Enter" into the user window.

- To reconnect the server or for the other users connect, just type "ipAddress portNumber" again.

Otherwise, for testing the server, which means the user sent words to the server and the server will tell it okay or misspelled we can call the server as follow:

    ./testing portNumber textFileName
    
and do something the same as the below instructions. 
