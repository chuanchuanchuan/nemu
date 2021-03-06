#include "common.h"
#include "memory.h"
#include "avatar.h"
#include "x86.h"
#include <string.h>
#include "inline.h"

#define VMEM_ADDR 0xa0000
#define VMEM_SIZE 0x20000
#define SCR_SIZE (320 * 200)

static PTE vptable[(VMEM_ADDR + VMEM_SIZE) / PAGE_SIZE] align_to_page;
/* Use the function to get the start address of user page directory. */
PDE* get_updir();

void create_video_mapping() {
    /* Create an identical mapping from virtual memory area
     * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area
     * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
     * some page tables to create this mapping.
     */
    PDE *updir = get_updir();
    PTE *ptable = (PTE *)va_to_pa(vptable);
    updir[0].val = make_pde(ptable);

    memset(vptable, 0, sizeof(vptable));

    uint32_t vmem_addr = VMEM_ADDR;
    uint32_t idx = VMEM_ADDR / PAGE_SIZE;
    for (vmem_addr = VMEM_ADDR; vmem_addr < VMEM_ADDR + VMEM_SIZE; vmem_addr += PAGE_SIZE) {
        vptable[idx].val = make_pte(vmem_addr);
        idx++;
    }
}

static uint8_t palette[256][3];

void video_mapping_write_test() {
    uint32_t *buf = (void *)VMEM_ADDR;
    for (uint32_t i = 0; i < 256; i++) {
        out_byte(0x3c7, i);
        palette[i][0] = in_byte(0x3c9);
        palette[i][1] = in_byte(0x3c9);
        palette[i][2] = in_byte(0x3c9);
    }
    out_byte(0x3c8, 0);
    for (uint32_t i = 0; i < 256; i++) {
        out_byte(0x3c9, header_data_cmap[i][0] >> 2);
        out_byte(0x3c9, header_data_cmap[i][1] >> 2);
        out_byte(0x3c9, header_data_cmap[i][2] >> 2);
    }
    fast_memcpy(buf, header_data, width * height);
}

void video_mapping_read_test() {
    int i;
    uint32_t *buf = (void *)VMEM_ADDR;
    for(i = 0; i < SCR_SIZE / 4; i ++) {
        assert(buf[i] == ((uint32_t *)header_data)[i]);
    }
}

void video_mapping_clear() {
    memset((void *)VMEM_ADDR, 0, SCR_SIZE);
    out_byte(0x3c8, 0);
    for (uint32_t i = 0; i < 256; i++) {
        out_byte(0x3c9, palette[i][0]);
        out_byte(0x3c9, palette[i][1]);
        out_byte(0x3c9, palette[i][2]);
    }
}
