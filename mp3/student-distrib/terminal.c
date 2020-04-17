#include "terminal.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "multi_terminal.h"
#include "schedule.h"


file_op_t stdin_op;
file_op_t stdout_op;


/* Func: terminal_init()
 * init the terminals
 * input: none
 * output: none
 *
 */
void terminal_init(){


    stdin_op.open = &terminal_open;
    stdin_op.close = &terminal_close;
    stdin_op.read = &terminal_read;
    stdin_op.write = &terminal_no_write;

    stdout_op.open = &terminal_open;
    stdout_op.close = &terminal_close;
    stdout_op.read = &terminal_no_read;
    stdout_op.write = &terminal_write;

    return;
}


/* Fucntions do nothing */
int32_t terminal_no_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

int32_t terminal_no_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* Func: terminal_open()
 * open the terminal, actually do nothing.
 * input: none
 * output: none
 *
 */
int32_t terminal_open(const uint8_t* file){
    return -1;
}
/* Func: terminal_close()
 * clear terminal variables, actually do nothing
 * input: none
 * output: none
 *
 */

int32_t terminal_close(int32_t fd){
    return -1;
}
/* Func: terminal_read()
 * read from keyboard buffer into buf, return num of bytes read
 * input: destination buffer pointer and size to read into(bytes)
 * output: number  transfered
 *
 *
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
      // check if buf is valid
      if(buf == NULL){
          return -1;
      }

      // check enter
      while(term[cur_term_sch].enter_flag == 0);

      //if(cur_term_sch != cur_term_index){
          //return -1;
      //}


      cli();
      int32_t i;
      int8_t* buff = (int8_t*) buf;

      // copy buffer
      for (i = 0; i < nbytes; i++) {
          buff[i] = key_buffer[i];
          key_buffer[i] = '\0';
          if (buff[i] == '\n'){
              break;
          }
      }

      bufi = 0;
      term[cur_term_sch].enter_flag = 0;
      sti();

      return i;
}
/* Func: terminal_write()
 * write to screen from buf, return num of bytes write or -1
 * input: source buffer pointer and its size(bytes)
 * output: number displayed
 *
 *
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
      cli();
      int i;        // iterator

      // write buffer
      for (i = 0; i < nbytes; i++) {
          if(cur_term_sch == cur_term_index){
              putc(((uint8_t*)buf)[i]);
          }
          else{
              putc_backing(((uint8_t*)buf)[i]);
          }
      }

      sti();
      return i;
}
