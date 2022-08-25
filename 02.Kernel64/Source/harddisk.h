#ifndef __harddisk_h__
#define __harddisk_h__

#include "types.h"
#include "sync.h"

#define HDD_PORT_PRIMARYBASE    0x1F0
#define HDD_PORT_SECONDARYBASE  0x170

#define HDD_PORT_INDEX_DATA             0x00
#define HDD_PORT_INDEX_SECTORCOUNT      0x02
#define HDD_PORT_INDEX_SECTORNUMBER     0x03
#define HDD_PORT_INDEX_CYLINDERLSB      0x04
#define HDD_PORT_INDEX_CYLINDERMSB      0x05
#define HDD_PORT_INDEX_DRIVEANDHEAD     0x06
#define HDD_PORT_INDEX_STATUS           0x07
#define HDD_PORT_INDEX_COMMAND          0x07
#define HDD_PORT_INDEX_DIGITALOUTPUT    0x206

#define HDD_COMMAND_READ        0x20
#define HDD_COMMAND_WRITE       0x30
#define HDD_COMMAND_IDENTIFY    0xEC

#define HDD_STATUS_ERROR            0x01
#define HDD_STATUS_INDEX            0x02
#define HDD_STATUS_CORRECTEDDATA    0x04
#define HDD_STATUS_DATAREQUEST      0x08
#define HDD_STATUS_SEEKCOMPLETE     0x10
#define HDD_STATUS_WRITEFAULT       0x20
#define HDD_STATUS_READY            0x40
#define HDD_STATUS_BUSY             0x80

#define HDD_DRIVEANDHEAD_LBA        0xE0
#define HDD_DRIVEANDHEAD_SLAVE      0x10

#define HDD_DIGITALOUTPUT_RESET             0x04
#define HDD_DIGITALOUTPUT_DISABLEINTERRUPT  0x01

#define HDD_WAITTIME            500
#define HDD_MAXBULKSECTORCOUNT  256

#pragma pack(push, 1)

typedef struct _HDDINFORMATION {
    WORD configuration;
    WORD number_of_cylinder;
    WORD reserved1;

    WORD number_of_head;
    WORD unformatted_bytes_per_track;
    WORD unformatted_bytes_per_sector;

    WORD number_of_sector_per_cylinder;
    WORD inter_sector_gap;
    WORD bytes_in_phase_lock;
    WORD number_of_vendor_unique_status_word;

    WORD serial_number[10];
    WORD controler_type;
    WORD buffer_size;
    WORD number_of_ecc_bytes;
    WORD firmware_revision[4];

    WORD model_number[20];
    WORD reserved2[13];

    DWORD total_sectors;
    WORD reserved3[196];
} HDDINFORMATION;

#pragma pack(pop)

typedef struct _HDDMANAGER {
    BOOL hdd_detected;
    BOOL can_write;

    volatile BOOL primary_interrupt_occur;
    volatile BOOL secondary_interrupt_occur;
    MUTEX lock;

    HDDINFORMATION hdd_information;
} HDDMANAGER;

BOOL initialize_hdd(void);
BOOL read_hdd_information(BOOL is_primary, BOOL is_master, HDDINFORMATION *hdd_information);
int read_hdd_sector(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, BYTE *buf);
int write_hdd_sector(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, BYTE *buf);
void set_hdd_interrupt_flag(BOOL is_primary, BOOL flag);

static void swap_byte_in_word(WORD *data, int word_count);
static BYTE read_hdd_status(BOOL is_primary);
static BOOL is_hdd_busy(BOOL is_primary);
static BOOL is_hdd_ready(BOOL is_primary);
static BOOL wait_for_hdd_nobusy(BOOL is_primary);
static BOOL wait_for_hdd_ready(BOOL is_primary);
static BOOL wait_for_hdd_interrupt(BOOL is_primary);

#endif