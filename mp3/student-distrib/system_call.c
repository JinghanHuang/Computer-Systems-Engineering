#include "system_call.h"
#include "file.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "x86_desc.h"
#include "multi_terminal.h"
#include "schedule.h"

uint32_t cur_pid = 0;
uint32_t pre_pid = 0;
file_op_t null_op = {null_open, null_close, null_read, null_write};

/* int32_t halt(uint8_t status)
 * Inputs: status
 * Return Value: 0
 * Function: halt */
int32_t halt(uint8_t status) {
    if(get_cursor_x() != 0){
        putc('\n');
    }
    cli();
    int i;
    uint32_t post_pid;

    if(pcb_array[cur_pid]->parent == pcb_array[cur_pid]){
        pre_pid = cur_pid;
        pcb_array[cur_pid]->flag = 0;
        execute((uint8_t*)"shell");
    }
    else{
        pcb_array[cur_pid]->parent = NULL;
        pcb_array[cur_pid]->fd_array[0].file_op = &null_op;
        pcb_array[cur_pid]->fd_array[0].flags = 0;
        pcb_array[cur_pid]->fd_array[1].file_op = &null_op;
        pcb_array[cur_pid]->fd_array[1].flags = 0;

        pcb_array[cur_pid]->flag = 0;
        pcb_array[cur_pid]->process_id = 0;
        pcb_array[cur_pid]->terminal_index = -1;

        //what about trying to halt the last process
        for(i = FILE_START; i < FD_MAX; i++){
            // halt() calls close() on opened files
            if(pcb_array[cur_pid]->fd_array[i].flags == 1){
                pcb_array[cur_pid]->fd_array[i].file_op->close(i);
            }
            pcb_array[cur_pid]->fd_array[i].flags = 0;
            pcb_array[cur_pid]->fd_array[i].file_op = &null_op;
        }
    }

    // restore the parent page
    post_pid = cur_pid;
    cur_pid = pre_pid;
    pre_pid = pcb_array[cur_pid]->parent->process_id;
    set_up_page(cur_pid);

    flush_TLB();

    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - cur_pid * EIGHT_KB - PAGE_BORDER;

    sti();

    asm volatile("                      \n\
                 mov %0, %%eax          \n\
                 mov %1, %%esp          \n\
                 mov %2, %%ebp          \n\
                 leave                  \n\
                 ret                    \n\
                 "
                 :/* no outputs */
                 :"r"((uint32_t)status), "r"(pcb_array[post_pid]->parent_stack_ptr), "r"(pcb_array[post_pid]->parent_base_ptr)
                 :"%eax"
                 );
    return 0;
}

/* int32_t execute(const uint8_t* command)
 * Inputs: command
 * Return Value: 0
 * Function: execute */
int32_t execute(const uint8_t* command) {

    int32_t i;
    uint8_t buf[START_INFO_SIZE];
    uint32_t idx = 0, parsed_idx = 0, arg_idx = 0;
    uint8_t parsed_cmd[BUF_SIZE], argu[BUF_SIZE];

    cli();

    //****************** Parse Args ******************//
    if (!command){
        return -1;	// EMPTY_CMD
    }
  	parsed_cmd[0] = NULL;
  	argu[0] = NULL;

  	// parse the command
	while (command[idx] == ' '){
        idx++;
    }
  	if (!command[idx]){
        return -1;	// EMPTY_CMD
    }

  	while (command[idx] != NULL && command[idx] != ' '){
        parsed_cmd[parsed_idx++] = command[idx++];
    }
  	parsed_cmd[parsed_idx] = 0;

  	if (command[idx]!=NULL){
  	    while (command[idx] == ' '){
            idx++;
        }
  	    if (!command[idx]){
            return 0;
        }
        while (command[idx] != NULL && command[idx] != ' '){
            argu[arg_idx++] = command[idx++];
        }
        argu[arg_idx] = 0;
    }

    if (argu[0]){
            terminal_write(0, (void*)argu, arg_idx);
            printf("\n");
    }

    //****************** Check file validity ******************//
    if(file_open(parsed_cmd) == -1){
        return -1;
    }

    // read the first 40 bytes to the buffer
    if(file_read(MODE_ONE, buf, START_INFO_SIZE) == -1){
        return -1;
    }

    // check the first 4 bytes of the file
    if(buf[EXE_1] != EXE_ONE || buf[EXE_2] != EXE_TWO || buf[EXE_3] != EXE_THREE || buf[EXE_4] != EXE_FOUR){
        return -1;
    }

    // entry point: the virtual address of the first instruction that should be executed
    // in bytes 24-27 of the executable
    uint32_t entry_point = 0;
    memcpy(&entry_point, buf + 24, 4);

    //****************** Set up page ********************//
    // find free PCB
    for(i = 0; i < PCB_ARRAY_SIZE; i++){
        if(pcb_array[i]->flag == 0){
            pre_pid = cur_pid;
            cur_pid = i;
            break;
        }
    }

    //printf("curpid: %d\n", cur_pid);
    //printf("cur_term_sch: %d\n", cur_term_sch);

    // No PCB is free
    if(i == PCB_ARRAY_SIZE){
        return -1;
    }

    set_up_page(cur_pid);
    flush_TLB();

    //*********************  Load file into memory *************************//
    if(file_read(MODE_ONE, (uint8_t *)PROGRAM_IMAGE_VIRTUAL_ADDR, FOUR_MB) == -1){
        return -1;
    }

    //***********************  Create PCB/ Open FDs ************************//
    pcb_array[cur_pid]->flag = 1;
    pcb_array[cur_pid]->process_id = i;
    pcb_array[cur_pid]->parent = pcb_array[pre_pid];

    // Store esp, used for ret in halt
    uint32_t esp;
    asm volatile("movl %%esp, %0":"=g"(esp));
    pcb_array[cur_pid]->parent_stack_ptr = esp;
    //pcb_array[cur_pid]->sch_esp = esp;

    // Store ebp, used for ret in halt
    uint32_t ebp;
    asm volatile("movl %%ebp, %0":"=g"(ebp));
    pcb_array[cur_pid]->parent_base_ptr = ebp;
    //pcb_array[cur_pid]->sch_ebp = ebp;

    // store arguments
    memset((int8_t*) pcb_array[cur_pid]->args, 0, BUF_SIZE);
    strcpy((int8_t*) pcb_array[cur_pid]->args, (const int8_t*) argu);

    pcb_array[cur_pid]->fd_array[0].file_op = &stdin_op;
    pcb_array[cur_pid]->fd_array[0].flags = 1;
    pcb_array[cur_pid]->fd_array[1].file_op = &stdout_op;
    pcb_array[cur_pid]->fd_array[1].flags = 1;

    if(new_term_excute == 1){
        new_term_excute = 0;
        pcb_array[cur_pid]->parent = pcb_array[cur_pid];
        pre_pid = cur_pid;

        pcb_t* prev_pcb = pcb_array[term[pre_term_index].cur_pid];
        prev_pcb->sch_ss0 = tss.ss0;
        prev_pcb->sch_esp0 = tss.esp0;

        cur_term_sch = cur_term_index;
        term[cur_term_index].exist = 1;
    }

    pcb_array[cur_pid]->terminal_index = cur_term_index;

    for(i = FILE_START; i < FD_MAX; i++){
        pcb_array[cur_pid]->fd_array[i].flags = 0;
        pcb_array[cur_pid]->fd_array[i].file_op = &null_op;
    }

    //****************  Prepare for context switch *******************//
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - cur_pid * EIGHT_KB - PAGE_BORDER;

    //pcb_t* cur_pcb = pcb_array[term[cur_term_index].cur_pid];

    //cur_pcb->sch_ss0 = tss.ss0;
    //cur_pcb->sch_esp0 = tss.esp0;

    sti();

    //**************** Push IRET context to stack/ IRET ************//
    asm volatile("                                                  \n\
        movw  $0x2B, %%ax                                           \n\
        movw  %%ax, %%ds                                            \n\
        pushl $0x002B               /* User DS */                   \n\
        pushl %1                    /* ESP */                       \n\
        pushfl                      /* EFLAGS */                    \n\
        popl %%eax                                                  \n\
        orl  $0x200, %%eax                                          \n\
        pushl %%eax                 /* EFLAGS OR 0x200 */           \n\
        pushl $0x0023               /* User CS */                   \n\
        pushl %0                    /* EIP */                       \n\
        iret                                                        \n\
        "
        : /* no outputs */
        : "r"(entry_point), "r"(ONE_TWO_EIGHT_MB + FOUR_MB - PAGE_BORDER)
        : "eax"
    );

    return 0;
}

/* int32_t read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: read */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    // check if the buffer is valid
    if(buf == NULL){
        return -1;
    }

    // check if fd is in range
    if(fd < FD_START || fd >= FD_MAX){
        return -1;
    }

    // check if file exists
    if(pcb_array[cur_pid]->fd_array[fd].flags == 0){
        return -1;
    }

    return pcb_array[cur_pid]->fd_array[fd].file_op->read(fd, buf, nbytes);
}

/* int32_t write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd, buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: write */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    // check if the buffer is valid
    if(buf == NULL){
        return -1;
    }

    // check if fd is in range
    if(fd < FD_START || fd >= FD_MAX){
        return -1;
    }

    // check if file exists
    if(pcb_array[cur_pid]->fd_array[fd].flags == 0){
        return -1;
    }

    return pcb_array[cur_pid]->fd_array[fd].file_op->write(fd, buf, nbytes);
}

/* int32_t open(const uint8_t* filename)
 * Inputs: filename
 * Return Value: >=0 - success, -1 - fail
 * Function: open */
int32_t open(const uint8_t* filename) {

    int32_t i;
    dentry_t dentry;

    // check if the filename can be found
    if(read_dentry_by_name(filename, &dentry) == -1){
        return -1;
    }

    // find if there is empty position in PCB
    for(i = FILE_START; i < FD_MAX; i++){
        if( pcb_array[cur_pid]->fd_array[i].flags == 0){
            pcb_array[cur_pid]->fd_array[i].inode = dentry.inode_index;
            pcb_array[cur_pid]->fd_array[i].file_pos = 0;
            pcb_array[cur_pid]->fd_array[i].flags = 1;
            break;
        }
    }

    // if PCB is full
    if(i == FD_MAX){
        return -1;
    }

    // check the file type and assign corresponding operations
    if(dentry.filetype == RTC_FILE_TYPE){
        pcb_array[cur_pid]->fd_array[i].file_op = &rtc_op;
    }
    else if(dentry.filetype == DIR_FILE_TYPE){
        pcb_array[cur_pid]->fd_array[i].file_op = &dir_op;
    }
    else if(dentry.filetype == REGULAR_FILE_TYPE){
        pcb_array[cur_pid]->fd_array[i].file_op = &file_op;
    }
    else{
        return -1;
    }

    //pcb_array[cur_pid]->fd_array[i].file_op->open(filename);

    return i;
}

/* int32_t close(int32_t fd)
 * Inputs: fd
 * Return Value: 0 - success, -1 - fail
 * Function: close */
int32_t close(int32_t fd) {

    int32_t ret;

    // check out of range
    if(fd < FD_START || fd >= FD_MAX){
        return -1;
    }

    // check if file exists
    if(pcb_array[cur_pid]->fd_array[fd].flags == 0){
        return -1;
    }


    ret = pcb_array[cur_pid]->fd_array[fd].file_op->close(fd);

    if(ret != -1 ){
        pcb_array[cur_pid]->fd_array[fd].flags = 0;
    }

    return ret;
}

/* int32_t getargs(uint8_t* buf, int32_t nbytes)
 * Inputs: buf, nbytes
 * Return Value: 0 - success, -1 - fail
 * Function: getargs */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    // check invalid input
    if(buf == NULL || nbytes <= 0){
        return -1;
    }

    strncpy((int8_t*) buf, (const int8_t*) pcb_array[cur_pid]->args, nbytes);

    return 0;
}

/* int32_t vidmap(uint8_t** screen_start)
 * Inputs: screen_start
 * Return Value: 0 - success, -1 - fail
 * Function: vidmap */
int32_t vidmap(uint8_t** screen_start) {
    if((uint32_t)screen_start < ONE_TWO_EIGHT_MB || (uint32_t)screen_start > ONE_TWO_EIGHT_MB + FOUR_MB){
        return -1;
    }

    /*
    if(cur_term_sch != cur_term_index){
        map_backing(term[cur_term_sch].term_vm_addr);
    }
    */

    map_video_memory_into_user();
    flush_TLB();
    *screen_start = (uint8_t*) VIDEO_MAP;

    /*
    if(cur_term_sch != cur_term_index){
        *screen_start = (uint8_t*) term[cur_term_sch].term_vm_addr;
    }
    */

    return (int32_t) VIDEO_MAP;
}


int32_t set_handler(int32_t signum, void* handler_address) {
    return 0;
}

int32_t sigreturn(void) {
    return 0;
}

/* Fucntions do nothing */

int32_t null_open(const uint8_t* file){
    return -1;
}

int32_t null_close(int32_t fd){
    return -1;
}

int32_t null_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t null_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
