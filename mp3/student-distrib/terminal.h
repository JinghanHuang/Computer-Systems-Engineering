#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "keyboard.h"
#include "file.h"


extern file_op_t stdin_op;
extern file_op_t stdout_op;


//see the header of .c file
void terminal_init();
int32_t terminal_open(const uint8_t* file);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);


int32_t terminal_no_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_no_write(int32_t fd, const void* buf, int32_t nbytes);


#endif
