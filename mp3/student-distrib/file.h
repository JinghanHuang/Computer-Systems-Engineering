#ifndef _FILE_H
#define _FILE_H

#include "types.h"
#include "lib.h"

#define FILENAME_SIZE 32
#define DENTRY_RESERVED_SIZE 24
#define DENTRY_SIZE 64
#define BOOT_RESERVED_SIZE 52
#define DENTRY_NUM 63
#define BLOCK_SIZE 4096
#define BLOCK_BYTES 1024


// struct of directory entry
typedef struct dentry {
    uint8_t filename[FILENAME_SIZE];
    uint32_t filetype;
    uint32_t inode_index;
    uint8_t reserved[DENTRY_RESERVED_SIZE];
} dentry_t;

// struct of inode
typedef struct inode {
    uint32_t length;
    uint32_t data_block_index[BLOCK_BYTES - 1];
} inode_t;

// struct of boot block
typedef struct boot_block {
    uint32_t dentry_num;
    uint32_t inode_num;
    uint32_t data_block_num;
    uint8_t reserved[BOOT_RESERVED_SIZE];
    dentry_t dentries[DENTRY_NUM];
} boot_block_t;



// struct of file operation
typedef struct file_op {
    int32_t (*open)(const uint8_t*);
    int32_t (*close)(int32_t);
    int32_t (*read)(int32_t, void*, int32_t);
    int32_t (*write)(int32_t, const void*, int32_t);
} file_op_t;


// struct of file descriptor
typedef struct file_desc {
    file_op_t* file_op;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
} file_desc_t;


// Global operation
extern file_op_t file_op;
extern file_op_t dir_op;
extern file_op_t rtc_op;


// Global variables
extern inode_t* cur_file_inode;
extern inode_t* cur_dir_inode;
extern dentry_t cur_file_dentry;
extern dentry_t cur_dir_dentry;

// file initialization
extern void file_init(uint32_t start_addr);

// file operations
extern int32_t file_open(const uint8_t* fname);
extern int32_t file_close(int32_t fd);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

// directory operations
extern int32_t dir_open(const uint8_t* fname);
extern int32_t dir_close(int32_t fd);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

// help functions
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif /* _FILE_H */
