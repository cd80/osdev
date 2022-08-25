#ifndef __filesystem_h__
#define __filesystem_h__

#include "types.h"
#include "sync.h"
#include "harddisk.h"

#define FILESYSTEM_SIGNATURE        0xfeedcd80
#define FILESYSTEM_SECTORSPERCLUSTER    8
#define FILESYSTEM_LASTCLUSTER          0xFFFFFFFF
#define FILESYSTEM_FREECLUSTER          0
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT   ((FILESYSTEM_SECTORSPERCLUSTER * 512 ) / sizeof(DIRECTORYENTRY))
#define FILESYSTEM_CLUSTERSIZE          (FILESYSTEM_SECTORSPERCLUSTER * 512)

#define FILESYSTEM_MAXFILENAMELENGTH    24

typedef BOOL (*READ_HDD_INFORMATION)(BOOL is_primary, BOOL is_master, HDDINFORMATION *hdd_information);
typedef int (*READ_HDD_SECTOR)(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, BYTE *buf);
typedef int (*WRITE_HDD_SECTOR)(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, BYTE *buf);

#pragma pack(push, 1)
typedef struct _PARTITION {
    BYTE bootable_flag;
    BYTE starting_chs_address[3];
    BYTE partition_type;
    BYTE ending_chs_address[3];
    DWORD starting_lba_address;
    DWORD size_in_sector;
} PARTITION;

typedef struct _MBR {
    BYTE boot_code[430];

    DWORD signature;
    DWORD reserved_sector_count;
    DWORD cluster_link_sector_count;
    DWORD total_cluster_count;

    PARTITION partition[4];

    BYTE boot_loader_signature[2];
} MBR;

typedef struct _DIRECTORYENTRY {
    char file_name[FILESYSTEM_MAXFILENAMELENGTH];
    DWORD file_size;
    DWORD start_cluster_index;
} DIRECTORYENTRY;

#pragma pack(pop)

typedef struct _FILESYSTEMMANAGER {
    BOOL is_mounted;
    
    DWORD reserved_sector_count;
    DWORD cluster_link_area_start_address;
    DWORD cluster_link_area_size;
    DWORD data_area_start_address;

    DWORD total_cluster_count;

    DWORD last_allocated_cluster_link_sector_offset;

    MUTEX lock;
} FILESYSTEMMANAGER;

BOOL initialize_file_system(void);
BOOL format(void);
BOOL mount(void);
BOOL get_hdd_information(HDDINFORMATION *hdd_information);
BOOL read_cluster_link_table(DWORD offset, BYTE *buf);
BOOL write_cluster_link_table(DWORD offset, BYTE *buf);
BOOL read_cluster(DWORD offset, BYTE *buf);
BOOL write_cluster(DWORD offset, BYTE *buf);
DWORD find_free_cluster(void);
BOOL set_cluster_link_data(DWORD cluster_index, DWORD data);
BOOL get_cluster_link_data(DWORD cluster_index, DWORD *data);
int find_free_directory_entry(void);
BOOL set_directory_entry_data(int index, DIRECTORYENTRY *entry);
BOOL get_directory_entry_data(int index, DIRECTORYENTRY *entry);
int find_directory_entry(const char *filename, DIRECTORYENTRY *entry);
void get_file_system_information(FILESYSTEMMANAGER *fs_manager);

#endif