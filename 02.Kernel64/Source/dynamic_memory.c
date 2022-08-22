#include "dynamic_memory.h"
#include "utility.h"
#include "task.h"
#include "sync.h"

static DYNAMICMEMORYMANAGER dynamic_memory_manager;

BOOL initialize_dynamic_memory(void){
    QWORD dynamic_memory_size;
    QWORD max_level_count;
    QWORD block_remainder;
    BYTE *cur_bitmap_pos;
    int block_count_of_level, meta_block_count;

    dynamic_memory_size = calculate_dynamic_memory_size();
    meta_block_count = calculate_meta_block_count(dynamic_memory_size);

    dynamic_memory_manager.block_count_of_smallest_block =
            (dynamic_memory_size / DYNAMICMEMORY_MIN_SIZE) - meta_block_count;
    
    for (max_level_count = 0; 
        (dynamic_memory_manager.block_count_of_smallest_block >> max_level_count) > 0;
        ++max_level_count) {
        ;
    }
    dynamic_memory_manager.max_level_count = max_level_count;

    dynamic_memory_manager.allocated_block_list_index = (BYTE *)DYNAMICMEMORY_START_ADDRESS;
    for (int i = 0; i < dynamic_memory_manager.block_count_of_smallest_block; ++i) {
        dynamic_memory_manager.allocated_block_list_index[i] = 0xFF;
    }

    dynamic_memory_manager.bitmap_of_level = (BITMAP *)(DYNAMICMEMORY_START_ADDRESS + 
            (sizeof(BYTE) * dynamic_memory_manager.block_count_of_smallest_block));
    cur_bitmap_pos = ((BYTE *)dynamic_memory_manager.bitmap_of_level) +
            (sizeof(BITMAP) * dynamic_memory_manager.max_level_count);
    
    for (int i = 0; i < dynamic_memory_manager.max_level_count; ++i) {
        dynamic_memory_manager.bitmap_of_level[i].bitmap = cur_bitmap_pos;
        dynamic_memory_manager.bitmap_of_level[i].exist_bit_count = 0;
        block_count_of_level = dynamic_memory_manager.block_count_of_smallest_block >> i;

        for (int j = 0; j < block_count_of_level / 8; ++j) {
            *cur_bitmap_pos = 0x00;
            cur_bitmap_pos++;
        }

        if ((block_count_of_level % 8) != 0) {
            *cur_bitmap_pos = 0x00;
            block_remainder = block_count_of_level % 8;
            if ((block_remainder % 2) == 1) {
                *cur_bitmap_pos |= (DYNAMICMEMORY_EXIST << (block_remainder - 1));
                dynamic_memory_manager.bitmap_of_level[i].exist_bit_count = 1;
            }
            cur_bitmap_pos++;
        }
    }

    dynamic_memory_manager.start_address = DYNAMICMEMORY_START_ADDRESS +
                meta_block_count * DYNAMICMEMORY_MIN_SIZE;
    dynamic_memory_manager.end_address = calculate_dynamic_memory_size() + 
                DYNAMICMEMORY_START_ADDRESS;
    dynamic_memory_manager.used_size = 0;

    return TRUE;
}

void *allocate_memory(QWORD size){
    QWORD aligned_size;
    QWORD relative_address;
    long offset;
    int size_array_offset;
    int index_of_block_list;

    aligned_size = get_buddy_block_size(size);
    if (aligned_size == 0) {
        return NULL;
    }

    if (dynamic_memory_manager.start_address + dynamic_memory_manager.used_size +
        aligned_size > dynamic_memory_manager.end_address) {
        return NULL;
    }

    offset = allocate_buddy_block(aligned_size);
    if (offset == -1) {
        return NULL;
    }

    index_of_block_list = get_block_list_index_of_match_size(aligned_size);

    relative_address = aligned_size * offset;
    size_array_offset = relative_address / DYNAMICMEMORY_MIN_SIZE;
    dynamic_memory_manager.allocated_block_list_index[size_array_offset] = 
                (BYTE)index_of_block_list;
    dynamic_memory_manager.used_size += aligned_size;

    return (void *)(relative_address + dynamic_memory_manager.start_address);
}

BOOL free_memory(void *address){
    QWORD relative_address;
    int size_array_offset;
    QWORD block_size;
    int block_list_index;
    int bitmap_offset;

    if (address == NULL) {
        return FALSE;
    }

    relative_address = ((QWORD)address) - dynamic_memory_manager.start_address;
    size_array_offset = relative_address / DYNAMICMEMORY_MIN_SIZE;

    if (dynamic_memory_manager.allocated_block_list_index[size_array_offset] == 0xFF) {
        return FALSE;
    }

    block_list_index = (int)dynamic_memory_manager.allocated_block_list_index[size_array_offset];
    dynamic_memory_manager.allocated_block_list_index[size_array_offset] = 0xFF;

    block_size = DYNAMICMEMORY_MIN_SIZE << block_list_index;

    bitmap_offset = relative_address / block_size;
    if (free_buddy_block(block_list_index, bitmap_offset) == TRUE) {
        dynamic_memory_manager.used_size -= block_size;
        return TRUE;
    }

    return FALSE;
}

void get_dynamic_memory_information(QWORD *dynamic_memory_start_address,
                QWORD *dynamic_memory_total_size, QWORD *meta_data_size,
                QWORD *used_memory_size){
    *dynamic_memory_start_address = DYNAMICMEMORY_START_ADDRESS;
    *dynamic_memory_total_size = calculate_dynamic_memory_size();
    *meta_data_size = calculate_meta_block_count(*dynamic_memory_total_size) * 
                    DYNAMICMEMORY_MIN_SIZE;
    *used_memory_size = dynamic_memory_manager.used_size;
}

DYNAMICMEMORYMANAGER *get_dynamic_memory_manager(void){
    return &dynamic_memory_manager;
}

static QWORD calculate_dynamic_memory_size(void){
    QWORD ram_size;
    ram_size = (get_total_ram_size() * 1024 * 1024);
    if (ram_size > ((QWORD)3 * 1024 * 1024 * 1024)) {
        ram_size = ((QWORD)3 * 1024 * 1024 * 1024);
    }
    return ram_size - DYNAMICMEMORY_START_ADDRESS;
}

static int calculate_meta_block_count(QWORD dynamic_ram_size){
    long block_count_of_smallest_block;
    DWORD size_of_allocated_block_list_index;
    DWORD size_of_bitmap;

    block_count_of_smallest_block = dynamic_ram_size / DYNAMICMEMORY_MIN_SIZE;
    size_of_allocated_block_list_index = block_count_of_smallest_block * sizeof(BYTE);

    size_of_bitmap = 0;
    for (long i = 0; (block_count_of_smallest_block >> i) > 0; ++i) {
        size_of_bitmap += sizeof(BITMAP);
        size_of_bitmap += ((block_count_of_smallest_block >> i) + 7) / 8;
    }

    return (size_of_allocated_block_list_index + size_of_bitmap +
            DYNAMICMEMORY_MIN_SIZE - 1) / DYNAMICMEMORY_MIN_SIZE;
}

static int allocate_buddy_block(QWORD aligned_size){
    int block_list_index, free_offset;
    int block_idx;
    BOOL prev_flag;

    block_list_index = get_block_list_index_of_match_size(aligned_size);
    if (block_list_index == -1) {
        return -1;
    }

    prev_flag = lock_system();

    for (block_idx = block_list_index; block_idx < dynamic_memory_manager.max_level_count; ++block_idx) {
        free_offset = find_free_block_in_bitmap(block_idx);
        if (free_offset != -1) {
            break;
        }
    }
    if (free_offset == -1) {
        unlock_system(prev_flag);
        return -1;
    }

    set_flag_in_bitmap(block_idx, free_offset, DYNAMICMEMORY_EMPTY);

    if (block_idx > block_list_index) {
        for (block_idx = block_idx - 1; block_idx >= block_list_index; --block_idx) {
            set_flag_in_bitmap(block_idx, free_offset * 2, DYNAMICMEMORY_EMPTY);
            set_flag_in_bitmap(block_idx, free_offset * 2 + 1, DYNAMICMEMORY_EXIST);
            free_offset = free_offset * 2;
        }
    }

    unlock_system(prev_flag);

    return free_offset;
}

static QWORD get_buddy_block_size(QWORD size){
    for (long i = 0; i < dynamic_memory_manager.max_level_count; ++i) {
        if (size <= (DYNAMICMEMORY_MIN_SIZE << i)) {
            return (DYNAMICMEMORY_MIN_SIZE << i);
        }
    }
    return 0;
}

static int get_block_list_index_of_match_size(QWORD aligned_size){
    for (int i = 0; i < dynamic_memory_manager.max_level_count; ++i) {
        if (aligned_size <= (DYNAMICMEMORY_MIN_SIZE << i)) {
            return i;
        }
    }
    return -1;
}

static int find_free_block_in_bitmap(int block_list_index){
    int max_count;
    BYTE *bitmap_base;
    QWORD *bitmap;

    if (dynamic_memory_manager.bitmap_of_level[block_list_index].exist_bit_count == 0) {
        return -1;
    }

    max_count = dynamic_memory_manager.block_count_of_smallest_block >> block_list_index;
    bitmap_base = dynamic_memory_manager.bitmap_of_level[block_list_index].bitmap;

    for (int i = 0; i < max_count; ) {
        if (((max_count - i) / 64) > 0) {
            bitmap = (QWORD *)&(bitmap_base[i / 8]);
            if (*bitmap == 0) {
                i += 64;
                continue;
            }
        }

        if ((bitmap_base[i / 8] & (DYNAMICMEMORY_EXIST << (i % 8))) != 0) {
            return i;
        }
        ++i;
    }
    return -1;
}

static void set_flag_in_bitmap(int block_list_index, int offset, BYTE flag){
    BYTE *bitmap;
    bitmap = dynamic_memory_manager.bitmap_of_level[block_list_index].bitmap;
    if (flag == DYNAMICMEMORY_EXIST) {
        if ((bitmap[offset / 8] & (0x01 << (offset % 8))) == 0) {
            dynamic_memory_manager.bitmap_of_level[block_list_index].exist_bit_count++;
        }
        bitmap[offset / 8] |= (0x01 << (offset % 8));
    }
    else {
        if ((bitmap[offset/8] & (0x01 << (offset % 8))) != 0) {
            dynamic_memory_manager.bitmap_of_level[block_list_index].exist_bit_count--;
        }
        bitmap[offset / 8] &= ~(0x01 << (offset % 8));
    }
}

static BOOL free_buddy_block(int block_list_index, int block_offset){
    int buddy_block_offset;
    BYTE flag;
    BOOL prev_flag;

    prev_flag = lock_system();

    for (int i = block_list_index; i < dynamic_memory_manager.max_level_count; ++i) {
        set_flag_in_bitmap(i, block_offset, DYNAMICMEMORY_EXIST);

        if ((block_offset % 2) == 0) {
            buddy_block_offset = block_offset + 1;
        }
        else {
            buddy_block_offset = block_offset - 1;
        }
        flag = get_flag_in_bitmap(i, buddy_block_offset);
        if (flag == DYNAMICMEMORY_EMPTY) {
            break;
        }

        set_flag_in_bitmap(i, buddy_block_offset, DYNAMICMEMORY_EMPTY);
        set_flag_in_bitmap(i, block_offset, DYNAMICMEMORY_EMPTY);

        block_offset = block_offset / 2;
    }
    unlock_system(prev_flag);
    return TRUE;
}

static BYTE get_flag_in_bitmap(int block_list_index, int offset){
    BYTE *bitmap;
    bitmap = dynamic_memory_manager.bitmap_of_level[block_list_index].bitmap;
    if ((bitmap[offset / 8] & (0x01 << (offset % 8))) != 0x00) {
        return DYNAMICMEMORY_EXIST;
    }
    return DYNAMICMEMORY_EMPTY;
}