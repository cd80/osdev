#include "harddisk.h"
#include "utility.h"
#include "helper_asm.h"
#include "console.h"

static HDDMANAGER g_hdd_manager;

BOOL initialize_hdd(void){
    init_mutex(&(g_hdd_manager.lock));

    g_hdd_manager.primary_interrupt_occur = FALSE;
    g_hdd_manager.secondary_interrupt_occur = FALSE;

    out1(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);
    out1(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0);

    if (read_hdd_information(TRUE, TRUE, &(g_hdd_manager.hdd_information)) == FALSE) {
        g_hdd_manager.hdd_detected = FALSE;
        g_hdd_manager.can_write = FALSE;
        return FALSE;
    }

    g_hdd_manager.hdd_detected = TRUE;
    if (memcmp(g_hdd_manager.hdd_information.model_number, "QEMU", 4) == 0) {
        g_hdd_manager.can_write = TRUE;
    }
    else {
        g_hdd_manager.can_write = FALSE;
    }
    return TRUE;
}

BOOL read_hdd_information(BOOL is_primary, BOOL is_master, HDDINFORMATION *hdd_information){
    WORD port_base;
    QWORD last_tick_count;
    BYTE status;
    BYTE drive_flag;
    WORD temp;
    BOOL wait_result;

    port_base = is_primary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;

    mutex_lock(&(g_hdd_manager.lock));

    if (wait_for_hdd_nobusy(is_primary) == FALSE) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }

    drive_flag = is_master ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;

    out1(port_base + HDD_PORT_INDEX_DRIVEANDHEAD, drive_flag);

    if (wait_for_hdd_ready(is_primary) == FALSE) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }

    set_hdd_interrupt_flag(is_primary, FALSE);

    out1(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY);

    wait_result = wait_for_hdd_interrupt(is_primary);

    status = read_hdd_status(is_primary);
    if ((wait_result == FALSE) ||
        ((status & HDD_STATUS_ERROR) == HDD_STATUS_ERROR)) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }

    for (int i = 0; i < 512 / 2; ++i) {
        ((WORD *)hdd_information)[i] = in2(port_base + HDD_PORT_INDEX_DATA);
    }
    swap_byte_in_word(hdd_information->model_number,
                    sizeof(hdd_information->model_number)/2);
    swap_byte_in_word(hdd_information->serial_number,
                    sizeof(hdd_information->serial_number)/2);

    mutex_unlock(&(g_hdd_manager.lock));
    return TRUE;
}

int read_hdd_sector(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, char *buf){
    WORD port_base;
    BYTE drive_flag;
    BYTE status;
    long read_count;
    BOOL wait_result;

    if ((g_hdd_manager.hdd_detected == FALSE) ||
        (sector_count <= 0) || (256 < sector_count) ||
        ((LBA + sector_count) >= g_hdd_manager.hdd_information.total_sectors)) {
        return 0;
    }

    port_base = is_primary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;

    mutex_lock(&(g_hdd_manager.lock));
    if (wait_for_hdd_nobusy(is_primary) == FALSE) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }

    out1(port_base + HDD_PORT_INDEX_SECTORCOUNT, sector_count);
    out1(port_base + HDD_PORT_INDEX_SECTORNUMBER, LBA);
    out1(port_base + HDD_PORT_INDEX_CYLINDERLSB, LBA >> 8);
    out1(port_base + HDD_PORT_INDEX_CYLINDERMSB, LBA >> 16);

    drive_flag = is_master ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    out1(port_base + HDD_PORT_INDEX_DRIVEANDHEAD, drive_flag | ((LBA >> 24) & 0x0F));

    if (wait_for_hdd_ready(is_primary) == FALSE) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }

    set_hdd_interrupt_flag(is_primary, FALSE);

    out1(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ);

    for (int i = 0; i < sector_count; ++i) {
        status = read_hdd_status(is_primary);
        if ((status & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            printf("[ERROR] Error in read_hdd_sector!!!\n");
            mutex_unlock(&(g_hdd_manager.lock));
            return i;
        }

        if ((status & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST) {
            wait_result = wait_for_hdd_interrupt(is_primary);
            set_hdd_interrupt_flag(is_primary, FALSE);
            if (wait_result == FALSE) {
                printf("[ERROR] Interrupt not happened in read_hdd_sector\n");
                mutex_unlock(&(g_hdd_manager.lock));
                return FALSE;
            }
        }

        for (int j = 0; j < (512 / 2); ++j) {
            ((WORD *)buf)[read_count++] = in2(port_base+HDD_PORT_INDEX_DATA);
        }
    }
    mutex_unlock(&(g_hdd_manager.lock));
    return sector_count;
}

int write_hdd_sector(BOOL is_primary, BOOL is_master, DWORD LBA, int sector_count, char *buf){
    WORD port_base;
    WORD temp;
    BYTE drive_flag;
    BYTE status;
    long read_count = 0;
    BOOL wait_result;

    if ((g_hdd_manager.can_write == FALSE) ||
        (sector_count <= 0) || (256 < sector_count) ||
        ((LBA + sector_count) >= g_hdd_manager.hdd_information.total_sectors)) {
        return 0;
    }

    port_base = is_primary ? HDD_PORT_PRIMARYBASE : HDD_PORT_SECONDARYBASE;

    if (wait_for_hdd_nobusy(is_primary) == FALSE) {
        return FALSE;
    }
    mutex_lock(&(g_hdd_manager.lock));

    out1(port_base + HDD_PORT_INDEX_SECTORCOUNT, sector_count);
    out1(port_base + HDD_PORT_INDEX_SECTORNUMBER, LBA);
    out1(port_base + HDD_PORT_INDEX_CYLINDERLSB, LBA >> 8);
    out1(port_base + HDD_PORT_INDEX_CYLINDERMSB, LBA >> 16);

    drive_flag = is_master ? HDD_DRIVEANDHEAD_LBA : HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    out1(port_base + HDD_PORT_INDEX_DRIVEANDHEAD, drive_flag | ((LBA >> 24) & 0x0F));

    if (wait_for_hdd_ready(is_primary) == FALSE) {
        mutex_unlock(&(g_hdd_manager.lock));
        return FALSE;
    }
    out1(port_base + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE);

    while (1) {
        status = read_hdd_status(is_primary);
        if ((status & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            mutex_unlock(&(g_hdd_manager.lock));
            return 0;
        }
        if ((status & HDD_STATUS_DATAREQUEST) == HDD_STATUS_DATAREQUEST) {
            break;
        }
        sleep(1);
    }

    for (int i = 0; i < sector_count; ++i) {
        set_hdd_interrupt_flag(is_primary, FALSE);
        for (int j = 0; j < 512 / 2; ++j) {
            out2(port_base + HDD_PORT_INDEX_DATA, ((WORD *)buf)[read_count++]);
        }
        status = read_hdd_status(is_primary);
        if ((status & HDD_STATUS_ERROR) == HDD_STATUS_ERROR) {
            mutex_unlock(&(g_hdd_manager.lock));
            return i;
        }

        if ((status & HDD_STATUS_DATAREQUEST) != HDD_STATUS_DATAREQUEST) {
            wait_result = wait_for_hdd_interrupt(is_primary);
            set_hdd_interrupt_flag(is_primary, FALSE);
            if (wait_result == FALSE) {
                mutex_unlock(&(g_hdd_manager.lock));
                return FALSE;
            }
        }
    }

    mutex_unlock(&(g_hdd_manager.lock));
    return sector_count;
}

void set_hdd_interrupt_flag(BOOL is_primary, BOOL flag){
    if (is_primary == TRUE) {
        g_hdd_manager.primary_interrupt_occur = flag;
    }
    else {
        g_hdd_manager.secondary_interrupt_occur = flag;
    }
}

static void swap_byte_in_word(WORD *data, int word_count){
    WORD temp;
    for (int i = 0; i < word_count; ++i) {
        temp = data[i];
        data[i] = (temp >> 8) | (temp << 8);
    }
}

static BYTE read_hdd_status(BOOL is_primary){
    if (is_primary == TRUE) {
        return in1(HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS);
    }
    return in1(HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS);
}

static BOOL is_hdd_busy(BOOL is_primary){ // ?
    return FALSE;
}

static BOOL is_hdd_ready(BOOL is_primary){ // ?
    return TRUE;
}

static BOOL wait_for_hdd_nobusy(BOOL is_primary){
    QWORD start_tick_count;
    BYTE status;

    start_tick_count = get_tick_count();
    while ((get_tick_count() - start_tick_count) <= HDD_WAITTIME) {
        status = read_hdd_status(is_primary);

        if ((status & HDD_STATUS_BUSY) != HDD_STATUS_BUSY) {
            return TRUE;
        }
        sleep(1);
    }
    return FALSE;
}

static BOOL wait_for_hdd_ready(BOOL is_primary){
    QWORD start_tick_count;
    BYTE status;
    start_tick_count = get_tick_count();

    while ((get_tick_count() - start_tick_count) <= HDD_WAITTIME) {
        status = read_hdd_status(is_primary);

        if ((status & HDD_STATUS_READY) == HDD_STATUS_READY) {
            return TRUE;
        }
        sleep(1);
    }
    printf("FAILED wait_for_hdd_ready\n");
    return FALSE;
}

static BOOL wait_for_hdd_interrupt(BOOL is_primary){
    QWORD start_tick_count;
    start_tick_count = get_tick_count();

    while ((get_tick_count() - start_tick_count) <= HDD_WAITTIME) {
        if ((is_primary == TRUE) &&
            (g_hdd_manager.primary_interrupt_occur == TRUE)) {
            return TRUE;
        }
        else if ((is_primary == FALSE) &&
                (g_hdd_manager.secondary_interrupt_occur == TRUE)) {
            return TRUE;
        }
    }
    return FALSE;
}