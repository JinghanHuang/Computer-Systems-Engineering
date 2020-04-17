#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "types.h"
#include "pcb.h"

#define RTC_FILE_TYPE       0
#define DIR_FILE_TYPE       1
#define REGULAR_FILE_TYPE   2
#define FD_START      0
#define FILE_START    2
#define FD_MAX        8
#define START_INFO_SIZE    40
#define MODE_ONE      -1
#define MODE_TWO      -2
#define EXE_ONE       0x7f
#define EXE_TWO       0x45
#define EXE_THREE     0x4c
#define EXE_FOUR      0x46
#define EXE_1  0
#define EXE_2  1
#define EXE_3  2
#define EXE_4  3
#define BYTE_24 24
#define BYTE_25 25
#define BYTE_26 26
#define BYTE_27 27
#define ONE_BYTE 8
#define TWO_BYTE 16
#define THREE_BYTE 24
#define PROGRAM_IMAGE_VIRTUAL_ADDR 0x08048000
#define ONE_TWO_EIGHT_MB 0x08000000
#define BUF_SIZE 128
#define PAGE_BORDER     4

extern uint32_t cur_pid;
extern uint32_t pre_pid;


int32_t halt(uint8_t status);

int32_t execute(const uint8_t* command);

int32_t read(int32_t fd, void* buf, int32_t nbytes);

int32_t write(int32_t fd, const void* buf, int32_t nbytes);

int32_t open(const uint8_t* filename);

int32_t close(int32_t fd);

int32_t getargs(uint8_t* buf, int32_t nbytes);

int32_t vidmap(uint8_t** screen_start);

int32_t set_handler(int32_t signum, void* handler_address);

int32_t sigreturn(void);

int32_t null_open(const uint8_t* file);
int32_t null_close(int32_t fd);
int32_t null_read(int32_t fd, void* buf, int32_t nbytes);
int32_t null_write(int32_t fd, const void* buf, int32_t nbytes);


#endif /* _SYSTEM_CALL_H */
