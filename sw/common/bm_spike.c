/* Spike back end for bm.h, using the HTIF host interface (no proxy kernel).
 *
 * Output goes through the fesvr syscall proxy: tohost holds a pointer to a
 * 4-word block {SYS_write, fd, buf, len}. Exit writes (code << 1) | 1.
 * On RV32 tohost is written as two 32-bit stores; both payloads keep the
 * upper word zero, so Spike can never observe a half-written value. */
#include <stdarg.h>
#include <stdint.h>
#include "bm.h"

volatile uint64_t tohost   __attribute__((section(".tohost"), aligned(64)));
volatile uint64_t fromhost __attribute__((section(".tohost"), aligned(64)));

#define SYS_write 64

static void htif_syscall(uint64_t n, uint64_t a0, uint64_t a1, uint64_t a2)
{
    static volatile uint64_t magic[8] __attribute__((aligned(64)));
    magic[0] = n; magic[1] = a0; magic[2] = a1; magic[3] = a2;
    __asm__ volatile("fence" ::: "memory");
    tohost = (uintptr_t)magic;
    while (fromhost == 0) { }
    fromhost = 0;
    __asm__ volatile("fence" ::: "memory");
}

/* Output is buffered per line to keep the number of host calls low. */
static char   outbuf[128];
static unsigned outlen;

static void flush(void)
{
    if (outlen) {
        htif_syscall(SYS_write, 1, (uintptr_t)outbuf, outlen);
        outlen = 0;
    }
}

void bm_putc(char c)
{
    outbuf[outlen++] = c;
    if (c == '\n' || outlen == sizeof outbuf) flush();
}

void bm_puts(const char *s) { while (*s) bm_putc(*s++); }

void bm_exit(int code)
{
    flush();
    tohost = ((uint64_t)(uint32_t)code << 1) | 1;
    for (;;) { }
}

void bm_trap(uint32_t mcause)
{
    uint32_t mepc, mtval;
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    bm_printf("\nTRAP mcause=%u mepc=0x%08x mtval=0x%08x\n", mcause, mepc, mtval);
    bm_exit(0x7f);
}

/* ---------------------------------------------------------------- printf */

static void put_u64(uint64_t v, unsigned base, int width, char pad, int neg)
{
    char tmp[24];
    int n = 0;
    do { unsigned d = (unsigned)(v % base); tmp[n++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); v /= base; } while (v);
    if (neg) {
        if (pad == '0') { bm_putc('-'); width--; } else tmp[n++] = '-';
    }
    while (width-- > n) bm_putc(pad);
    while (n) bm_putc(tmp[--n]);
}

int bm_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { bm_putc(*fmt); continue; }
        fmt++;
        char pad = ' ';
        int width = 0, lng = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');
        while (*fmt == 'l') { lng++; fmt++; }
        switch (*fmt) {
        case 'd': {
            int64_t v = lng >= 2 ? va_arg(ap, long long) : lng ? va_arg(ap, long) : va_arg(ap, int);
            put_u64(v < 0 ? -(uint64_t)v : (uint64_t)v, 10, width, pad, v < 0);
            break;
        }
        case 'u': case 'x': {
            uint64_t v = lng >= 2 ? va_arg(ap, unsigned long long)
                       : lng ? va_arg(ap, unsigned long) : va_arg(ap, unsigned);
            put_u64(v, *fmt == 'x' ? 16 : 10, width, pad, 0);
            break;
        }
        case 's': { const char *s = va_arg(ap, const char *); bm_puts(s ? s : "(null)"); break; }
        case 'c': bm_putc((char)va_arg(ap, int)); break;
        case '%': bm_putc('%'); break;
        case '\0': fmt--; break;
        default: bm_putc('%'); bm_putc(*fmt); break;
        }
    }
    va_end(ap);
    return 0;
}

/* GCC may emit calls to these even with -ffreestanding. */
void *memset(void *d, int c, unsigned long n)
{ unsigned char *p = d; while (n--) *p++ = (unsigned char)c; return d; }
void *memcpy(void *d, const void *s, unsigned long n)
{ unsigned char *p = d; const unsigned char *q = s; while (n--) *p++ = *q++; return d; }
