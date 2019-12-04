#include <stddef.h>
#include <printf.h>
#include <memory.h>
#include <stdlib.h>
#include "file.h"
#include "disk.h"

super_block*    super_block_ptr;               // Super block
file_info*      dir_info;                      // Directory info
file_descriptor fd_table[MAX_FILE_DESCRIPTOR]; // File descriptor table

int make_fs(char *disk_name)
{
    make_disk(disk_name);
    open_disk(disk_name);

    /* Initialize the super block */
    super_block_ptr = (super_block*)malloc(sizeof(super_block));
    if (super_block_ptr == NULL) return -1;
    super_block_ptr->dir_index = 1;
    super_block_ptr->dir_len = 0;
    super_block_ptr->data_index = 2;

    /* write the super block to disk (block 0) */
    char buf[BLOCK_SIZE] = "";
    // memset(void *b, int c, size_t len):
    // writes len bytes of value c (converted to an unsigned char) to the string b.
    memset(buf, 0, BLOCK_SIZE);
    // memcpy(void *restrict dst, const void *restrict src, size_t n);
    // copies n bytes from memory area src to memory area dst.
    memcpy(buf, &super_block_ptr, sizeof(super_block));
    block_write(0, buf);
    free(super_block_ptr);

    close_disk();
    printf("make_fs()\t called successfully.\n");
    return 0;
}

int mount_fs(char *disk_name)
{
    if(disk_name == NULL) return -1;
    open_disk(disk_name);

    /* read super block */
    char buf[BLOCK_SIZE] = "";
    memset(buf, 0, BLOCK_SIZE);
    block_read(0, buf);
    memcpy(&super_block_ptr, buf, sizeof(super_block_ptr));

    /* read directory info */
    dir_info = (file_info*)malloc(BLOCK_SIZE);
    memset(buf, 0, BLOCK_SIZE);
    block_read(super_block_ptr->dir_index, buf);
    memcpy(dir_info, buf, sizeof(file_info) * super_block_ptr->dir_len);

    /* clear file descriptors */
    int i;
    for(i = 0; i < MAX_FILE_DESCRIPTOR; ++i) {
        fd_table[i].used = false;
    }

    printf("mount_fs()\t called successfully: file system [%s] mounted.\n", disk_name);
    return 0;
}

int umount_fs(char *disk_name)
{
    if(disk_name == NULL) return -1;

    /* write directory info */
    int i, j = 0;
    file_info* file_ptr = (file_info*)dir_info;
    char buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    char* block_ptr = buf;

    for (i = 0; i < MAX_FILE; ++i) {
        if(dir_info[i].used == true) {
            memcpy(block_ptr, &dir_info[i], sizeof(dir_info[i]));
            block_ptr += sizeof(file_info);
        }
    }

    block_write(super_block_ptr->dir_index, buf);

    /* clear file descriptors */
    for(j = 0; j < MAX_FILE_DESCRIPTOR; ++j) {
        if(fd_table[j].used == 1) {
            fd_table[j].used = false;
            fd_table[j].file = -1;
            fd_table[j].offset = 0;
        }
    }

    free(dir_info);
    close_disk();
    printf("umount_fs()\t called successfully: file system [%s] umounted.\n", disk_name);
    return 0;
}

int fs_open(char *name)
{
    char file_index = find_file(name);
    if(file_index < 0) {  // file not found
        fprintf(stderr, "fs_open()\t error: file [%s] does not exist.\n",name);
        return -1;
    }

    int fd = find_free_file_des(file_index);
    if (fd < 0){
        fprintf(stderr, "fs_open()\t error: exceed the maximum file descriptor number.\n");
        return -1;
    }

    dir_info[file_index].fd_count++;
    printf("fs_open()\t called successfully: file [%s] opened.\n", name);
    return fd;
}

int fs_close(int fildes)
{
    if(fildes < 0 || fildes >= MAX_FILE_DESCRIPTOR || !fd_table[fildes].used) {
        return -1;
    }

    file_descriptor* fd = &fd_table[fildes];

    dir_info[fd->file].fd_count--;
    fd->used = false;

    printf("fs_close()\t called successfully: file [%s] closed\n", dir_info[fd->file].name);
    return 0;
}

int fs_create(char *name)
{
    char file_index = find_file(name);

    if (file_index < 0){  // Create file
        char i;
        for(i = 0; i < MAX_FILE; i++) {
            if(dir_info[i].used == false) {
                super_block_ptr->dir_len++;
                /* Initialize file information */
                dir_info[i].used = true;
                strcpy(dir_info[i].name, name);
                dir_info[i].size = 0;
                dir_info[i].head = -1;
                dir_info[i].num_blocks = 0;
                dir_info[i].fd_count = 0;
                printf("fs_create()\t called successfully: file [%s] created.\n", name);
                return 0;
            }
        }
        fprintf(stderr, "fs_create()\t error: exceed the maximum file number.\n");
        return -1;
    } else {              // File already exists
        fprintf(stderr, "fs_create()\t error: file [%s] already exists\n",name);
        return 0;
    }
}

