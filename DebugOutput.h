// Debug output macros
#include "Debug.h"

#ifdef LOGGING_DBG
#define LOG_INT_VALUE(TEXT, VALUE)                                                                                     \
    do {                                                                                                               \
        LOGGING_UART.print(TEXT);                                                                                      \
        LOGGING_UART.print(VALUE);                                                                                     \
        LOGGING_UART.print("\n");                                                                                      \
    } while (0)
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC)                                                                        \
    do {                                                                                                               \
        LOGGING_UART.print(TEXT);                                                                                      \
        LOGGING_UART.fixPrint(VALUE, PREC);                                                                            \
        LOGGING_UART.print("\n");                                                                                      \
    } while (0)
#else
#define LOG_INT_VALUE(TEXT, VALUE) ((void)0)
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC) ((void)0)
#endif

#ifdef LOGGING_DBG
#define DEBUG LOGGING_UART.print
#else
#define DEBUG(...) ((void)0)
#endif
