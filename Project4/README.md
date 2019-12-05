# DESCRIPTION OF "A SIMPLE FILE SYSTEM ON TOP OF A VIRTUAL DISK"
1. Overview of the project

The purpose of this project are to implement a simple file system on top of a virtual disk and to understand implementation details of file systems.

The file system structure is concluded super block where space block is 0, directory information which block 1 function has lists of file information and data block usage which block 2-3 function store the file index of each block and data blocks which include block 4-8191 function that store the data of the file.

2. Overview of the features of the library components

Function system management:
- int make_fs(char *disk_name): the function create a fresh (and empty) file system on the virtual disk.
- int mount_fs(char *disk_name): the function is stored on a virtual disk.
- int unmount_fs(char *disk_name): the function is unmounted file system from a virtual disk. The data must be written onto the virtual disk.

File system operations
- int fs_open(char *name): the function is opened for reading and writing and the file descriptor corresponding to the file is returned to the calling function.
- int fs_close(int fildes): the file descriptor fildes is closed.
- int fs_create(char *name): the function creates a new file with name "name". The file is initially empty. A value of 0 is returned due to the succesful completion otherwise return -1.
- int fs_delete(char * name): the function deletes the file with the path and name "name" from the directory of your file system and frees all data blocks and meta-information.
- int fs_read(int fildes, void *buf, size_t nbyte): the function is built to read nbyte bytes of data from the file referenced by the descriptor fildes into the buffer pointed to by buf.
- int fs_write(int fildes, void *buf, size_t nbyte): the function is built to write nbyte bytes of data from the file referenced by the descriptor fildes into the buffer pointed to by buf.
- int fs_get_filesize(int fildes): the function returns current size of the file pointed to by the file descriptor fildes. It will return 0 when succedd otherwise return -1 in case the fildes is invalid.
- int fs_lseek(int fildes, off_t offset): the function sets the file pointer associated with the file descriptor fildes to the argument offset.
- int fs_truncate(int fildes, off_t length): the function causes the file referenced by fildes to be truncated to length bytes in size.

3. Test applications

To test my project, I divided it into many small parts:

- Start by testing the functionality of function systems to observe that the file system provide persistent storage. Assume the user had created a file system on a virtual disk and mounted it, then create a few files and write data on them. Finally, the user unmounted the file system. That means all data must be written onto the virtual disk.
- Then continuing testing the file system operation functions to help the disk create, open, close, delete, etc.
- Next, testing the helper functions to file the file descriptor or find the free block.
- Finally, testing the the files created are actually persistent, which means they still remain in the virtual disk even when the user close the program.

To use this program, we just follow commands below:

- Compile the project

gcc -o filesystem fs.c disk.c

./filesystem

- Show the contents of the virtual disk

hexdump -l 4096 mydisk

xxd -l 4096 mydisk