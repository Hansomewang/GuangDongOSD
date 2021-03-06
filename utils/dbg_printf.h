#ifndef _DBGPRINTF_INCLUDED_
#define _DBGPRINTF_INCLUDED_

extern void lprintf(const char *fmt, ...);

extern int dbg_initialize();
extern void dbg_terminate();
extern int dbg_setlevel( int nNewLevel );
extern void dbg_setwaitflushed( int wait );
extern int dbg_getlevel();
extern int dbg_getpid();
extern int dbg_printf(int level, const char* format, ...);
extern int dbg_getnumlogger();
extern int dbg_getloggerip(int idx);

// for process in-active time monitoring. dbg_printf will touch the time mark
// 1. set address to keep the time marker. default is an internal static 
extern void dbg_setmarkaddr( int *marker );		
extern void dbg_touch();				// touch the time marker as current system time 
extern int	dbg_getidlesec();	// get number of seconds that marker has not been touched

#define PRINTF(fmt...)	dbg_printf(0,fmt)
#define PRINTF1(fmt...)	dbg_printf(1,fmt)
#define PRINTF2(fmt...)	dbg_printf(2,fmt)
#define PRINTF3(fmt...)	dbg_printf(3,fmt)
#define PRINTF4(fmt...) dbg_printf(4,fmt)
#define PRINTF5(fmt...)

#define DBG_PRINTF(fmt...)	fprintf(stderr,fmt)

#endif
