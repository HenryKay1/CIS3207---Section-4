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

int fs_delete(char *name)
{
    char i;

    for(i = 0; i < MAX_FILE; ++i) {
        if(strcmp(dir_info[i].name, name) == 0) {
            char file_index = i;
            file_info* file = &dir_info[i];
            int block_index = file->head;
            int block_count = file->num_blocks;

            if(dir_info[i].fd_count != 0) { // File is currently open
                fprintf(stderr, "fs_delete()\t error: file [%s] is currently open.\n",name);
                return -1;
            }

            // Remove file information
            super_block_ptr->dir_len--;
            file->used = false;
            strcpy(file->name, "");
            file->size = 0;
            file->fd_count = 0;

            /* Free file blocks */
            char buf1[BLOCK_SIZE] = "";
            char buf2[BLOCK_SIZE] = "";
            block_read(super_block_ptr->data_index, buf1);
            block_read(super_block_ptr->data_index + 1, buf2);
            while (block_count > 0){
                if (block_index < BLOCK_SIZE){
                    buf1[block_index] = '\0';
                } else {
                    buf2[block_index - BLOCK_SIZE] = '\0';
                }
                block_index = find_next_block(file->head, file_index);
                block_count--;
            }

            dir_info[i].head = -1;
            dir_info[i].num_blocks = 0;
            block_write(super_block_ptr->data_index, buf1);
            block_write(super_block_ptr->data_index + 1, buf2);

            printf("fs_delete()\t called successfully: file [%s] deleted.\n", name);
            return 0;
        }
    }

    fprintf(stderr, "fs_delete()\t error: file [%s] does not exists\n", name);
    return -1;
}

int fs_read(int fildes, void *buf, size_t nbyte)
{
    if(nbyte <= 0 || !fd_table[fildes].used) {
        return -1;
    }

    int i, j = 0;
    char *dst = buf;
    char block[BLOCK_SIZE] = "";
    char file_index = fd_table[fildes].file;
    file_info* file = &dir_info[file_index];
    int block_index = file->head;
    int block_count = 0;
    int offset = fd_table[fildes].offset;

    /* load current block */
    while (offset >= BLOCK_SIZE){
        block_index = find_next_block(block_index, file_index);
        block_count++;
        offset -= BLOCK_SIZE;
    }
    block_read(block_index, block);

    /* read current block */
    int read_count = 0;
    for(i = offset; i < BLOCK_SIZE; i++) {
        dst[read_count++] = block[i];
        if(read_count == (int)nbyte) {
            fd_table[fildes].offset += read_count;
            return read_count;
        }
    }
    block_count++;

    /* read the following blocks */
    strcpy(block,"");
    while(read_count < (int)nbyte && block_count <= file->num_blocks) {
        block_index = find_next_block(block_index, file_index);
        strcpy(block,"");
        block_read(block_index, block);
        for(j=0; j < BLOCK_SIZE; j++, i++) {
            dst[read_count++] = block[j];
            if(read_count == (int)nbyte ) {
                fd_table[fildes].offset += read_count;
                return read_count;
            }
        }
        block_count++;
    }
    fd_table[fildes].offset += read_count;
    return read_count;
}

int fs_write(int fildes, void *buf, size_t nbyte)
{
    if(nbyte <= 0 || !fd_table[fildes].used) {
        return -1;
    }

    int i = 0;
    char *src = buf;
    char block[BLOCK_SIZE] = "";
    char file_index = fd_table[fildes].file;
    file_info* file = &dir_info[file_index];
    int block_index = file->head;
    int size = file->size;
    int block_count = 0;
    int offset = fd_table[fildes].offset;

    /* load current block */
    while (offset >= BLOCK_SIZE){
        block_index = find_next_block(block_index, file_index);
        block_count++;
        offset -= BLOCK_SIZE;
    }

    int write_count = 0;
    if (block_index != -1){
        /* write current block */
        block_read(block_index, block);
        for(i = offset; i < BLOCK_SIZE; i++) {
            block[i] = src[write_count++];
            if (write_count == (int)nbyte || write_count == strlen(src)) {
                block_write(block_index, block);
                fd_table[fildes].offset += write_count;
                if(size < fd_table[fildes].offset){
                    file->size = fd_table[fildes].offset;
                }
                return write_count;
            }
        }
        block_write(block_index, block);
        block_count++;
    }

    /* write the allocated blocks */
    strcpy(block, "");
    while(write_count < (int)nbyte && write_count < strlen(src) && block_count < file->num_blocks) {
        block_index = find_next_block(block_index, file_index);
        for(i = 0; i < BLOCK_SIZE; i++) {
            block[i] = src[write_count++];
            if(write_count == (int)nbyte || write_count == strlen(src)) {
                block_write(block_index, block);
                fd_table[fildes].offset += write_count;
                if(size < fd_table[fildes].offset){
                    file->size = fd_table[fildes].offset;
                }
                return write_count;
            }
        }
        block_write(block_index, block);
        block_count++;
    }

    /* write into new blocks */
    strcpy(block, "");
    while(write_count < (int)nbyte && write_count < strlen(src)) {
        block_index = find_free_block(file_index);
        file->num_blocks++;
        if (file->head == -1){
            file->head = block_index;
        }
        if (block_index < 0){
            fprintf(stderr, "fs_write()\t error: No free blocks.\n");
            return -1;
        }
        for(i = 0; i < BLOCK_SIZE; i++) {
            block[i] = src[write_count++];
            if(write_count == (int)nbyte || write_count == strlen(src)) {
                block_write(block_index, block);
                fd_table[fildes].offset += write_count;
                if(size < fd_table[fildes].offset){
                    file->size = fd_table[fildes].offset;
                }
                return write_count;
            }
        }
        block_write(block_index, block);
    }

    fd_table[fildes].offset += write_count;
    if(size < fd_table[fildes].offset){
        file->size = fd_table[fildes].offset;
    }
    return write_count;
}

