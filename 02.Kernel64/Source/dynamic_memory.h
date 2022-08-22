#ifndef __dynamic_memory_h__
#define __dynamic_memory_h__

#include "types.h"
#include "task.h"

#define DYNAMICMEMORY_START_ADDRESS ((TASK_STACKPOOLADDRESS + \
    (TASK_STACKSIZE * TASK_MAXCOUNT) + 0xfffff) & 0xfffffffffff00000)

#define DYNAMICMEMORY_MIN_SIZE  (1 * 1024)

#define DYNAMICMEMORY_EXIST     0x01
#define DYNAMICMEMORY_EMPTY     0x00

typedef struct _BITMAP {
    BYTE *bitmap;
    QWORD exist_bit_count;
} BITMAP;

typedef struct _DYNAMICMEMORYMANAGER {
    int max_level_count;
    int block_count_of_smallest_block;
    QWORD used_size;

    QWORD start_address;
    QWORD end_address;

    BYTE *allocated_block_list_index;
    BITMAP *bitmap_of_level;
} DYNAMICMEMORYMANAGER;

BOOL initialize_dynamic_memory(void);
void *allocate_memory(QWORD size);
BOOL free_memory(void *address);
void get_dynamic_memory_information(QWORD *dynamic_memory_start_address,
                QWORD *dynamic_memory_total_size, QWORD *meta_data_size,
                QWORD *used_memory_size);
DYNAMICMEMORYMANAGER *get_dynamic_memory_manager(void);

static QWORD calculate_dynamic_memory_size(void);
static int calculate_meta_block_count(QWORD dynamic_ram_size);
static int allocate_buddy_block(QWORD aligned_size);
static QWORD get_buddy_block_size(QWORD size);
static int get_block_list_index_of_match_size(QWORD aligned_size);
static int find_free_block_in_bitmap(int block_list_index);
static void set_flag_in_bitmap(int block_list_index, int offset, BYTE flag);
static BOOL free_buddy_block(int block_list_index, int block_offset);
static BYTE get_flag_in_bitmap(int block_list_index, int offset);

#endif