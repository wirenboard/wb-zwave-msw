// Debug output of values
// DBG
#ifdef LOGGING_DBG
#define LOG_INT_VALUE(TEXT, VALUE)                                                                                     \
    LOGGING_UART.print(TEXT);                                                                                          \
    LOGGING_UART.print(VALUE);                                                                                         \
    LOGGING_UART.print("\n");
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC)                                                                        \
    LOGGING_UART.print(TEXT);                                                                                          \
    LOGGING_UART.fixPrint(VALUE, PREC);                                                                                \
    LOGGING_UART.print("\n");
#else
#define LOG_INT_VALUE(TEXT, VALUE)
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC)
#endif

#ifdef LOGGING_DBG
#define DEBUG LOGGING_UART.print
#else
#define DEBUG(...) ((void)0)
#endif
