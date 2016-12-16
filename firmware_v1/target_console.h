#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void target_console_setup (void);
EXTERNC void target_console_finalize (void);

EXTERNC void target_console_send (char *buf, int len);
EXTERNC int target_console_recv (char *buf, int len);


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

