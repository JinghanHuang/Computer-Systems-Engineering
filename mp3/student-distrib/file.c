#include "file.h"
#include "system_call.h"
#include "rtc.h"

//File variables
static boot_block_t* boot_block;
static uint32_t dir_read_index;
static uint32_t cur_dir_inode_index;
static uint32_t cur_file_inode_index;
inode_t* cur_dir_inode;
inode_t* cur_file_inode;
dentry_t cur_file_dentry;
dentry_t cur_dir_dentry;


file_op_t file_op;
file_op_t dir_op;
file_op_t rtc_op;


/* void file_init(uint32_t start_addr)
 * Inputs: start_addr - the start address of boot block
 * Return Value: none
 * Function: Initialize file */
void file_init(uint32_t start_addr) {

    boot_block = (boot_block_t*) start_addr;
    dir_read_index = 0;
    cur_dir_inode_index = 0;
    cur_dir_inode = NULL;

    //initialize pointer to file operations jump table
    file_op.open = &file_open;
    file_op.close = &file_close;
    file_op.read = &file_read;
    file_op.write = &file_write;
    dir_op.open = &dir_open;
    dir_op.close = &dir_close;
    dir_op.read = &dir_read;
    dir_op.write = &dir_write;
    rtc_op.open = &rtc_open;
    rtc_op.close = &rtc_close;
    rtc_op.read = &rtc_read;
    rtc_op.write = &rtc_write;
}

/* int32_t file_open(const uint8_t * fname)
 * Inputs: fname - flie name
 * Return Value: 0 - success, -1 - fail
 * Function: Initialize any temporary structures */
int32_t file_open(const uint8_t * fname) {
    if(read_dentry_by_name(fname, &cur_file_dentry) != -1){
        cur_file_inode_index = cur_file_dentry.inode_index;
        cur_file_inode = (inode_t*) (boot_block + cur_file_inode_index + 1);
        return 0;
    }
    return -1;
}

/* int32_t file_close(int32_t fd)
 * Inputs: fd
 * Return Value: 0 - success, -1 - fail
 * Function: undo what you did in the open function */
int32_t file_close(int32_t fd) {
    return 0;
}

/* int32_t file_write(int32_t fd, const void * buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: should do nothing */
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes) {
    return -1;
}

/* int32_t file_read(int32_t fd, void * buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: reads count bytes of data from file into buf */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes) {
    int32_t ret;

    if(fd == -1){
        ret = read_data(cur_file_inode_index, 0, buf, nbytes);
    }
    else{
        uint32_t inode = pcb_array[cur_pid]->fd_array[fd].inode;
        uint32_t file_pos = pcb_array[cur_pid]->fd_array[fd].file_pos;
        ret = read_data(inode, file_pos, buf, nbytes);
        pcb_array[cur_pid]->fd_array[fd].file_pos += ret;
    }

    return ret;
}

/* int32_t dir_open(const uint8_t * fname)
 * Inputs: fname
 * Return Value: 0 - success, -1 - fail
 * Function: opens a directory file (note file types) */
int32_t dir_open(const uint8_t * fname) {
    if(read_dentry_by_name(fname, &cur_dir_dentry) != -1){
		    // store information of the first file
        cur_dir_inode_index = cur_dir_dentry.inode_index;
		    cur_dir_inode = (inode_t*) (boot_block + cur_dir_inode_index + 1);
        return 0;
    }
    return -1;
}

/* int32_t dir_close(int32_t fd)
 * Inputs: fd
 * Return Value: 0 - success, -1 - fail
 * Function: probably does nothing */
int32_t dir_close(int32_t fd) {
	  cur_dir_inode_index = 0;
	  cur_dir_inode = NULL;
    return 0;
}

/* int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: probably does nothing */
int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes) {
    //should do nothing
    return -1;
}

/* int32_t dir_read(int32_t fd, void * buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: read files filename by filename, including “.” */
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes) {
    // check if the index is out of range
    if(dir_read_index >= boot_block->dentry_num){
        dir_read_index = 0;
        return 0;
    }

	  // check if we can find the dentry
    if(read_dentry_by_index(dir_read_index, &cur_dir_dentry) == -1){
        return 0;
    }

	  cur_dir_inode_index = cur_dir_dentry.inode_index;
	  cur_dir_inode = (inode_t*) (boot_block + cur_dir_inode_index + 1);

	  // copy file name to buf
    memcpy(buf, (const char *) &cur_dir_dentry.filename, nbytes);
    dir_read_index++;

    if(strlen((const char *)cur_dir_dentry.filename) >= 32){
        return 32;
    }
    else{
        return strlen((const char *)cur_dir_dentry.filename);
    }
}


/* int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry)
 * Inputs: fname, dentry
 * Return Value: 0 - success, -1 - fail
 * Function: read dentry by name */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry) {

    int i;

    if(strlen((const int8_t*) fname) > FILENAME_SIZE){
        return -1;
    }

    for(i = 0; i < boot_block->dentry_num; i++){
		    // compare file names
        if(strncmp((const char *) fname, (const char *) boot_block->dentries[i].filename, FILENAME_SIZE) == 0){
            memcpy((void *) dentry, (const void *) &boot_block->dentries[i], DENTRY_SIZE);
            return 0;
        }
    }
    return -1;
}

/* int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry)
 * Inputs: index, dentry
 * Return Value: 0 - success, -1 - fail
 * Function: read dentry by index */
int32_t read_dentry_by_index(uint32_t index, dentry_t * dentry) {

    if(index >= boot_block->dentry_num){
        return -1;
    }

    memcpy((void *) dentry, (const void *) &boot_block->dentries[index], DENTRY_SIZE);
    return 0;
}

/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length)
 * Inputs: inode, offset, buf, length
 * Return Value: the number of bytes read and placed in the buffer
 * Function: read dentry by index */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length) {

    int i;
    inode_t* inode_ptr;
    uint32_t data_block_node_index;
    uint32_t data_block_offset;
    uint8_t* data_block_addr;

	  // check if inode is out of range
    if(inode >= boot_block->inode_num){
        return -1;
    }

    inode_ptr = (inode_t*) (boot_block + inode + 1);

	  // check if offset out of file
    if(offset >= inode_ptr->length){
        return 0;
    }

    if(offset + length > inode_ptr->length){
        length = inode_ptr->length - offset;
    }

	  // find the data block
    data_block_node_index = offset / BLOCK_SIZE;
    data_block_offset = offset % BLOCK_SIZE;
	  data_block_addr = (uint8_t*) (boot_block + boot_block->inode_num + 1);;
    data_block_addr += inode_ptr->data_block_index[data_block_node_index] * BLOCK_SIZE;

	  // copy data in data block to buffer
    for(i = 0; i < length; i++){
        buf[i] = data_block_addr[data_block_offset];
        data_block_offset++;
        if(data_block_offset >= BLOCK_SIZE){
            data_block_node_index++;
            data_block_addr = (uint8_t*) (boot_block + boot_block->inode_num + 1);;
		      	data_block_addr += inode_ptr->data_block_index[data_block_node_index] * BLOCK_SIZE;
            data_block_offset = 0;
        }
    }

    return length;
}
