## PROJECT 1 OVERVIEW 

The purpose of this project is to gain some understanding of how processes move through a computer system and gain more experiences to discrete event simulator.

This project is implemented base on 4 data structures included 3 FIFO queue as LinkedList for CPU, Disk1, Disk2 and 1 priority queue as LinkedList used to store events in temporal order. Events are something like "a job arrives", "CPU reads file", "a disk arrives" or "a job finishes", etc. In fact, events should be removed from the priority queue and built based on the time the event occurred.

In general, this process will follow two main things, which are execute on the CPU and read/write on the disk. A computer is always running processes severally. Since it would not be allow for many processes running at the same time, waiting would be prefer. To implement this, we will have the CPU queue and Disk queue, where are places for a process to wait for execute.

There are two c Files in my source Files:
- readFile.c: is used to read the CONFIG.txt, which includes  some constants such as SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN & ARRIVE_MAX, QUIT_PROB, CPU_MIN & CPU_MAX, DISK1_MAX & DISK2_MIN, for entering the input to the system.
- main.c: is used to execute the process of the event simulation. 

At first, when a task arrived at CPU, we created a new event and then before sending task to CPU FIFO, we appended event to event queue so they will automatically be placed in the correct temporal position. If CPU state is busy, we did nothing otherwise we did ordered followings such as pop task from CPU FIFO, generate the finish time with CPU min and CPU max, operate CPU finish event, inset event into event queue and set CPU state to busy . On the other hand, when a task is finished at the CPU, we set CPU state to IDLE and then exit. 

Continuing, when a task arrived at the DISK, we did the same as arriving at the CPU. On the one side, when it was finished at the DISK, we found the corresponding job in disk1 or disk2, created CPU arrival event and inserted event inside eventQ orderly.

After constructing with 4 data structures, events, tasks and some constants, we tied it all together inside of a main loop. That will essentially include add events to priority queue, pop events from event queue and place them inside CPU queue and pick the appropriate event handler function and call it. We continued this process several time until we hit the end of the simulation. Finally, we will have successfully completed the discrete event simulator.





