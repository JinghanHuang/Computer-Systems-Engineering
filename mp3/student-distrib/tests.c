#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "file.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "system_call.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER     \
    printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)   \
    printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
    /* Use exception #15 for assertions, otherwise
       reserved by Intel */
    asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
/*
int idt_test(){
    TEST_HEADER;

    int i;
    int result = PASS;
    for (i = 0; i < 10; ++i){
        if ((idt[i].offset_15_00 == NULL) &&
            (idt[i].offset_31_16 == NULL)){
            assertion_failure();
            result = FAIL;
        }
    }


    return result;
}
*/

// add more tests here

/* Paging Structure Test
 *
 * Check values in Paging
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h
 */
 /*
int paging_structure_test(){
    TEST_HEADER;

    uint32_t i;
    int result = PASS;

    // Values contained in your paging structures
    if(page_directory[0].pde_4kb.page_table_base_addr != (uint32_t) (&page_table) >> ADDR_SHIFT || page_directory[0].pde_4kb.present != 1){
        assertion_failure();
        result = FAIL;
    }

    if(page_directory[1].pde_4mb.page_base_addr != (uint32_t) KERNEL_ADDR >> ADDR_SHIFT_4MB ||
        page_directory[1].pde_4mb.page_size != 1 || page_directory[1].pde_4mb.present != 1){
        assertion_failure();
        result = FAIL;
    }

    i = VIDEO_MEM_ADDR >> ADDR_SHIFT;
    if(page_table[i].present != 1 || page_table[i].read_write != 1 || page_table[i].page_base_addr !=  i){
        assertion_failure();
        result = FAIL;
    }

    return result;
}
*/

/* Paging Def Valid Test
 *
 * Dereferencing different address ranges with paging turned on
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h
 */
 /*
int paging_def_valid_test() {
    TEST_HEADER;

    uint32_t deference;
    int result = PASS;

    deference = *((uint32_t *) VIDEO_MEM_ADDR);
    deference = *((uint32_t *) KERNEL_ADDR);

    return result;
}
*/

/* Paging Def Invalid Test
 *
 * Dereferencing different address ranges with paging turned on
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h
 */
 /*
int paging_def_invalid_test() {
    TEST_HEADER;

    uint32_t deference;
    int result = PASS;

    deference = *((uint32_t *) (KERNEL_ADDR * 2));

    return result;
}
*/

/* Divide Zero Test
 *
 * Dereferencing different address ranges with paging turned on
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h
 */
 /*
int divide_zero_test(){
    TEST_HEADER;
    int i;
    i = 5/0;
    return 0;
}
*/

/* Checkpoint 2 tests */
/* List All Files Test
 *
 * list all the file names, types and sizes; test open and close directory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load File, File definition
 * Files: file.h
 */
 /*
int list_all_files_test(const uint8_t * name) {
    TEST_HEADER;

	int i;
    uint8_t buf[32];
	memset((void *)buf, 0, 32);

	// open directory
    if(dir_open(name) == -1){
		printf("%s", "DIRECTORY OPEN FAIL");
		printf("\n");
        return FAIL;
    }
	printf("%s", "DIRECTORY OPEN PASS");
	printf("\n");

	// read directory
    while(dir_read(0, buf, FILENAME_SIZE) != 0){
		printf("%s", "file_name: ");
		for(i = 0; i < FILENAME_SIZE; i++){
			putc(buf[i]);
		}
		//read_file_by_name_test(buf);
		printf("%s", ", file_type: ");
		printf("%d, ", cur_dentry.filetype);
		printf("%s", "file_size: ");
		printf("%d", cur_dir_inode->length);
		printf("\n");
		memset((void *)buf, 0, FILENAME_SIZE);
    }

	// close directory
	dir_close(0);
	if(cur_dir_inode != NULL){
		printf("%s", "DIRECTORY CLOSE FAIL");
		printf("\n");
        return FAIL;
	}
	printf("%s", "DIRECTORY CLOSE PASS");
	printf("\n");

    return PASS;
}
*/

/* Read File By Name Test
 *
 * read file by name
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load File, File definition
 * Files: file.h
 */
 /*
int read_file_by_name_test(const uint8_t * name) {
    TEST_HEADER;

	int i;

	// open file
	if(file_open(name) == -1){
		printf("%s", "FILE OPEN FAIL");
		printf("\n");
        return FAIL;
    }
	printf("%s", "FILE OPEN PASS");
	printf("\n");

	uint8_t buf[cur_file_inode->length];
	memset((void *)buf, 0, cur_file_inode->length);

	// read file
	if(file_read(0, buf, 0) == 0){
        return FAIL;
    }
	for(i = 0; i < cur_file_inode->length; i++){
		//if(buf[i] != 0){
		putc(buf[i]);
		//}
	}
	printf("\n");
	printf("file_name: %s\n", name);

	// close file
    file_close(0);
	if(cur_file_inode != NULL){
		printf("%s", "FILE CLOSE FAIL");
		printf("\n");
        return FAIL;
	}
	printf("%s", "FILE CLOSE PASS");
	printf("\n");

    return PASS;
}
*/

/* terminal hello Test
 * motivated by the provided hello function
 * Inputs: None
 * Outputs: PASS, actually won't show on screen
 * Side Effects: None
 */
 /*
int test_hello ()
{
	char buf[KEY_BUF_SIZE];//128 is max size
	enter_flag = 0;

	terminal_write(1, (uint8_t*)"Hello, ", 7);//count amnually, value doesn't matter actually.
	while(!enter_flag);
	terminal_write(1, (uint8_t*)"ECE391 bless you, ", 18);//as prev comment
	terminal_read(0, buf, KEY_BUF_SIZE);
	terminal_write(1, buf, KEY_BUF_SIZE);//128not matter.

    return PASS;
}
*/

/* trc freq change Test
 * change the printing frequency automatically. The entire test should take
   ten seconds. use a stop watch.
 * Inputs: None
 * Outputs: None
 * Side Effects: prints to the terminal at the rtc intterrupt rate
 */

/*
void rtc_freqs(){
  int32_t i = 1;
	int32_t * buffer;
  *buffer = 2;

  //while loop goes to one frequency rate higher than rtc allows to test
  //all frequncies
  while((*buffer) < 2048){

    rtc_write(1, buffer, BYTE_SIZE);
	  while(i%(*buffer) != 0){
      	int a = rtc_read(0, buffer, BYTE_SIZE);
	 	    if(a==0){
			     putc('1');
           i++;
	      }
     }
     clear();
     i=1;
     *buffer = (*buffer)*2;
  }
}
*/


/*RTC_OPEN and RTC_CLOSE tests
 * calls both open and close and checks if they return 0
   if they do then it will print to the terminal showing that the
   open and close functions changed the frequency rate to 2 Hz
 * Inputs: None
 * Outputs: None
 * Side Effects: prints to the terminal at the rtc intterrupt rate
 */
 /*
void rtc_open_close(){
    set_Freq(1024);  // set the Frequency to 1024Hz to show that open changes Hz
    int b = 1;
    b = RTC_open((const uint8_t *)'1');
    //check if the open fuction worked successfully
    if(b==0){
      //print to the screen at the open rate
      while(1){
        int a = RTC_read(0, 0, BYTE_SIZE);
        if(a==0){
           putc('1');
         }
      }
    }else{
      //if open failed print 0 to the screen
           putc('0');
    }
}
*/

/* Checkpoint 3 tests */
/*
int execute_test(const uint8_t* command) {
    TEST_HEADER;

    int32_t ret;

    ret = execute(command);

    if(ret == -1){
        return FAIL;
    }
    return PASS;
}
*/




/* Checkpoint 4 tests */
/*
void rtc_system_test() {

    int32_t fd;
    int32_t ret_val;
    ret_val = 32;

    fd = open("rtc");
    printf("%d\n", fd);
    write(fd, &ret_val, 4);
    printf("%d\n", ret_val);
}
*/

/* Checkpoint 5 tests */
void pingpong_test() {

    //execute((const uint8_t*) "shell");
    execute((const uint8_t*) "pingpong");

    int32_t ret_val;
    ret_val = 32;

    write(1, (uint8_t*)"Starting 391 Shell\n", 1024);
}

/* Test suite entry point */
void launch_tests(){
    /* Checkpoint 1 tests */
    //TEST_OUTPUT("idt_test", idt_test());
    // launch your tests here
    //TEST_OUTPUT("paging_structure_test", paging_structure_test());
    //TEST_OUTPUT("paging_def_valid_test", paging_def_valid_test());
    //TEST_OUTPUT("paging_def_invalid_test", paging_def_invalid_test());
    //TEST_OUTPUT("divide_zero_test", divide_zero_test());

    /* Checkpoint 2 tests */
	  //TEST_OUTPUT("list_all_files_test", list_all_files_test((const uint8_t *)"."));
    //TEST_OUTPUT("read_file_by_name_test", read_file_by_name_test((const uint8_t *)"frame0.txt"));
	  //test_hello();
	  //rtc_freqs();
	  //rtc_open_close();

    /* Checkpoint 3 tests */
    //TEST_OUTPUT("execute_test", execute_test((const uint8_t*) "ls"));

    /* Checkpoint 4 tests */
    //rtc_system_test();
    /* Checkpoint 6 tests */
    //pingpong_test();
}
