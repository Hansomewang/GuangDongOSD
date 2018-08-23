/*
 *	dbg_printf.c
 * 
 *  Author: Thomas Chang
 *  Date: 2014-02-26
 *
 *  This file contains two functions:
 *  dbg_setlevel(int nOutLevel) - set the output level, 0 would be the lowest level which is the default.
 *  dbg_getlevel() - get current level
 *  dbg_printf( level, fmt...) - print out the message to stdout. when nOutLevel >= level, output will go.
 *
 *  in your application, all tracing, debugging or message output can be category in different level
 *  using dbg_printf instead of printf. Therefore, user can turn on message to any detail level by
 *  assign it through the argument of application.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "utils_ptrlist.h"
#include "utils_net.h"

#define DIR_LOG "./"
#define SYSLOG_FILE "run.log"
#define SYSLOG_BKUP "run.log.bak"
#define SYSLOG_LIMIT 4000000

pid_t dbg_pid = 0;
// void lprintf(const char *fmt, ...);

#define DBG_SERVICE_PORT 8000
#define max(a, b) ((a) > (b) ? (a) : (b))

static pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thread_cond = PTHREAD_COND_INITIALIZER;

#define Thread_Lock() pthread_mutex_lock(&thread_lock)
#define Thread_Unlock() pthread_mutex_unlock(&thread_lock)

static PtrList Monitor_list = PTRLIST_INITIALIZER;
static PtrList Output_list = PTRLIST_INITIALIZER;

typedef struct tagMonitor
{
	char peer_ip[16];
	int peer_port;
	int fd;	// socket
	int level; // output level, default is 3
} Monitor;

static int nMyLevel = 0;  // default output level
static int nMaxLevel = 0; // max level (local + all monitor)
static int dbgthread_run = 0;
static int fd_svc = 0; // work thread listen port
static int wait_flushed = 0;
static pthread_t dbg_threadId = 0;
static time_t tLastTouch = 0;
static time_t *pMarker = &tLastTouch;

static int get_maxlevel()
{
	POSITION pos;
	int maxlvl = nMyLevel;

	for (pos = Monitor_list.head; pos != NULL; pos = pos->next)
	{
		Monitor *monitor = (Monitor *)pos->ptr;
		maxlvl = max(maxlvl, monitor->level);
	}
	return maxlvl;
}

static const char *time_stamp()
{
	static char timestr[64];
#ifdef linux
	char *p = timestr;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	strftime(p, sizeof(timestr), "%y/%m/%d %H:%M:%S", localtime(&tv.tv_sec));
	sprintf(p + strlen(p), ".%03lu", tv.tv_usec / 1000);

#else
	SYSTEMTIME tnow;
	GetLocalTime(&tnow);
	sprintf(timestr, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
			tnow.wYear, tnow.wMonth, tnow.wDay,
			tnow.wHour, tnow.wMinute, tnow.wSecond, tnow.wMilliseconds);
#endif
	return timestr;
}

static void lprintf(const char *fmt, ...)
{
	// log message in a peticular log file.
	FILE *fp = fopen(DIR_LOG SYSLOG_FILE, "a");
	long fsize;
	char buf[1024];

	va_list va;
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	if (buf[0] == '\t')
		fprintf(fp, "%24s%s", " ", buf + 1);
	else
		fprintf(fp, "[%s] %s", time_stamp(), buf);
	//	PRINTF(buf);
	fsize = ftell(fp);
	fclose(fp);
	if (fsize > SYSLOG_LIMIT)
	{
		unlink(DIR_LOG SYSLOG_BKUP);
		rename(DIR_LOG SYSLOG_FILE, DIR_LOG SYSLOG_BKUP);
	}
}

static int prepare_fdset(fd_set *set)
{
	int nfdmax = fd_svc;
	POSITION pos;

	FD_ZERO(set);
	if (fd_svc > 0)
		FD_SET(fd_svc, set);
	for (pos = Monitor_list.head; pos != NULL; pos = pos->next)
	{
		Monitor *monitor = (Monitor *)pos->ptr;
		FD_SET(monitor->fd, set);
		nfdmax = max(nfdmax, monitor->fd);
	}
	return nfdmax;
}

static Monitor *accept_monitor()
{
	Monitor *monitor = NULL;
	int fd = sock_accept(fd_svc);
	if (fd < 0)
	{
		printf("[log svc] abnormal - cannot accept client connection.\n");
		close(fd_svc);
		fd_svc = sock_listen(DBG_SERVICE_PORT, NULL, 0);
		if (fd_svc == -1)
		{
			printf("--> cannot open log service listen port again.\n");
			fd_svc = 0;
		}
	}
	else
	{
		monitor = malloc(sizeof(Monitor));
		memset(monitor, 0, sizeof(Monitor));
		monitor->fd = fd;
		monitor->level = 3;
		if (nMaxLevel < 3)
			nMaxLevel = 3;
		strcpy(monitor->peer_ip, sock_getpeername(fd, &monitor->peer_port));
	}
	return monitor;
}

static void thread_timewait(int usec)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += usec * 1000;
	if (ts.tv_nsec > 1000000000LL)
	{
		ts.tv_nsec -= 1000000000LL;
		ts.tv_sec++;
	}

	pthread_mutex_lock(&thread_lock);
	pthread_cond_timedwait(&thread_cond, &thread_lock, &ts);
	pthread_mutex_unlock(&thread_lock);
}

static void *dbg_threadfxc(void *param)
{
	fd_set rd_set;
	struct timeval tv, *ptv;
	int nsel, nfds;
	char *ptr;

	dbgthread_run = 1;
	dbg_pid = getpid();
	printf("[log svc] - work thread (%d) up and run (pid=%d).\n", (int)dbg_threadId, dbg_pid);
	for (; dbgthread_run;)
	{
		nfds = prepare_fdset(&rd_set);
		if (nfds == 0)
			break;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		ptv = Monitor_list.count > 0 ? &tv : NULL;
		nsel = select(nfds + 1, &rd_set, NULL, NULL, ptv);
		if (nsel > 0)
		{
			POSITION pos;
			Monitor *monitor;

			if (FD_ISSET(fd_svc, &rd_set) && (monitor = accept_monitor()) != NULL)
			{
				PtrList_append(&Monitor_list, monitor);
				lprintf("[log svc] - accept a new monitor (ip=%s, port=%d), total=%d\n",
						monitor->peer_ip, monitor->peer_port, Monitor_list.count);
			}
			// check for any input from monitor
			for (pos = Monitor_list.head; pos != NULL;)
			{
				Monitor *monitor = (Monitor *)pos->ptr;
				POSITION pos_next = pos->next;

				if (FD_ISSET(monitor->fd, &rd_set))
				{
					char buf[8];
					if (sock_read_line(monitor->fd, buf, sizeof(buf)) <= 0)
					{
						PtrList_remove(&Monitor_list, pos);
						close(monitor->fd);
						lprintf("[log svc] - monitor %s:%d gone, remove it (%d left).\n",
								monitor->peer_ip, monitor->peer_port, Monitor_list.count);
						free(monitor);
						nMaxLevel = get_maxlevel();
					}
					else
					{
						int nlevel = atoi(buf);
						if (nlevel >= 0)
							monitor->level = nlevel;
						nMaxLevel = max(nMaxLevel, monitor->level);
						printf("[log svc] - client %s:%d switch to level %d. global max level=%d\n",
							   monitor->peer_ip, monitor->peer_port, monitor->level, nMaxLevel);
					}
				}
				pos = pos_next;
			}
		}
		// output all data to every monitor
		while ((ptr = PtrList_remove_head(&Output_list)) != NULL)
		{
			int len = strlen(ptr + 1);
			POSITION pos;
			for (pos = Monitor_list.head; pos != NULL; pos = pos->next)
			{
				Monitor *monitor = (Monitor *)pos->ptr;
				if (monitor->level >= (int)ptr[0])
					sock_write(monitor->fd, ptr + 1, len);
			}
			free(ptr);
		}
		// wait for new data or time out (1000 usec)
		thread_timewait(1000);
	}
	while ((ptr = PtrList_remove_head(&Output_list)) != NULL)
		free(ptr);
	printf("[log svc] - service thread (%d) exit.\n", (int)dbg_threadId);
	dbg_threadId = 0;
	dbg_pid = 0;
	return NULL;
}

int dbg_setlevel(int nNewLevel)
{
	int nOldLevel = nMyLevel;
	if (nNewLevel >= 0)
		nMyLevel = nNewLevel;
	nMaxLevel = max(nMaxLevel, nMyLevel);
	return nOldLevel;
}

int dbg_getlevel()
{
	return nMyLevel;
}

int dbg_getpid()
{
	return (int)dbg_pid;
}

int dbg_printf(int level, const char *format, ...)
{
	va_list va;
	int n;
	char line[1024];
	char *ptr;

	*pMarker = time(NULL);
	if (nMaxLevel < level)
		return 0;
	va_start(va, format);
	n = vsprintf(line, format, va);
	va_end(va);
	if (n < 0)
	{
		printf("vsprintf error - %s\n", strerror(errno));
		return 0;
	}
	if (nMyLevel >= level)
	{
		fputs(line, stdout);
		fflush(stdout);
	}
	if (Monitor_list.count > 0)
	{
		// append new output line to list
		if (line[n - 1] == '\n')
		{
			line[n - 1] = '\r';
			line[n++] = '\n';
			line[n] = '\0';
			;
		}
		ptr = malloc(n + 2);
		if (ptr)
		{
			ptr[0] = level;
			memcpy(ptr + 1, line, n + 1);
			PtrList_append(&Output_list, ptr);
		}
		// wake-up working thread
		pthread_mutex_lock(&thread_lock);
		pthread_cond_signal(&thread_cond);
		pthread_mutex_unlock(&thread_lock);
		if (wait_flushed)
			while (Output_list.count > 0)
				usleep(100);
	}
	return n;
}

int dbg_initialize()
{
	*pMarker = time(NULL);
	fd_svc = sock_listen(DBG_SERVICE_PORT, NULL, 0);
	if (fd_svc == -1)
	{
		printf("faied to listen TCP port %d - dbg_service not avilable.\n", DBG_SERVICE_PORT);
		return -1;
	}
	pthread_create(&dbg_threadId, 0, dbg_threadfxc, NULL);
	return 0;
}

void dbg_terminate()
{
	dbgthread_run = 0;
	if (fd_svc != 0)
		close(fd_svc);
	fd_svc = 0;
}

void dbg_setmarkaddr(int *marker)
{
	pMarker = (time_t *)marker;
}

void dbg_setwaitflushed(int wait)
{
	wait_flushed = wait;
}

int dbg_getnumlogger()
{
	return Monitor_list.count;
}
int dbg_getloggerip(int idx)
{
	POSITION pos;
	int i;
	for (i = 0, pos = Monitor_list.head; i < idx && pos != NULL; pos = pos->next)
		;
	if (pos != NULL)
	{
		Monitor *monitor = (Monitor *)pos->ptr;
		return inet_addr(monitor->peer_ip);
	}
	return 0;
}

void dbg_touch()
{
	*pMarker = time(NULL);
}

int dbg_getidlesec()
{
	return time(NULL) - (*pMarker);
}
