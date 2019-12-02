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
