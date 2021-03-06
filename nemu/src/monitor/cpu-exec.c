#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "cpu/instr-fetch.h"
#include <setjmp.h>
#include "device/i8259.h"
#include "cpu/interrupt.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the ``si'' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 100

int nemu_state = STOP;

int exec(swaddr_t);
void device_update();

char assembly[80];
char asm_buf[128];

/* Used with exception handling. */
jmp_buf jbuf;

#ifdef PRINT_INSTR
static inline void sprint_hex(char *buf, uint32_t value, int digits, bool fillzero) {
    char *p = buf + digits;
    do {
        p--;
        char c = value & 0xf;
        c = c > 9 ? c - 10 + 'a' : c + '0';
        *p = c;
        value >>= 4;
    } while (p != buf && value != 0);
    char fill = fillzero ? '0' : ' ';
    while (p-- != buf) {
        *p = fill;
    }
}

static inline void print_bin_instr(swaddr_t eip, int len) {
    swaddr_t target = eip + len;
    char *pbuf = asm_buf;
    sprint_hex(pbuf, eip, 8, false);
    pbuf += 8;
    *(pbuf++) = ':';
    *(pbuf++) = ' ';
    *(pbuf++) = ' ';
    *(pbuf++) = ' ';
    for(; eip < target; eip++) {
        sprint_hex(pbuf, instr_fetch(eip, 1), 2, true);
        pbuf[2] = ' ';
        pbuf += 3;
    }
    char *bufend = asm_buf + 50;
    for (; pbuf < bufend; pbuf++) {
        *pbuf = ' ';
    }
    *pbuf = 0;
}
#endif

/* This function will be called when an `int3' instruction is being executed. */
void do_int3() {
    printf("\nHit breakpoint at eip = 0x%08x\n", cpu.eip);
    nemu_state = STOP;
}

#ifndef PERFORMANCE
/* Called when hit watchpoint */
void do_watchpoint(WP *wp, uint32_t old_result, uint32_t new_result) {
    printf("\nHit watchpoint at eip = 0x%08x\n", cpu.eip);
    printf("Watchpoint %d: %s\n", wp_get_no(wp), wp_get_expr(wp));
    printf("Old value = %#x (%d)\nNew value = %#x (%d)\n", old_result, old_result, new_result, new_result);
    nemu_state = STOP;
}
#endif

/* Simulate how the CPU works. */
void cpu_exec(volatile uint32_t n) {
    if(nemu_state == END) {
        printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
        return;
    }
    nemu_state = RUNNING;

#ifdef DEBUG
    volatile uint32_t n_temp = n;
#endif

    n -= setjmp(jbuf);

    for(; n > 0; n--) {
        if (n == ~0) n++;
#ifdef DEBUG
        swaddr_t eip_temp = cpu.eip;
#ifdef DOT
        if((n & 0xffff) == 0) {
            /* Output some dots while executing the program. */
            fputc('.', stderr);
        }
#endif
#endif

        /* Execute one instruction, including instruction fetch,
         * instruction decode, and the actual execution. */
#ifdef DEBUG
        int instr_len =
#endif
        exec(cpu.eip);

#ifdef DEBUG
        print_bin_instr(eip_temp, instr_len);
        Log_write("%s%s\n", asm_buf, assembly);
        if(n_temp < MAX_INSTR_TO_PRINT) {
            printf("%s%s\n", asm_buf, assembly);
        }
#endif

#ifndef PERFORMANCE
        WP *wp = NULL;
        uint32_t old_result, new_result;
        if (wp_watch(&wp, &old_result, &new_result)) {
            do_watchpoint(wp, old_result, new_result);
        }
#endif

        if(nemu_state != RUNNING) { return; }

#ifdef HAS_DEVICE
        if (cpu.INTR & cpu.ief) {
            uint32_t intr_no = i8259_query_intr();
            i8259_ack_intr();
            raise_intr(intr_no);
        }
        device_update();
#endif
    }

    if(nemu_state == RUNNING) { nemu_state = STOP; }
}
