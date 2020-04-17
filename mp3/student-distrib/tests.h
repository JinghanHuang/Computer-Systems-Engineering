#ifndef TESTS_H
#define TESTS_H

#include "types.h"

// test launcher
void launch_tests();

/* Checkpoint 1 tests */
/*
// test interrupt describer table
int idt_test();

// test paging initialization
int paging_structure_test();

// test dereference valid address
int paging_def_valid_test();

// test dereference invalid address
int paging_def_invalid_test();

// test divide exception
//int divide_zero_test();
*/

/* Checkpoint 2 tests */

/*
// list all files
int list_all_files_test(const uint8_t * name);

// read file by name
int read_file_by_name_test(const uint8_t * name);

// test terminal
int test_hello ();

// test rtc
void rtc_freqs();
void rtc_open_close();
*/

/* Checkpoint 3 tests */
//int execute_test(const uint8_t* command)


/* Checkpoint 4 tests */
//void rtc_system_test();

/* Checkpoint 5 tests */
void pingpong_test();

#endif /* TESTS_H */
