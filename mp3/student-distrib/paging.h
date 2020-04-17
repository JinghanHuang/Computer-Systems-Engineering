#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

#define ENTRY_NUMBER    1024
#define PAGE_SIZE       4096
#define ADDR_SHIFT      12		 // We store address in bit 13-31
#define ADDR_SHIFT_4MB  22
#define KERNEL_ADDR     0x400000 // 4MB
#define VIDEO_MEM_ADDR  0xB8000  // Reference: Piazza Question #756: We can find video memory start address in lib.c
#define TERMINAL_ONE_VIDEO_ADDR  0xB9000
#define TERMINAL_TWO_VIDEO_ADDR  0xBA000
#define TERMINAL_THREE_VIDEO_ADDR  0xBB000
#define TERMINAL_NUM    3

#define FOUR_MB          0x400000 // 4MB
#define EIGHT_MB         0x800000 // 8MB
#define PI_INDEX        32 // 128MB / 4MB = 32 -> the program image index

#define PROGRAM_IMAGE_VIRTUAL_ADDR 0x08048000
#define ONE_TWO_EIGHT_MB 0x08000000
#define FOUR_KB    4096
#define VIDEO_MAP  ONE_TWO_EIGHT_MB + FOUR_MB + FOUR_KB;
#define VM_PD_IDX  33

// Initialize paging
extern void paging_init();

// functions which change registers
void store_pd_addr(uint32_t page_directory_addr);
void enable_paging();
void enable_4mb_paging();

// Paging operation functions
void flush_TLB();
void set_up_page(uint32_t pid);
void map_video_memory_into_user();

// Termianl Switch functions
void switch_terminal_video(uint8_t dest, uint8_t src);

void map_backing(uint32_t terminal_video_memory_addr);

// Struct format reference: IA32-ref-manual-vol-3.pdf 3-24 ~ 3-25

// struct of Page-Table Entries for 4-KByte Pages and 32-Bit Physical Addresses
typedef union page_table_entry {
    uint32_t val;
    struct {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1; // 1 for user, 0 for supervisor
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t page_table_attri_idx       : 1;
        uint32_t global_page                : 1;
        uint32_t for_system_programmer_use  : 3;
        uint32_t page_base_addr             : 20;
    } __attribute__ ((packed));
} pte_t;

// struct of Page-Directory Entries for 4-KByte Pages and 32-Bit Physical Addresses
typedef struct __attribute__((packed)) page_directory_entry_4kb {
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_supervisor    : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disabled     : 1;
    uint32_t accessed           : 1;
    uint32_t reserved           : 1; // set to 0
    uint32_t page_size          : 1; // 0 indicates 4 KBytes
    uint32_t global_page        : 1; // Ignored
    uint32_t for_system_programmer_use : 3;
    uint32_t page_table_base_addr      : 20;
} pde_4kb_t;

// struct of Page-Directory Entries for 4-MByte Pages and 32-Bit Addresses
typedef struct __attribute__((packed)) page_directory_entry_4mb {
    uint32_t present            : 1;
    uint32_t read_write         : 1;
    uint32_t user_supervisor    : 1;
    uint32_t write_through      : 1;
    uint32_t cache_disabled     : 1;
    uint32_t accessed           : 1;
    uint32_t dirty              : 1;
    uint32_t page_size          : 1; // 1 indicates 4 MBytes
    uint32_t global_page                : 1;
    uint32_t for_system_programmer_use  : 3;
    uint32_t page_table_attri_idx       : 1;
    uint32_t reserved                   : 9;
    uint32_t page_base_addr             : 10;
} pde_4mb_t;

typedef union page_directory_entry {
    uint32_t val;
    pde_4kb_t pde_4kb;
    pde_4mb_t pde_4mb;
} pde_t;

// declare page directory and page table
extern pde_t page_directory[ENTRY_NUMBER];
extern pte_t page_table[ENTRY_NUMBER];
extern pte_t vidmap_table[ENTRY_NUMBER];

#endif /* _PAGING_H */
