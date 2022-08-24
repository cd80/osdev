#include "filesystem.h"
#include "harddisk.h"
#include "dynamic_memory.h"

static FILESYSTEMMANAGER g_fs_manager;
static BYTE g_temp_buf[FILESYSTEM_SECTORSPERCLUSTER * 512];

READ_HDD_INFORMATION gf_read_hdd_information = NULL;
READ_HDD_SECTOR gf_read_hdd_sector = NULL;
WRITE_HDD_SECTOR gf_write_hdd_sector = NULL;

BOOL initialize_file_system(void) {
    memset(&g_fs_manager, 0, sizeof(g_fs_manager));
    init_mutex(&(g_fs_manager.lock));

    if (initialize_hdd() == TRUE) {
        gf_read_hdd_information = read_hdd_information;
        gf_read_hdd_sector = read_hdd_sector;
        gf_write_hdd_sector = write_hdd_sector;
    }
    else {
        return FALSE;
    }

    return mount();
}

BOOL format(void) {
    HDDINFORMATION *hdd;
    MBR *mbr;
    DWORD total_sector_count, remain_sector_count;
    DWORD max_cluster_count, cluster_count;
    DWORD cluster_link_sector_count;
    
    mutex_lock(&(g_fs_manager.lock));

    hdd = (HDDINFORMATION *)g_temp_buf;
    if (gf_read_hdd_information(TRUE, TRUE, hdd) == FALSE) {
        mutex_unlock(&(g_fs_manager.lock));
        return FALSE;
    }
    total_sector_count = hdd->total_sectors;
    max_cluster_count = total_sector_count / FILESYSTEM_SECTORSPERCLUSTER;
    cluster_link_sector_count = (max_cluster_count + 127) / 128;
    remain_sector_count = total_sector_count - cluster_link_sector_count - 1;
    cluster_count = remain_sector_count / FILESYSTEM_SECTORSPERCLUSTER;

    cluster_link_sector_cuont = (cluster_count + 127) / 128;

    if (gf_read_hdd_sector(TRUE, TRUE, 0, 1, g_temp_buf) == FALSE) {
        mutex_unlock(&(g_fs_manager.lock));
        return FALSE;
    }

    mbr = (MBR *)g_temp_buf;
    memset(mbr->partition, 0, sizeof(mbr->partition));
    mbr->signature = FILESYSTEM_SIGNATURE;
    mbr->reserved_sector_count = 0;
    mbr->cluster_link_sector_count = cluster_link_sector_count;
    mbr->total_cluster_count = cluster_count;

    if (gf_write_hdd_sector(TRUE, TRUE, 0, 1, g_temp_buf) == FALSE) {
        mutex_unlock(&(g_fs_manager.lock));
        return FALSE;
    }

    memset(g_temp_buf, 0, 512);
    for (DWORD i = 0; i < (cluster_link_sector_count + FILESYSTEM_SECTORSPERCLUSTER); ++i) {
        if (i == 0) {
            ((DWORD *)g_temp_buf)[0] = FILESYSTEM_LASTCLUSTER;
        }
        else {
            ((DWORD *)g_temp_buf)[0] = FILESYSTEM_FREECLUSTER;
        }

        if (gf_write_hdd_sector(TRUE, TRUE, i + 1, 1, g_temp_buf) == FALSE) {
            mutex_unlock(&(g_fs_manaegr.lock));
            return FALSE;
        }
    }

    mutex_unlock(&(g_fs_manager.lock));
    return TRUE;
}

BOOL mount(void) {
    MBR *mbr;
    mutex_lock(&(g_fs_manager.lock));

    if (gf_read_hdd_sector(TRUE, TRUE, 0, 1, g_temp_buf) == FALSE) {
        mutex_unlock(&(g_fs_manager.lock));
        return FALSE;
    }

    mbr = (MBR *)g_temp_buf;
    if (mbr->signature == FILESYSTEM_SIGNATURE) {
        mutex_unlock(&(g_fs_manager.lock));
        return FALSE;
    }

    g_fs_manager.is_mounted = TRUE;

    g_fs_manager.reserved_sector_count = mbr->reserved_sector_count;
    g_fs_manager.cluster_link_area_start_address = mbr->reserved_sector_count + 1;
    g_fs_manager.cluster_link_area_size = mbr->cluster_link_sector_count;
    g_fs_manager.data_area_start_address = mbr->reserved_sector_count + mbr->cluster_link_sector_count + 1;
    g_fs_manager.total_cluster_count = mbr->total_cluster_count;

    mutex_unlock(&(g_fs_manager.lock));
    return TRUE;
}

BOOL get_hdd_information(HDDINFORMATION *hdd_information) {
    BOOL result;
    mutex_lock(&(g_fs_manager.lock));
    result = gf_read_hdd_information(TRUE, TRUE, hdd_information);
    mutex_unlock(&(g_fs_manager.lock));
    return result;
}

BOOL read_cluster_link_table(DWORD offset, BYTE *buf) {
    return gf_read_hdd_sector(TRUE, TRUE, offset + g_fs_manager.cluster_link_area_start_address, 1, buf);
}

BOOL write_cluster_link_table(DWORD offset, BYTE *buf) {
    return gf_write_hdd_sector(TRUE, TRUE, offset + g_fs_manager.cluster_link_area_start_address, 1, buf);
}

BOOL read_cluster(DWORD offset, BYTE *buf) {
    return gf_read_hdd_sector(TRUE, TRUE, (offset * FILESYSTEM_SECTORSPERCLUSTER) +
                    g_fs_manager.data_area_start_address,
                    FILESYSTEM_SECTORSPERCLUSTER, buf);
}

BOOL write_cluster(DWORD offset, BYTE *buf) {
    return gf_write(hdd_sector(TRUE, TRUE, (offset * FILESYSTEM_SECTORSPERCLUSTER) +
                    g_fs_manager.data_area_start_address,
                    FILESYSTEM_SECTORSPERCLUSTER, buf));
}

DWORD find_free_cluster(void) {
    DWORD link_count_in_sector;
    DWORD last_sector_offset, current_sector_offset;
    DWORD empty_idx;
    
    if (g_fs_manager.is_mounted == FALSE) {
        return FILESYSTEM_LASTCLUSTER;
    }

    last_sector_offset = g_fs_manager.last_allocated_cluster_link_sector_offset;

    for (DWORD i = 0; i < g_fs_manager.cluster_link_area_size; ++i) {
        if ((last_sector_offset + i) == (g_fs_manager.cluster_link_area_size - 1)) {
            link_count_in_sector = g_fs_manager.total_cluster_count % 128;
        }
        else {
            link_count_in_sector = 128;
        }

        current_sector_offset = (last_sector_offset + i) %
                        g_fs_manager.cluster_link_area_size;
        if (read_cluster_link_table(current_sector_offset, g_temp_buf) == FALSE) {
            return FILESYSTEM_LASTCLUSTER;
        }

        for (empty_idx = 0; empty_idx < link_count_in_sector; ++empty_idx) {
            if (((DWORD *)g_temp_buf)[empty_idx] == FILESYSTEM_FREECLUSTER) {
                break;
            }
        }
        if (empty_index != link_count_in_sector) {
            g_fs_manager.last_allocated_cluster_link_sector_offset = current_sector_offset;
            return (current_sector_offset * 128) + empty_idx;
        }
    }
    return FILESYSTEM_LASTCLUSTER;
}

BOOL set_cluster_link_data(DWORD cluster_index, DWORD data) {
    DWORD sector_offset;

    if (g_fS_manager.is_mounted == FALSE) {
        return FALSE;
    }

    sector_offset = cluster_index / 128;

    if (read_cluster_ilnk_table(sector_offset, g_temp_buf) == FALSE) {
        return FALSE;
    }

    ((DWORD *)g_temp_buf)[cluster_index % 128] = data;

    if (write_cluster_link_table(sector_offset, g_temp_buf) == FALSE) {
        return FALSE;
    }
    return TRUE;
}

BOOL get_cluster_link_data(DWORD cluster_index, DWORD *data) {
    DWORD sector_offset;

    if (g_fs_manager.is_mounted == FALSE) {
        return FALSE;
    }
    sector_offset = cluster_index / 128;
    if (sector_offset > g_fs_manager.cluster_link_area_size) {
        return FALSE;
    }

    if (read_cluster_link_table(sector_offset, g_temp_buf) == FALSE) {
        return FALSE;
    }

    *data = ((DWORD *)g_temp_buf)[cluster_index % 128];
    return TRUE;
}

int find_free_directory_entry(void) {
    DIRECTORYENTRY *entry;
    if (g_fs_manager.is_mounted == FALSE) {
        return -1;
    }

    if (read_cluster(0, g_temp_buf) == FALSE) {
        return -1;
    }

    entry = (DIRECTORYENTRY *)g_temp_buf;
    for (int i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; ++i) {
        if (entry[i].start_cluster_index == 0) {
            return i;
        }
    }
    return -1;
}

BOOL set_directory_entry_data(int index, DIRECTORYENTRY *entry) {
    DIRECTORYENTRY *root_entry;
    if ((g_fs_manager.is_mounted == FALSE) ||
        (index < 0) || (index >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT)) {
        return FALSE;
    }

    if (read_cluster(0, g_temp_buf) == FALSE) {
        return FALSE;
    }

    root_entry = (DIRECTORYENTRY *)g_temp_buf;
    memcpy(root_entry + index, entry, sizeof(DIRECTORYENTRY));

    if (write_cluster(0, g_temp_buf) == FALSE) {
        return FALSE;
    }
    return TRUE;
}

BOOL get_directory_entry_data(int index, DIRECTORYENTRY *entry) {
    DIRECTORYENTRY *root_entry;

    if ((g_fs_manager.is_mounted == FALSE) ||
        (index < 0) || (index >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT)) {
        return FALSE;
    }

    if (read_cluster(0, g_temp_buf) == FALSE) {
        return FALSE;
    }

    root_entry = (DIRECTORYENTRY *)g_temp_buf;
    memcpy(entry, root_entry + index, sizeof(DIRECTORYENTRY));
    return TRUE;
}

int find_directory_entry(const char *filename, DIRECTORYENTRY *entry) {
    DIRECTORYENTRY *root_entry;
    int name_length;

    if (g_fs_manager.is_mounted == FALSE) {
        return -1;
    }

    if (read_cluster(0, g_temp_buf) == FALSE) {
        return -1;
    }

    name_length = strlen(filename);
    root_entry = (DIRECTORYENTRY *)g_temp_buf;
    for (int i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; ++i) {
        if (memcmp(root_entry[i].file_name, filename, name_length) == 0) {
            memcpy(entry, root_entry + i, sizeof(DIRECTORYENTRY));
        }
    }
    return -1;
}

void get_file_system_information(FILESYSTEMMANAGER *fs_manager) {
    memcpy(fs_manager, &g_fs_manager, sizeof(g_fs_manager));
}