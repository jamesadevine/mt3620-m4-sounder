#include "dmesg.h"
#include "lib/NVIC.h"
#include <string.h>
#include <stdio.h>

#if DEVICE_DMESG_BUFFER_SIZE > 0

struct CodalLogStore codalLogStore;

static void logwriten(const char *msg, int l) {
    uint32_t pri = NVIC_BlockIRQs();
    if (codalLogStore.ptr + l >= sizeof(codalLogStore.buffer)) {
#if 1
        codalLogStore.buffer[0] = '.';
        codalLogStore.buffer[1] = '.';
        codalLogStore.buffer[2] = '.';
        codalLogStore.ptr = 3;
#else
        // this messes with timings too much
        const int jump = sizeof(codalLogStore.buffer) / 4;
        codalLogStore.ptr -= jump;
        memmove(codalLogStore.buffer, codalLogStore.buffer + jump, codalLogStore.ptr);
        // zero-out the rest so it looks OK in the debugger
        memset(codalLogStore.buffer + codalLogStore.ptr, 0,
               sizeof(codalLogStore.buffer) - codalLogStore.ptr);
#endif
    }
    if (l + codalLogStore.ptr >= sizeof(codalLogStore.buffer))
        return; // shouldn't happen
    memcpy(codalLogStore.buffer + codalLogStore.ptr, msg, l);
    codalLogStore.ptr += l;
    codalLogStore.buffer[codalLogStore.ptr] = 0;
    NVIC_RestoreIRQs(pri);
}

void codal_dmesg(const char *format, ...) {
    va_list arg;
    // while(1);
    va_start(arg, format);
    codal_vdmesg(format, arg);
    va_end(arg);
}

void codal_dmesgf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    codal_vdmesg(format, arg);
    va_end(arg);
}

void cust_string_reverse(char *s) {
    if (s == NULL)
        return;

    char *j;
    int c;

    j = s + strlen(s) - 1;

    while (s < j) {
        c = *s;
        *s++ = *j;
        *j-- = c;
    }
}

/**
 * Converts a given integer into a string representation.
 *
 * @param n The number to convert.
 *
 * @param s A pointer to the buffer where the resulting string will be stored.
 */
void cust_itoa(int n, char *s) {
    int i = 0;
    int positive = (n >= 0);

    if (s == NULL)
        return;

    // Record the sign of the number,
    // Ensure our working value is positive.
    unsigned k = positive ? n : -n;

    // Calculate each character, starting with the LSB.
    do {
        s[i++] = (k % 10) + '0';
    } while ((k /= 10) > 0);

    // Add a negative sign as needed
    if (!positive)
        s[i++] = '-';

    // Terminate the string.
    s[i] = '\0';

    // Flip the order.
    cust_string_reverse(s);
}

static void writeNum(char *buf, uintptr_t n, bool full) {
    int i = 0;
    int sh = sizeof(uintptr_t) * 8 - 4;
    while (sh >= 0) {
        int d = (n >> sh) & 0xf;
        if (full || d || sh == 0 || i) {
            buf[i++] = d > 9 ? 'A' + d - 10 : '0' + d;
        }
        sh -= 4;
    }
    buf[i] = 0;
}

#define WRITEN(p, sz_)                                                                             \
    do {                                                                                           \
        sz = sz_;                                                                                  \
        ptr += sz;                                                                                 \
        if (ptr < dstsize) {                                                                       \
            memcpy(dst + ptr - sz, p, sz);                                                         \
            dst[ptr] = 0;                                                                          \
        }                                                                                          \
    } while (0)

int cust_vsprintf(char *dst, unsigned dstsize, const char *format, va_list ap) {
    const char *end = format;
    unsigned ptr = 0, sz;
    char buf[sizeof(uintptr_t) * 2 + 8];

    for (;;) {
        char c = *end++;
        if (c == 0 || c == '%') {
            if (format != end)
                WRITEN(format, end - format - 1);
            if (c == 0)
                break;
            uint32_t val = va_arg(ap, uint32_t);

            c = *end++;
            buf[1] = 0;
            switch (c) {
            case 'c':
                buf[0] = val;
                break;
            case 'd':
                cust_itoa(val, buf);
                break;
            case 'x':
            case 'p':
            case 'X':
                buf[0] = '0';
                buf[1] = 'x';
                writeNum(buf + 2, val, c != 'x');
                break;
            case 's':
                WRITEN((char *)(void *)val, strlen((char *)(void *)val));
                buf[0] = 0;
                break;
            case '%':
                buf[0] = c;
                break;
            default:
                buf[0] = '?';
                break;
            }
            format = end;
            WRITEN(buf, strlen(buf));
        }
    }

    return ptr;
}


void codal_vdmesg(const char *format, va_list ap) {
    // if (format[0] == '!')
    //    pin_pulse(PIN_X0, 1);
    char tmp[80];
    cust_vsprintf(tmp, sizeof(tmp) - 1, format, ap);
    int len = strlen(tmp);
#if JD_LORA
    while (len && (tmp[len - 1] == '\r' || tmp[len - 1] == '\n'))
        len--;
#endif
    tmp[len] = '\n';
    tmp[len + 1] = 0;
    logwriten(tmp, len + 1);
}

#endif

