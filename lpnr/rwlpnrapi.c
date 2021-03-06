/*
 *  rwlpnrapi.c
 *	
 *	This source file implement the RunWell vehicle license plate number recognition (LPNR) camera
 *  API. 
 *
 *  This source file is dual plateform coded (linux and Windows). In Windows, using VC6 or VS 20xx can generate
 *  a DLL with each exported API in CALLTYPE (defined in rwlpnrapi.h, could be C or PASCAL) calling sequence.
 *  In Linux plateform, it's very easy to create a so file (shared object) for Lane software to bind into their application
 *  in runtime (of cause, to statically link into lane application is also applicable). In Linux, the API calling sequency
 *  is 'C'.
 *
 *  In Windows plateform, Event is forward to application via Windows PostMessage system call, while in LInux
 *  plateform, a callback function assigned by application is invoked to notice following event:
 *	  event number 1: LPNR equipment online
 *    event number 2: LPNR equipment offline
 *    event number 3: LPNR equipment start one image processing
 *    event number 4: LPNR processing done. data is available
 *
 *  Author: Thomas Chang
 *  Copy Right: RunWell Control Technology, Suzhou
 *  Date: 2014-10-29
 *  
 * NOTE 
 *  To build a .so file in linux, use command:
 *  $ gcc -o3 -Wall -shared -orwlpnrapi.so  rwlpnrapi.c
 *  To build a standalone executable tester for linux, include log, use command:
 *  $ gcc -o3 -Wall -DENABLE_LOG -DENABLE_TESTCODE -olpnrtest -lpthread -lrt rwlpnrapi.c 
 *  To enable the program log for so
 *  $ gcc -o3 -Wall -DENABLE_LOG -shared -rwlpnrapi.so -lpthread rwlpnrapi.c
 *
 *  To built a DLL for Windows plateform,  use Microsoft VC or Visual Studio, create a project for DLL and add rwlpnrapi.c, rwlpnrapi.h lpnrprotocol.h
 *  and rwlpnrapi.def (define the DLL export API entry name, otherwise, VC or VS will automatically add prefix/suffix for each exported entries).
 *  
 *
 *  车道软件使用此函数库说明：
 *
 *  1. 初始化：
 *	   	LPNR_Init 参数为LPNR设备的IP地址字串。返回0表示成功。-1 表示失败 (IP地址错误，或是该IP没有响应UDP询问)
 *		  
 *		 
 *	2. 设置回调函数(linux)，或是消息接收窗口(Windows)
 *		 LPNR_SetCallBack -- for linux and/or Windows
 *		 LPNR_SetWinMsg   -- for windows
 *			初始化成功后立刻设置。
 *
 *	3. 获取车牌号码：
 *		 LPNR_GetPlateNumber - 参数为调用函数储存车牌号码字串的指针，长度必须足够。车牌号码存放为
 *       <颜色><汉字><数字>，例如：蓝苏A123456. 如果没有辨识到车牌，返回字串"整牌拒识"。
 *
 *	4. 获取车牌彩色小图
 *		 LPNR_GetPlateColorImage - 参数为储存图片的buffer指针，图片内容为bmp格式。直接储存整个buffer到一个
 *       文件就是一个bmp图档。函数返回值是整个buffer的长度。应用程序需保证buffer长度足够，需要的长度为
 *			 车牌小图长度x宽度+54。因为车牌图片尺寸每次处理都不相同，必须提供一个最大可能的尺寸。
 * 
 *	4. 获取车牌二值化小图
 *		 LPNR_GetPlateBinaryImage - 参数为储存图片的buffer指针，图片内容为bmp格式。直接储存整个buffer到一个
 *       文件就是一个bmp图档。函数返回值是整个buffer的长度。应用程序需保证buffer长度足够，需要的长度为
 *			 车牌小图长度x宽度+54。因为车牌图片尺寸每次处理都不相同，必须提供一个最大可能的尺寸。
 *
 *	5. 获取抓拍整图 （进行车牌识别的摄像机输入图片）
 *		 LPNR_GetCapturedImage - 参数为储存图片的buffer指针，图片内容为jpg格式。直接储存整个buffer到一个
 *       文件就是一个jpeg图档。函数返回值是整个buffer的长度。应用程序需保证buffer长度足够，需要的长度概算为
 *       摄像机解像度 x factor. factor根据设置的JPEG压缩质量大约为0.1~0.5。
 *
 *  6. 获取当前实时图像帧 （开始分析后实时图像会暂停，直到当前图像分析完成才继续更新）
 *		 LPNR_GetLiveFrame - 参数和LPNR_GetCapturedImage相同，也是一个jpeg帧的内容。默认是使能发送实时图像。
 *		 可以调用LPNR_EnableLiveFrame禁能实时图像发送，减小网络负荷。
 *
 *  7. 询问状态的函数
 *		LPNR_IsOnline - 是否连线，返回布林值
 *		LPNR_IsIdle - 识别机是否进行识别处理中，返回布林值
 *     LPNR_GetPlateColorImageSize - 返回车牌彩色小图大小
 *     LPNR_GetPlateBinaryImageSize - 返回车牌二值化小图大小
 *     LPNR_GetCapturedImageSize - 返回抓拍大图大小（# of bytes)
 *     LPNR_GetHeadImageSize - 获取车头图大小（如有使能发送车头图）
 *     LPNR_GetQuadImageSize - 获取1/4图图片大小（如有使能发送抓拍小图）
 *	   LPNR_GetTiming - 获取识别机完成当前识别处理消耗的时间 （总过程时间和总处理时间）
 *
 *  8. 释放处理结果数据和图片(清除数据)
 *		 LPNR_ReleaseData
 *
 *  9. 软触发抓拍识别
 *		 LPNR_SoftTrigger - 抓拍 + 识别
 *	     LPNR_TakeSnapFrame - 抓拍一张图，但不进行识别
 *
 *	10. 结束
 *		 LPNR_Terminate - 调用此函数结束使用LPNR，动态库内部工作线程结束并关闭网络连接。
 *		
 *  11. 其他辅助函数
 *		LPNR_SyncTime - 同步识别机时间和计算机时间 （下发对时命令）
 *		LPNR_EnableLiveFrame - 使能或是禁能识别机发送实时图像帧
 *      LPNR_ReleaseData - 释放当前识别结果动态分配的数据
 *		LPNR_Lock/LPNR_Unlock - 上锁，解锁数据（获取实时帧或是任何识别结果数据之前先加锁，获取完后解锁，避免获取过程中被工作线程更改。
 *		LPNR_GetPlateAttribute - 获取车牌/车辆额外信息
 *		LPNR_GetExtDI - 获取一体机的扩展DI状态
 *		LPNR_GetMachineIP - 获取识别机IP字串
 *		LPNR_GetCameraLabel - 获取识别机标签（名字）
 *
 *	事件编号：
 *	回调函数只有一个参数就是事件编号。对于windows，这个编号在消息通知的 LPARAM里面，WPARAM为发出事件的摄像机
 *  句柄 (由LPNR_Init返回)。
 *    事件编号1：LPNR连线。
 *    事件编号2：LPNR离线
 *    事件编号3：开始进行识别处理
 *    事件编号4：识别处理结束可以获取结果。
 *	  事件编号5：实时图像帧更新，可以获取，更新画面。
 *    事件编号6：车辆进入虚拟线圈检测区（上位机要等收到事件编号4后才可以去获取车牌信息和图片）
 *    事件编号7：车辆离开虚拟线圈检测区
 *    事件编号8：扩展DI点状态有变化（200D, 200P, 500才有扩展IO）
 *
 *  NOTE - ENABLERS
 *  1. ENABLE_LOG	- define 后，程序会记录日志在固定的文件中
 *  2. ENABLE_TESTCODE - define 后，测试程序使能，可以编译出可执行程序测试（linux版本才有）
 *
 *  修改：
 *  A. 2015-10-01：
 *     1. 新增函数 LPNR_GetHeadImageSize, LPNR_GetHeadImage
 *         功能：获取车头图像，图像大小为CIF （PAL为720x576， 以车牌位置为中心点， 没有车牌则以识别区为中心的。
 *     2. 新增函数 LPNR_GetQuadImageSize, LPNR_GetQuadImage
 *         功能：获取1/4解像度图像。
 *     3. 新增函数 LPNR_GetHeadImageSize, LPNR_GetHeadImage获取车头图。图片大小为CIF
 *	   以上两种图像没有字符叠加信息需要配合识别机版本 1.3.1.21 （含）之后才有。
 *
 * B 2016-01-15
 *		1. 新增事件编号6，7，8  需配合识别机程序版本1.3.2.3之后（含）版本。
 *      2. 新增函数 LPNR_GetExtDI， LPNR_GetMachineIP，LPNR_GetPlateAttribute
 *	    3. 初始化识别机时，先测试指定IP的识别机有没有响应UDP查询请求。没有的话表示该IP没有识别机、没有连线或是程序没有运行。
 *          一样返回错误。这样可以在初始化时候立刻知道识别机是不是备妥。不用等超时没有连线。
 *		4. 日志保存位置修改 
 *			- linux: /var/log/lpnr_<ip>.log （/var/log必须存在）
 *          - Windows: ./LOG/LPNR-<IP>.log (注意，程序不会创建LOG文件夹，必须手工创建）
 *
 * C 2016-04-13
 *		1. 新增接口 LPNR_GetCameraLabel
 *
 * D 2016-12-16
 *     1. 新增接口 - LPNR_QueryPlate: 在不先建立TCP连接的情况下，简单的一个调用实现UDP创建，查询车牌号码，关闭UDP，返回结果。
 *          此函数用于对车位相机的轮询。上位机不需要建立大量连接的情况下，一一查询每个车位的当前车牌号码。当车牌号码内容是
 *          “整牌拒识”, 表示没有车辆（或是有车没有挂车牌）。这个函数是给上位机查询停车场车位相机当前车位上车辆的车牌号码。
 *
 * E 2017-06-07
 *     1. 新增接口 
 *			- LPNR_LightCtrl - 控制识别机照明灯（在200A机型，可以使用照明灯ON、OFF讯号控制栏杆机抬落杆。
 *			- LPNR_SetExtDO - 控制扩展DO（需具备扩展IO的机型才有效果)
 * F 2017-07-11
 *     1. 新增接口
 *		  - DLLAPI BOOL CALLTYPE LPNR_SetOSDTimeStamp(HANDLE h, BOOL bEnable, int x, int y);
 *
 * G 2017-09-29
 *		1. 新增识别机脉冲讯号输出控制接口
 *		  - DLLAPI BOOL CALLTYPE LPNR_PulseOut(HANDLE h, int pin, int period, int count);
 *		2. 新增串口透传功能
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_init(HANDLE h, BOOL bEnable, int Speed);
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_aync(HANDLE h, BOOL bEnable);
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_send(HANDLE h, BYTE *data, int len);
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_iqueue(HANDLE h);
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_peek(HANDLE h, BYTE *RxData, int size);
 *		  - DLLAPI BOOL CALLTYPE LPNR_COM_read(HANDLE h, BYTE *RxData, int size);
 *		  - DLLAPI int CALLTYPE LPNR_COM_remove(HANDLE h, int size);
 *		  - DLLAPI int CALLTYPE LPNR_COM_clear(HANDLE h);
 * 
 * H 2018-03-07
 *    1. LPNR_GetPlateAttribute 函数中，参数attr数组内容增加
 *		  attr[5]：车牌颜色代码 (PLATE_COLOR_E)
 *        attr[6]:  车牌类型(PLATE_TYPE_E)
 *    2. 黄绿色车牌的车牌颜色汉字改为"秋", 例如：秋苏E12345. 以便和绿色渐变车牌区别
 *    注： 事件EVT_PLATENUM是提前发送的车牌号码，识别机版本1.3.10.3开始，黄绿色才会发送“秋”，否则还是发送“绿”。
 *            收到EVT_DONE后去获取的车牌号码，则只要是支持新能源车的识别机版本，黄绿色牌都会给“秋”字。因为这是
 *            动态库根据车牌颜色代码给的颜色汉字。
 *
 *  I 2018-03-26
 *    1. 配合线圈触发双向功能（车头，车尾，车头+车尾），识别机提前上报的车牌号码如果是车尾识别，会在车牌号码后面加上后缀
 *        (车尾). 动态库会把这个后缀删除，并且设置属性 attr[1]设置为车尾。在收到EVT_PLATENUM事件后，可以获取车牌号码（LPNR_GetPlateNumber）
 *       以及车头车尾信息（LPNR_GetPlateAttribute），但是此时获取的attribute只有attr[1]是有意义的。其他的成员需要等收到EVT_DONE事件后再调用
 *       一次LPNR_GetPlateAttribute获取。
 *    2. 增加一个事件 EVT_ACK. 识别机收到下行控制帧(DO, PULSE)后，会发送应答帧。DataType是DTYP_ACK, DataId是下行控制帧的控制码 ctrlAttr.code
 *
 * J  2018-03-31
 *    1. LPNR_GetExtDI接口（获取DI值）改为 LPNR_GetExtDIO (获取当前DI和DO的值)
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef linux
#include <error.h>
#endif
#include <errno.h>
#include "lpnrprotocol.h"


// enabler
// #define ENABLE_LOG
// #define ENABLE_BUILTIN_SOCKET_CODE
// #define ENABLE_LPNR_TESTCODE

#define IsValidHeader( hdr)  ( ((hdr).DataType & DTYP_MASK) == DTYP_MAGIC )

#define FREE_BUFFER(buf)	\
	if ( buf ) \
	{ \
		free( buf ); \
		buf = NULL; \
	}

#ifdef linux 
// -------------------------- [LINUX] -------------------------
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// nutalize or mimic windows specific functions
#define winsock_startup()
#define winsock_cleanup()
#define closesocket(fd)	close(fd)
#define WSAGetLastError()	( errno )
// map other windows functions to linux equivalent
#define Sleep(n)		usleep((n)*1000)
#define ASSERT			assert
// nutral macro functions with common format for both windows and linux
#define Mutex_Lock(h)		pthread_mutex_lock(&(h)->hMutex )
#define Mutex_Unlock(h)	pthread_mutex_unlock(&(h)->hMutex )
#define Ring_Lock(h)		pthread_mutex_lock(&(h)->hMutexRing )
#define Ring_Unlock(h)	pthread_mutex_unlock(&(h)->hMutexRing )
#define DeleteObject(h)		pthread_mutex_destroy(&h)
//#define Mutex_Lock(h)	
//#define Mutex_Unlock(h)	
#define min(a,b)	( (a)>(b) ? (b) : (a) )
// log function. 
#ifdef ENABLE_LOG
#define TRACE_LOG(h, fmt...)		trace_log(h, fmt)
#else
#define TRACE_LOG(h, fmt...)		
#endif
#define LOG_FILE	"/var/log/lpnr-%s.log"		// /var/log is the log file director in linux convention
#define BKUP_FILE	"/var/log/lpnr-%s.bak"		// for embedded linux system, /var/log usually is a RAM file system
static void *lpnr_workthread_fxc( void *arg );

unsigned long GetTickCount()
{
	static signed long long begin_time = 0;
	static signed long long now_time;
	struct timespec tp;
	unsigned long tmsec = 0;
	if ( clock_gettime(CLOCK_MONOTONIC, &tp) != -1 )
	{
		now_time = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
	}
	if ( begin_time = 0 )
		begin_time = now_time;
	tmsec = (unsigned long)(now_time - begin_time);
	return tmsec;
}

#else	
// -------------------------- [WINDOWS] -------------------------
#include <winsock2.h>
#include <Ws2tcpip.h>

// common enabler - better put them in VS project setting
//#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRECATE

// nutral macro functions with common format for both windows and linux
#define Mutex_Lock(h)			WaitForSingleObject((h)->hMutex, INFINITE )
#define Mutex_Unlock(h)		ReleaseMutex(  (h)->hMutex )
#define Ring_Lock(h)			WaitForSingleObject((h)->hMutexRing, INFINITE )
#define Ring_Unlock(h)		ReleaseMutex(  (h)->hMutexRing )
static BOOL m_nWinSockRef = 0;

/* 'strip' is IP string, 'sadd_in' is struct sockaddr_in */
#define inet_aton( strip, saddr_in )	( ( (saddr_in) ->s_addr = inet_addr( (strip) ) ) != INADDR_NONE )  
#ifdef ENABLE_LOG
#define TRACE_LOG			trace_log
#else
#define TRACE_LOG     1 ? (void)0 : trace_log
#endif
#define LOG_FILE			"/rwlog/LPNR-%s.log"
#define BKUP_FILE		"/rwlog/LPNR-%s.bak"
DWORD WINAPI lpnr_workthread_fxc(PVOID lpParameter);
#endif

#include "rwlpnrapi.h"


// -------------------------- [END] -------------------------

#define LPNR_PORT			6008
#define MAX_LOGSIZE		4194304		// 4 MB
#define MAGIC_NUM			0xaabbccdd

#define STAT_RUN				1		// work thread running
#define STAT_END			2		// work thread end loop
#define STAT_EXIT			3		// work thread exit

#define MAX_RING_SIZE		1024

typedef enum {
	UNKNOWN=0,
	NORMAL,
	DISCONNECT,
	RECONNECT,
}  LINK_STAT_E;

typedef enum {
	OP_IDLE=0,
	OP_PROCESS,
	OP_RREPORT,
} OP_STAT_E;

typedef struct tagLPNRObject
{
	DWORD	dwMagicNum;
	SOCKET sock;
	char  strIP[16];
	int		status;
	char label[SZ_LABEL];
	LINK_STAT_E	enLink;
	OP_STAT_E		enOper;
	DWORD tickLastHeared;
	int		acked_ctrl_id;			// 收到的下行控制命令的ACK帧DataId, 这个内容就是下行控制帧的控制命令字 header.ctrlAttr.code
#ifdef linux
	pthread_t		hThread;
	pthread_mutex_t		hMutex;
#else	
	HANDLE	hThread;
	HANDLE	hMutex;
	HWND		hWnd;
	int				nMsgNo;
#endif	
	LPNR_callback  cbfxc;
	PVOID		pBigImage;			// input image in jpeg
	int				szBigImage;			// input image size
	PVOID		pSmallImage;			// plate image in BMP (note - Xinluwei is in JPEG
	int				szSmallImage;
	PVOID		pBinImage;
	int				szBinImage;
	PVOID		pHeadImage;
	int				szHeadImage;
	PVOID		pT3DImage;
	int				szT3DImage;
	PVOID		pQuadImage;
	int				szQuadImage;
	PVOID		pLiveImage;
	int				nLiveSize;
	int				nLiveAlloc;
	char			strPlate[12];
	PlateInfo	plateInfo;
	int				process_time;		// in msec
	int				elapsed_time;		// in msec
	// 触发识别原因
	TRIG_SRC_E enTriggerSource;		// TRIG_SRC_E 枚举类型的 int值 
	//	for model with extended GPIO 
	int				diParam;				// 高16位是last DI value，低16位是current DI value
	short			dio_val[2];			// [0]是DI，[1]是DO. 这是抓拍机在客户端连接或是下命令CTRL_READEXTDIO上报的
														// dio_val[0]应该永远和diParam的低16位相同。
	// 版本讯息
	VerAttr		verAttr;
	// 摄像机配置讯息
	ParamConf	paramConf;
	ExtParamConf extParamConf;
	DataAttr		dataCfg;
	H264Conf	h264Conf;
	OSDConf	osdConf;
	OSDPayload osdPayload;
	// 透明串口数据接收ring buffer (for model with external serial port)
#ifdef linux
	pthread_mutex_t		hMutexRing;
#else	
	HANDLE	hMutexRing;
#endif
	int		ring_head, ring_tail;
	BYTE	ring_buf[MAX_RING_SIZE];
} HVOBJ,		*PHVOBJ;

#define IS_VALID_OBJ(pobj)	( (pobj)->dwMagicNum==MAGIC_NUM )

#define NextTailPosit(h)		( (h)->ring_tail==MAX_RING_SIZE-1 ? 0 : (h)->ring_tail+1 )
#define NextHeadPosit(h)	( (h)->ring_head==MAX_RING_SIZE-1 ? 0 : (h)->ring_head+1 )
#define IsRingEmpty(h)		( (h)->ring_head==(h)->ring_tail )
#define IsRingFull(h)			( NextTailPosit(h) == (h)->ring_head )
#define RingElements(h)		(((h)->ring_tail >= (h)->ring_head) ? ((h)->ring_tail - (h)->ring_head) : (MAX_RING_SIZE - (h)->ring_head + (h)->ring_tail) )
#define	PrevPosit(h,n)		( (n)==0 ? MAX_RING_SIZE-1 : (n)-1 )
#define	NextPosit(h,n)		( (n)==MAX_RING_SIZE-1 ? 0 : (n)+1 )
#define RingBufSize(h)		( MAX_RING_SIZE-1 )
// pos 是在ring_buf里的位置，返回是第几个数据（由head开始算是0）
#define PositIndex(h,pos)		( (pos)==(h)->ring_tail ? -1 : ((pos)>=(h)->ring_head ? (pos)-(h)->ring_head : ((MAX_RING_SIZE-(h)->ring_head-1) + (pos))) )
// PositIndex的反操作，idx是由head开始的第几个数据（head是0），返回值是在ring_buffer里的位置。
#define IndexPoist(h,idx)	(  (idx)+(h)->ring_head<MAX_RING_SIZE ? (idx)+(h)->ring_head : (idx+(h)->ring_head+1-MAX_RING_SIZE) )
#define GetRingData(h,pos)		((h)->ring_buf[pos])
#define SetRingData(h,pos, b)		(h)->ring_buf[pos] = (BYTE)(b);

// function prototypes for forward reference
///////////////////////////////////////////////////////////
// TCP functions
#ifdef _WIN32
#define sock_read( s, buf, size )	recv( s, buf, size, 0 )
#define sock_write(s, buf, size )	send( s, buf, size, 0 )
#define sock_close( s )				closesocket( s )
#define WINSOCK_START()		winsock_startup()
#define WINSOCK_STOP()		winsock_cleanup()
#else
#define WINSOCK_START()
#define WINSOCK_STOP()
#endif

#ifdef ENABLE_BUILTIN_SOCKET_CODE
#ifndef linux		// windows plateform
static int winsock_startup()
{
	int rc = 0;
	if ( !m_nWinSockRef )
	{
		WORD wVersionRequested = MAKEWORD( 2, 2 );
		WSADATA wsaData;
		rc = WSAStartup( wVersionRequested, &wsaData );
	}
	if ( rc==0 ) m_nWinSockRef++;
	return rc;
}
static int winsock_cleanup()
{
	if ( m_nWinSockRef>0 && --m_nWinSockRef==0 )
		return WSACleanup();
	else
		return 0;
}
#else		// linux
typedef struct sockaddr_in SOCKADDR_IN;

#endif
static int sock_dataready( SOCKET fd, int tout )
{
	fd_set	rfd_set;
	struct	timeval tv, *ptv;
	int	nsel;

	FD_ZERO( &rfd_set );
	FD_SET( fd, &rfd_set );
	if ( tout == -1 )
	{
		ptv = NULL;
	}
	else
	{
		tv.tv_sec = 0;
		tv.tv_usec = tout * 1000;
		ptv = &tv;
	}
	nsel = select( (int)(fd+1), &rfd_set, NULL, NULL, ptv );
	if ( nsel > 0 && FD_ISSET( fd, &rfd_set ) )
		return 1;
	return 0;
}

static SOCKET sock_connect( const char *strIP, int port )
{
	SOCKET 			fd;
	struct sockaddr_in	destaddr;

	memset( & destaddr, 0, sizeof(destaddr) );
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = htons( (short)port );
  if ((inet_aton(strIP, & destaddr.sin_addr)) == 0)
		return INVALID_SOCKET;
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)   return INVALID_SOCKET;

	if ( connect(fd, (struct sockaddr *)&destaddr, sizeof(destaddr)) < 0 )
	{
		sock_close(fd);
		return INVALID_SOCKET;
	}
	return fd;
}

static int sock_read_n_bytes(SOCKET fd, void* buffer, int n)
{
	char *ptr = (char *)buffer;
	int len;

	while( n > 0 ) 
	{
		//len = recv(fd, ptr, n, MSG_WAITALL);
		if ( sock_dataready(fd, 3000)==0 ) break;		// data time out. in 1000 msec no following TCP package
		len = recv(fd, ptr, n, 0);
		if( len <= 0)			// socket broken. sender close it or directly connected device (computer) power off
			return -1;
		ptr += len;
		n -= len;
	}
	return (int)(ptr-(char*)buffer);
}

static int sock_skip_n_bytes(SOCKET fd, int n)
{
	char buffer[ 1024 ];
	int len;
	int left = n;

	while( left > 0 ) 
	{
		//len = recv(fd, ptr, n, MSG_WAITALL);
		if ( sock_dataready(fd, 1000)==0 ) break;		// data time out. in 1000 msec no following TCP package
		len = recv(fd, buffer, min( n, sizeof( buffer ) ), 0);
		if( len == SOCKET_ERROR )
			return -1;
		if( len <= 0 ) break;
		left -= len;
	}
	return (n-left);
}

// drain all data currently in socket input buffer
static int sock_drain( SOCKET fd )
{
	int	n = 0;
	char	buf[ 1024 ];

	while ( sock_dataready( fd, 0 ) )
		n += recv(fd, buf, sizeof(buf), 0);
	return n;
}

static int sock_drain_until( SOCKET fd, unsigned char *soh, int ns )
{
	char buffer[1024];
	char *ptr;
	int i, len, nskip=0;
	while ( sock_dataready(fd,300) )
	{
		/* peek the data, but not remove the data from the queue */
		if ( (len = recv(fd, buffer, sizeof(buffer), MSG_PEEK)) == SOCKET_ERROR || len<=0 )
			return -1;

		/* try to locate soh sequence in buffer */
		for(i=0, ptr=buffer; i<=len-ns; i++, ptr++) 
			if ( 0==memcmp( ptr, soh, ns) )
				break;
		nskip += (int)(ptr - buffer);
		if ( i > len-ns )
			recv( fd, buffer, len, 0 );
		else
			recv( fd, buffer, (int)(ptr-buffer), 0 );
		if ( i <= len-ns )
			break;
	}
	return nskip;
}

static SOCKET sock_udp_open()
{
	return socket(AF_INET,SOCK_DGRAM,0);		
}
static int sock_udp_timeout( SOCKET sock, int nTimeOut )
{
	return setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeOut, sizeof ( nTimeOut ));
}
static int sock_udp_send0( SOCKET udp_fd, DWORD dwIP, int port, const char * msg, int len )
{
	SOCKADDR_IN	udp_addr;
	SOCKET sockfd;
	int	 tmp = 1, ret;
	BOOL bBroadcast=TRUE;
	
	if ( udp_fd == INVALID_SOCKET )
	{
		sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
		if( sockfd == INVALID_SOCKET ) return SOCKET_ERROR;
		if ( dwIP == INADDR_BROADCAST )
			setsockopt( sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&bBroadcast, sizeof(BOOL) );
		else
    	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&tmp, sizeof(tmp));
	}
	else
		sockfd = udp_fd;

	memset( & udp_addr, 0, sizeof(udp_addr) );
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_addr.s_addr = dwIP;		// NOTE - dwIP is already network format. DON'T USE htonl(dwIP)
	udp_addr.sin_port = htons( (short)port );
	if ( len == 0 )
		len = (int)strlen( msg );

	ret = sendto( sockfd, msg, len, 0, (const struct sockaddr *) & udp_addr, sizeof(udp_addr) );

	if ( udp_fd == INVALID_SOCKET )
		sock_close( sockfd );

	return ret;
}

static int sock_udp_recv( SOCKET sock, char *buf, int size, DWORD *IPSender )
{
	SOCKADDR_IN  sender;
	int		addrlen = sizeof( sender );
	int		ret;

	if ( (ret = recvfrom( sock, buf, size, 0, (struct sockaddr *)&sender, &addrlen )) > 0 )
	{
		if ( IPSender != NULL )
			*IPSender = sender.sin_addr.s_addr;
	}
	return ret;
}
#else
#include <utils_net.h>
#endif

//////////////////////////////////////////////////////////////
static const char* time_stamp()
{
	static char timestr[64];
#ifdef linux
	char *p = timestr;
	struct timeval tv;
	gettimeofday( &tv, NULL );

	strftime( p, sizeof(timestr), "%y/%m/%d %H:%M:%S", localtime( &tv.tv_sec ) );
	sprintf( p + strlen(p), ".%03lu", tv.tv_usec/1000 );

#else
		SYSTEMTIME	tnow;
		GetLocalTime( &tnow );
		sprintf( timestr, "%04d/%02d/%02d %02d:%02d:%02d.%03d", 
					tnow.wYear, tnow.wMonth, tnow.wDay, 
					tnow.wHour, tnow.wMinute, tnow.wSecond, tnow.wMilliseconds );
#endif
	return timestr;
}
static void trace_log(PHVOBJ hObj, const char *fmt,...)
{
	va_list		va;
	char		str[ 1024 ] = "";
	char		file[128];
	FILE *fp;
	
	va_start(va, fmt);
	vsprintf(str, fmt, va);
	va_end(va);
	sprintf(file, LOG_FILE, hObj ? hObj->strIP : "null" );
	fp = fopen( file, "a" );
	if ( fp!=NULL && ftell(fp) > MAX_LOGSIZE )
	{
		char bkfile[128];
		fclose(fp);
		sprintf(bkfile, BKUP_FILE, hObj ? hObj->strIP : "null" );
		remove( bkfile );
		rename( file, bkfile );
		fp = fopen( file, "a" );
	}
	if ( fp != NULL )
	{
		if ( fmt[0] != '\t' )
			fprintf(fp, "[%s] %s", time_stamp(), str );
		else
			fprintf(fp, "%26s%s", " ", str+1 );
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////
// LPNR API
static const char *GetEventText( int evnt )
{
	switch (evnt)
	{
	case EVT_ONLINE	:	return "车牌识别机连线";
	case EVT_OFFLINE:	return "车牌识别机离线";
	case EVT_FIRED:		return "开始识别处理";
	case EVT_DONE:		return "识别结束，数据已接收完成";
	case EVT_LIVE:		return "实时帧更新";
	case EVT_VLDIN:		return "车辆进入虚拟线圈检测区";
	case EVT_VLDOUT:	return "车辆离开虚拟线圈检测区";
	case EVT_EXTDI:		return "扩展DI点状态变化";
	case EVT_SNAP:		return "接收到抓拍帧";
	case EVT_ASYNRX:	return "接收到透传串口输入数据";
	case EVT_PLATENUM: return "接收到提前发送的车牌号";
	case EVT_VERINFO:	return "收到识别机的软件版本讯息";
	case EVT_ACK:			return "收到下行控制帧的ACK回报";
	}
	return "未知事件编号";
}

static const char *GetTriggerSourceText(TRIG_SRC_E enTrig)
{
	switch(enTrig)
	{
	case IMG_UPLOAD:
		return "上位机返送图片识别";
	case IMG_HOSTTRIGGER:
		return "上位机软触发";
	case IMG_LOOPTRIGGER:
		return "定时自动触发";
	case IMG_AUTOTRIG:
		return "定时自动触发";
	case IMG_VLOOPTRG:
		return "虚拟线圈触发";
	case IMG_OVRSPEED:
		return "超速触发";
	}
	return "Unknown Trigger Source";
}

static void NoticeHostEvent( PHVOBJ hObj, int evnt  )
{
	if ( evnt != EVT_LIVE )
		TRACE_LOG((HANDLE)hObj, "发送事件编号 %d  (%s) 给应用程序.\n", evnt, GetEventText(evnt) );

	if ( hObj->cbfxc )  hObj->cbfxc((HANDLE)hObj, evnt);
#ifdef WIN32
	if ( hObj->hWnd )
		PostMessage( hObj->hWnd, hObj->nMsgNo, (DWORD)hObj, evnt );
#endif
}

static void ReleaseData(PHVOBJ pHvObj)
{
	Mutex_Lock(pHvObj);
	FREE_BUFFER ( pHvObj->pBigImage );
	FREE_BUFFER ( pHvObj->pSmallImage );
	FREE_BUFFER ( pHvObj->pBinImage );
	FREE_BUFFER( pHvObj->pHeadImage );
	FREE_BUFFER(pHvObj->pT3DImage);
	FREE_BUFFER( pHvObj->pQuadImage );
	pHvObj->szBigImage = 0;
	pHvObj->szBinImage = 0;
	pHvObj->szSmallImage = 0;
	pHvObj->szHeadImage = 0;
	pHvObj->szT3DImage = 0;
	pHvObj->szQuadImage = 0;
	pHvObj->strPlate[0] = '\0';
	memset( &pHvObj->plateInfo, 0, sizeof(PlateInfo) );
	Mutex_Unlock(pHvObj);
}

#ifdef PROBE_BEFORE_CONNECT
static BOOL TestCameraReady( DWORD dwIP )
{
	DataHeader header;
	int len;
	BOOL bFound = FALSE;
	SOCKET sock = sock_udp_open();

	sock_udp_timeout( sock, 500 );
	SEARCH_HEADER_INIT( header );
	len = sock_udp_send0( sock, dwIP, PORT_SEARCH, (char *)&header, sizeof(DataHeader) );
	if ( (len=sock_udp_recv( sock, (char *)&header, sizeof(DataHeader), &dwIP)) == sizeof(DataHeader) && 
		header.DataId == DID_ANSWER )
		bFound = TRUE;
	sock_close( sock );
	return bFound;
}
#endif

DLLAPI HANDLE CALLTYPE LPNR_Init( const char *strIP )
{
	PHVOBJ hObj = NULL;

#ifdef linux
	struct sockaddr_in 	my_addr;
	if ( inet_aton( strIP, (struct in_addr *)&my_addr)==0 )
#else
	DWORD threadId;

	WINSOCK_START();
	if ( inet_addr(strIP) == INADDR_NONE )
#endif
	{
		return NULL;
	}
#ifdef PROBE_BEFORE_CONNECT
	if ( !TestCameraReady(inet_addr(strIP)) )
	{
		// 识别机没应答
		WINSOCK_STOP();
		return NULL;
	}
#endif
	// 1. 创建对象
	hObj = (PHVOBJ)malloc(sizeof(HVOBJ));
	memset( hObj, 0, sizeof(HVOBJ) );
	strcpy( hObj->strIP, strIP );
	hObj->strIP[15] = '\0';
	hObj->sock = INVALID_SOCKET;
	hObj->dwMagicNum = MAGIC_NUM;

	// 2. 创建Mutex锁 and 启动工作线程

#ifdef linux
	pthread_mutex_init(&hObj->hMutex, NULL );
	pthread_mutex_init(&hObj->hMutexRing, NULL );
	pthread_create( &hObj->hThread, NULL, lpnr_workthread_fxc, (void *)hObj);
#else
	hObj->hMutex = CreateMutex(
							NULL,				// default security attributes
							FALSE,			// initially not owned
							NULL);				// mutex name (NULL for unname mutex)

	hObj->hMutexRing = CreateMutex(
							NULL,				// default security attributes
							FALSE,			// initially not owned
							NULL);				// mutex name (NULL for unname mutex)

	hObj->hThread = CreateThread(
						NULL,						// default security attributes
						0,								// use default stack size
(LPTHREAD_START_ROUTINE)lpnr_workthread_fxc,		// thread function
						hObj,						// argument to thread function
						0,								// thread will be suspended after created.
						&threadId);				// returns the thread identifier
#endif
	TRACE_LOG(hObj,"【LPNR_Init(%s)】 - OK\n", strIP );
	return hObj;
}

DLLAPI BOOL CALLTYPE LPNR_Terminate(HANDLE h)
{
	//int i;
	PHVOBJ pHvObj = (PHVOBJ)h;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
		
	TRACE_LOG(pHvObj, "【LPNR_Terminate】\n");
	if ( pHvObj->hThread )
	{
		pHvObj->status = STAT_END;
#ifndef linux		
		//for(i=0; i<10 && pHvObj->status!=STAT_EXIT; i++ ) Sleep(10);
		WaitForSingleObject(pHvObj->hThread, 100);
		if (  pHvObj->hThread )
		{
			// work thread not terminated. kill it
			TerminateThread( pHvObj->hThread, 0 );
		}
		CloseHandle(pHvObj->hMutex);
		CloseHandle(pHvObj->hMutexRing);
#else
		pthread_cancel( pHvObj->hThread );
		pthread_join(pHvObj->hThread, NULL);
		pthread_mutex_destroy(&pHvObj->hMutex );
		pthread_mutex_destroy(&pHvObj->hMutexRing );
#endif		
	}
	ReleaseData(pHvObj);
	//DeleteObject(pHvObj->hMutex);
	if ( pHvObj->sock != INVALID_SOCKET )
 		closesocket( pHvObj->sock);
	free( pHvObj );	
	WINSOCK_STOP();
	return TRUE;
}


// 设置应用程序的事件回调函数
DLLAPI BOOL CALLTYPE LPNR_SetCallBack(HANDLE h, LPNR_callback mycbfxc )
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
		
	TRACE_LOG(pHvObj,"【LPNR_SetCallBack】\n");
	pHvObj->cbfxc = mycbfxc;
	return TRUE;
}
#if defined _WIN32 || defined _WIN64
// 设置应用程序接收消息的窗口句柄和消息编号
DLLAPI BOOL CALLTYPE LPNR_SetWinMsg( HANDLE h, HWND hwnd, int msgno)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	TRACE_LOG(pHvObj,"【LPNR_SetWinMsg】 - msgno = %d\n", msgno);
	pHvObj->hWnd = hwnd;
	pHvObj->nMsgNo = msgno;
	return TRUE;
}
#endif

DLLAPI BOOL CALLTYPE LPNR_GetPlateNumber(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj, "【LPNR_GetPlateNumber】- 应用程序获取车牌号码 (%s)\n", pHvObj->strPlate);
	// NOTE:
	// let API user program decide to lock or not to prevent dead lock (User invoke LPNR_Lock then call this function will cause dead lock in Linux 
	// as we dont use recusive mutex. application try to lock a mutex which already owned by itself will cause self-deadlock.
	// same reasone for other functions that comment out the Mutex_Lock/Mutex_Unlock
	// It is OK in Windows as Windows's mutex is recursively.
	//Mutex_Lock(pHvObj);		
	strcpy(buf, pHvObj->strPlate );
	//Mutex_Unlock(pHvObj);
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_GetPlateAttribute(HANDLE h, BYTE *attr)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj, "【LPNR_GetPlateAttribute】- 应用程序获取车牌号码 (%s)\n", pHvObj->strPlate);
	memcpy(attr, pHvObj->plateInfo.MatchRate, sizeof(pHvObj->plateInfo.MatchRate));
	return TRUE;
}

DLLAPI int CALLTYPE LPNR_GetPlateColorImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj,"【LPNR_GetPlateColorImage】- 应用程序获取车牌彩色图片 size=%d\n", pHvObj->szSmallImage);
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szSmallImage > 0 )
		memcpy(buf, pHvObj->pSmallImage, pHvObj->szSmallImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szSmallImage;
}

DLLAPI int CALLTYPE LPNR_GetPlateBinaryImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj,"【LPNR_GetPlateBinaryImage】- 应用程序获取车牌二值图片 size=%d\n", pHvObj->szBinImage);
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szBinImage > 0 )
		memcpy(buf, pHvObj->pBinImage, pHvObj->szBinImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szBinImage;
}

DLLAPI int CALLTYPE LPNR_GetCapturedImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj,"【LPNR_GetCapturedImage】- 应用程序获取抓拍图片 size=%d\n", pHvObj->szBigImage );
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szBigImage > 0 )
		memcpy(buf, pHvObj->pBigImage, pHvObj->szBigImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szBigImage;
}

DLLAPI int CALLTYPE LPNR_GetHeadImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj,"【LPNR_GetHeadImage】- 应用程序获取车头图片 size=%d\n", pHvObj->szHeadImage );
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szHeadImage > 0 )
		memcpy(buf, pHvObj->pHeadImage, pHvObj->szHeadImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szHeadImage;
}

DLLAPI int CALLTYPE LPNR_GetMiddleImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj,"【LPNR_GetMiddleImage】- 应用程序获取4/9图片 size=%d\n", pHvObj->szT3DImage );
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szT3DImage > 0 )
		memcpy(buf, pHvObj->pT3DImage, pHvObj->szT3DImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szT3DImage;
}

DLLAPI int CALLTYPE LPNR_GetQuadImage(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj,"【LPNR_GetQuadImage】- 应用程序获取1/4图片 size=%d\n", pHvObj->szQuadImage );
	//Mutex_Lock(pHvObj);
	if ( pHvObj->szQuadImage > 0 )
		memcpy(buf, pHvObj->pQuadImage, pHvObj->szQuadImage );
	//Mutex_Unlock(pHvObj);
	return pHvObj->szQuadImage;
}

DLLAPI int CALLTYPE LPNR_GetLiveFrame(HANDLE h, char *buf)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	//Mutex_Lock(pHvObj);
	if ( pHvObj->nLiveSize > 0 )
		memcpy(buf, pHvObj->pLiveImage, pHvObj->nLiveSize );
	//Mutex_Unlock(pHvObj);
	return pHvObj->nLiveSize;
}

DLLAPI int CALLTYPE LPNR_GetLiveFrameSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	return pHvObj->nLiveSize;
}

DLLAPI BOOL CALLTYPE LPNR_IsOnline(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	//TRACE_LOG(pHvObj,"【LPNR_IsOnline】 - %s\n", pHvObj->enLink == NORMAL ? "yes" : "no");
	return pHvObj->enLink == NORMAL;
}

DLLAPI int CALLTYPE LPNR_GetPlateColorImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj,"【LPNR_GetPlateColorImageSize】- return %d\n", pHvObj->szSmallImage);
	return pHvObj->szSmallImage;
}

DLLAPI int CALLTYPE LPNR_GetPlateBinaryImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;

	TRACE_LOG(pHvObj, "【LPNR_GetPlateBinaryImageSize】 - return %d\n",  pHvObj->szBinImage);
	return pHvObj->szBinImage;
}

DLLAPI int CALLTYPE LPNR_GetCapturedImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj, "【LPNR_GetCapturedImageSize】 - return %d\n", pHvObj->szBigImage);
	return pHvObj->szBigImage;
}

DLLAPI int CALLTYPE LPNR_GetHeadImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj, "【LPNR_GetHeadImageSize】 - return %d\n", pHvObj->szHeadImage);
	return pHvObj->szHeadImage;
}

DLLAPI int CALLTYPE LPNR_GetMiddleImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj, "【LPNR_GetMiddleImageSize】 - return %d\n", pHvObj->szT3DImage);
	return pHvObj->szT3DImage;
}

DLLAPI int CALLTYPE LPNR_GetQuadImageSize(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	TRACE_LOG(pHvObj, "【LPNR_GetQuadImageSize】 - return %d\n", pHvObj->szQuadImage);
	return pHvObj->szQuadImage;
}

DLLAPI BOOL CALLTYPE LPNR_ReleaseData(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj,"【LPNR_ReleaseData】\n" );
	if ( pHvObj->enOper != OP_IDLE )
	{
		TRACE_LOG(pHvObj,"\t - 当前分析尚未完成，不可以释放数据\n");
		return FALSE;
	}
	ReleaseData(pHvObj);
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_SoftTrigger(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj,"【LPNR_SoftTrigger】\n");
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"\t - 抓拍机离线!\n");
		return FALSE;
	}
	if ( pHvObj->enOper != OP_IDLE )
	{
		TRACE_LOG(pHvObj,"\t - 抓拍机忙，当前分析尚未完成!\n");
		return FALSE;
	}
	CTRL_HEADER_INIT( header, CTRL_TRIGGER, 0 );
	sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return TRUE;
}


DLLAPI BOOL CALLTYPE LPNR_SoftTriggerEx(HANDLE h, int Id)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	TRACE_LOG(pHvObj,"[LPNR_SoftTriggerEx]- Id=%d\n", Id);
/*	
	if ( pHvObj->enLink != NORMAL )
	{
		MTRACE_LOG(pHvObj->hLog,"\t - Camera offline!\n");
		return FALSE;
	}
	if ( pHvObj->enOper != OP_IDLE )
	{
		MTRACE_LOG(pHvObj->hLog,"\t - Camera busy, current recognition has not finished yet!\n");
		return FALSE;
	}
*/	
	CTRL_HEADER_INIT( header, CTRL_TRIGGER, Id );
	sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_IsIdle(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
//	TRACE_LOG(pHvObj,"【LPNR_IsIdle】\n");
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"\t - 抓拍机离线!\n");
		return FALSE;
	}
	return ( pHvObj->enOper == OP_IDLE );
}

DLLAPI BOOL CALLTYPE LPNR_GetTiming(HANDLE h, int *elaped, int *processed )
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"\t - 抓拍机离线!\n");
		return FALSE;
	}
	*elaped = pHvObj->elapsed_time;
	*processed = pHvObj->process_time;
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_EnableLiveFrame(HANDLE h, int nSizeCode)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	TRACE_LOG(pHvObj,"【LPNR_EnableLiveFrame(%d)】\n", nSizeCode );
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"\t - 抓拍机离线!\n");
		return FALSE;
	}
	CTRL_HEADER_INIT( header, CTRL_LIVEFEED, nSizeCode );
	sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_TakeSnapFrame(HANDLE h, BOOL bFlashLight)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"LPNR_TakeSnapFrame-  失败，识别机离线!\n");
		return FALSE;
	}
	if ( pHvObj->enOper != OP_IDLE )
	{
		TRACE_LOG(pHvObj,"LPNR_TakeSnapFrame- 识别机正在处理识别中，不接受抓拍图片命令\n");
		return FALSE;
	}
	// 清除数据
	TRACE_LOG(pHvObj,"LPNR_TakeSnapFrame- 触发识别机抓拍并获取图像\n");
	ReleaseData(pHvObj);
	CTRL_HEADER_INIT(header, CTRL_SNAPCAP, bFlashLight ? 1 : 0 );
	sock_write( pHvObj->sock, (char *)&header, sizeof(header) );
/*
	for(i=0; i<100 && pHvObj->szBigImage==0; i++)
		Sleep(10);
	if ( pHvObj->szBigImage==0 )
	{
		TRACE_LOG(pHvObj,"\t--> 识别机超时没有发送抓拍图像！\n" );
		return FALSE;
	}	
*/
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_Lock(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	//TRACE_LOG(pHvObj,"【LPNR_Lock】\n");
	Mutex_Lock(pHvObj);
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_Unlock(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	//TRACE_LOG(pHvObj, "【LPNR_Unlock】\n");
	Mutex_Unlock(pHvObj);
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_SyncTime(HANDLE h)
{
	DataHeader header;
#ifdef WIN32
	SYSTEMTIME tm;
#else
	struct tm *ptm;
	struct timeval tv;
#endif
	PHVOBJ pHvObj = (PHVOBJ)h;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	if ( pHvObj->enLink != NORMAL )
	{
		TRACE_LOG(pHvObj,"LPNR_SyncTime - 失败，识别机离线!\n");
		return FALSE;
	}
	// set camera system time
#ifdef WIN32
	// set camera system time
	GetLocalTime( &tm );
	TIME_HEADER_INIT( header, tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds );
#else
	gettimeofday( &tv, NULL );
	// set camera system time
	ptm = localtime(&tv.tv_sec);
	TIME_HEADER_INIT( header, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
						 ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec/1000 );
#endif
	sock_write( pHvObj->sock, (char *)&header, sizeof(header) );	
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_ResetHeartBeat(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	TRACE_LOG(pHvObj, "[LPNR_ResetHeartBeat]\n");
	pHvObj->tickLastHeared = 0;
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_GetMachineIP(HANDLE h, LPSTR strIP)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	strcpy( strIP, pHvObj->strIP	);
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_GetExtDIO(HANDLE h, short dio_val[])
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	dio_val[0] = pHvObj->dio_val[0];
	dio_val[1] = pHvObj->dio_val[1];
	return TRUE;
}

DLLAPI LPCTSTR CALLTYPE LPNR_GetCameraLabel(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
			return "";
	TRACE_LOG(pHvObj, "【LPNR_GetCameraLabel】- return '%s'\n", pHvObj->label );
	return (LPCTSTR)pHvObj->label;
}

DLLAPI BOOL CALLTYPE LPNR_GetModelAndSensor(HANDLE h, int *modelId, int *sensorId)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	*modelId = pHvObj->verAttr.param[1] >> 16;
	*sensorId = pHvObj->verAttr.param[1] & 0x0000ffff;
	return TRUE;
}
DLLAPI BOOL CALLTYPE LPNR_GetVersion(HANDLE h, DWORD *version, int *algver)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	*version = pHvObj->verAttr.param[0];
	*algver = pHvObj->verAttr.algver;
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_GetCapability(HANDLE h, DWORD *cap)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	*cap = pHvObj->verAttr.param[2];
	return TRUE;
}


DLLAPI BOOL CALLTYPE LPNR_QueryPlate( IN LPCTSTR strIP, OUT LPSTR strPlate, int tout )
{
#if 0
	DataHeader header, rplyHeader;
	int len;
	BOOL bFound = FALSE;
	SOCKET sock;
	DWORD dwIP;

	TRACE_LOG(NULL, "【LPNR_QueryPlate(%s,...)】\n", strIP);
	if ( (dwIP=inet_addr(strIP)) == INADDR_NONE )
	{
		TRACE_LOG(NULL, "==> IP地址错误!\n");
		return FALSE;
	}
	sock = sock_udp_open();
	if ( sock == INVALID_SOCKET )
	{
		TRACE_LOG(NULL, "--> UDP socket open error -- %d\n", WSAGetLastError() );
		return FALSE;
	}
	//sock_udp_timeout( sock, tout );
	QUERY_HEADER_INIT( header, QID_PKPLATE );
	len = sock_udp_send0( sock, dwIP, PORT_SEARCH, (char *)&header, sizeof(DataHeader) );
	if ( sock_dataready(sock, tout) &&
	    (len=sock_udp_recv( sock, (char *)&rplyHeader, sizeof(DataHeader), &dwIP)) == sizeof(DataHeader) )
	{
		if ( rplyHeader.DataType == DTYP_REPLY && rplyHeader.DataId == QID_PKPLATE )
		{
			strcpy(strPlate, rplyHeader.plateInfo.chNum);
			bFound = TRUE;
		}
		else
		{
			TRACE_LOG(NULL, "车位相机应答帧类型(0x%x)或是ID(%d)错误！\n", rplyHeader.DataType, rplyHeader.DataId);
		}
	}
	else
	{
		TRACE_LOG(NULL, "--> 车位相机应答超时!\n");
	}

	sock_close( sock );
	return bFound;
#endif
}

#if defined _WIN32 || defined _WIN64
DLLAPI BOOL CALLTYPE LPNR_WinsockStart( BOOL bStart )
{
	if ( bStart )
		WINSOCK_START();
	else
		WINSOCK_STOP();
	return TRUE;		// 假设永远会成功
}
#endif

DLLAPI BOOL CALLTYPE LPNR_SetExtDO(HANDLE h, int pin, int value)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) ||  pin < 0 || pin >= 4 )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_WRITEEXTDO, 1);
	ctrlHdr.ctrlAttr.option[0] = pin;
	ctrlHdr.ctrlAttr.option[1] = value;
	pHvObj->acked_ctrl_id = 0;
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_PulseOut(HANDLE h, int pin, int period, int count)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len, param;

	if ( !IS_VALID_OBJ(pHvObj) ||  pin < 0 || pin >= 4 )
		return FALSE;

	param = pin + ((count & 0xff) << 8) + ((period & 0xffff) << 16);
	pHvObj->acked_ctrl_id = 0;
	CTRL_HEADER_INIT( ctrlHdr, CTRL_DOPULSE, param);
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_LightCtrl(HANDLE h, int onoff, int msec)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_LIGHT, onoff);
	memcpy(ctrlHdr.ctrlAttr.option, &msec, 4);
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_IRCut(HANDLE h, int onoff)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_IRCUT, onoff);
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_SetOSDTimeStamp(HANDLE h, BOOL bEnable, int x, int y)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if ( bEnable )
		pHvObj->osdConf.enabler |= OSD_EN_TIMESTAMP;
	else
		pHvObj->osdConf.enabler &= ~OSD_EN_TIMESTAMP;
	OSD_HEADER_INIT(header, &pHvObj->osdConf);
	header.size = sizeof(OSDPayload);
	if ( x!=0 && y!=0 )
		pHvObj->osdPayload.enabler |= OSD_EN_TIMESTAMP;
	else
		pHvObj->osdPayload.enabler &= ~OSD_EN_TIMESTAMP;
	pHvObj->osdPayload.x[OSDID_TIMESTAMP] = x;
	pHvObj->osdPayload.y[OSDID_TIMESTAMP] = y;
	len = sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	len += sock_write( pHvObj->sock, (const char*)&pHvObj->osdPayload, sizeof(OSDPayload) );
	return len==sizeof(DataHeader) + sizeof(OSDPayload);
}

DLLAPI BOOL CALLTYPE LPNR_SetOSDLabel(HANDLE h, BOOL bEnable, const char *label)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if ( label != NULL )
	{
		memset( &header, 0, sizeof(header) );
		header.DataType = DTYP_EXCONF;
		header.DataId = 0;
		memcpy(pHvObj->extParamConf.label, label, SZ_LABEL);
		pHvObj->extParamConf.label[SZ_LABEL-1] = '\0';
		memcpy( &header.extParamConf, &pHvObj->extParamConf, sizeof(ExtParamConf) );
		sock_write (pHvObj->sock, (char *)&header, sizeof(header) );
	}

	if ( bEnable )
		pHvObj->osdConf.enabler |= OSD_EN_LABEL;
	else
		pHvObj->osdConf.enabler &= ~OSD_EN_LABEL;
	OSD_HEADER_INIT(header, &pHvObj->osdConf);
	len = sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );

	return len > 0;
}

DLLAPI BOOL CALLTYPE LPNR_SetOSDLogo(HANDLE h, BOOL bEnable)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if ( bEnable )
		pHvObj->osdConf.enabler |= OSD_EN_LOGO;
	else
		pHvObj->osdConf.enabler &= ~OSD_EN_LOGO;
	OSD_HEADER_INIT(header, &pHvObj->osdConf);
	len = sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return len == sizeof(header);
}

DLLAPI BOOL CALLTYPE LPNR_SetOSDROI(HANDLE h, BOOL bEnable)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if ( bEnable )
		pHvObj->osdConf.enabler |= OSD_EN_ROI;
	else
		pHvObj->osdConf.enabler &= ~OSD_EN_ROI;
	OSD_HEADER_INIT(header, &pHvObj->osdConf);
	len = sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return len == sizeof(header);
}

DLLAPI BOOL CALLTYPE LPNR_SetOSDPlate(HANDLE h, BOOL bEnable, int loc, int dwell, BOOL bFadeOut)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;
	
	if ( bEnable )
	{
		pHvObj->osdConf.enabler |= OSD_EN_PLATE;
		if ( loc != 1 && loc != 2 && loc != 3 ) loc = 2;		// 默认是中下
		pHvObj->osdConf.x = -loc - 6;		// 1~3 --> -7 ~ -9
		pHvObj->osdConf.param[OSD_PARM_DWELL] = dwell;
		pHvObj->osdConf.param[OSD_PARM_FADEOUT] = bFadeOut ? 1 : 0;
	}
	else
		pHvObj->osdConf.enabler &= ~OSD_EN_PLATE;
	OSD_HEADER_INIT(header, &pHvObj->osdConf);
	len = sock_write( pHvObj->sock, (const char*)&header, sizeof(header) );
	return len == sizeof(header);
}

DLLAPI BOOL CALLTYPE LPNR_COM_init(HANDLE h, int Speed)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_COMSET, Speed);
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_UserOSDOn(HANDLE h, int x, int y, int align, int fontsz, int text_color, int opacity, const char *text)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	OSDTEXT_HEADER_INIT(header, NULL);
	header.osdText.x  = x;
	header.osdText.y  = y;
	header.osdText.nFontId = 0;		// 宋体
	header.osdText.nFontSize = fontsz;
	header.osdText.RGBForgrund = text_color;
	header.osdText.alpha[0] = opacity * 128 / 100;		// 0~100 --> 0 ~ 128
	header.osdText.alpha[1] = 0;
	if ( 1 < align && align < 4 )
		header.osdText.param[OSD_PARM_ALIGN] = align - 1;		// 1 ~ 3 --> 0 ~ 2
	header.osdText.param[OSD_PARM_SCALE] = 1;
	header.size = strlen(text) + 1;
	len = sock_write( pHvObj->sock, (const char *)&header, sizeof(header) );
	len += sock_write( pHvObj->sock, text, strlen(text) + 1);
	return len == sizeof(header) + header.size;
}

DLLAPI BOOL CALLTYPE LPNR_UserOSDOff(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	OSDTEXT_HEADER_INIT(ctrlHdr, NULL);
	sock_write( pHvObj->sock, (const char *)&ctrlHdr, sizeof(ctrlHdr) );
	return TRUE;
}

DLLAPI BOOL CALLTYPE LPNR_SetStream(HANDLE h,  int encoder, BOOL bSmallMajor, BOOL bSmallMinor)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int  nStreamSize = 0;
	int  len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if ( bSmallMinor )
		nStreamSize = 1;
	if ( bSmallMajor )
		nStreamSize |= 0x02;

	pHvObj->h264Conf.u8Param[1] = nStreamSize;
	pHvObj->h264Conf.u8Param[2] = encoder;

	H264_HEADER_INIT( header, pHvObj->h264Conf);
	len = sock_write( pHvObj->sock, (const char *)&header, sizeof(header) );
	return len==sizeof(header);
}

DLLAPI BOOL CALLTYPE LPNR_SetCaptureImage(HANDLE h, BOOL bDisFull, BOOL bEnMidlle, BOOL bEnSmall, BOOL bEnHead)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader header;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	if (bDisFull)
		pHvObj->paramConf.enabler |= PARM_DE_FULLIMG;
	else
		pHvObj->paramConf.enabler &= ~PARM_DE_FULLIMG;

	if ( bEnMidlle )
			pHvObj->paramConf.enabler |= PARM_EN_T3RDIMG;
	else
			pHvObj->paramConf.enabler &= ~PARM_EN_T3RDIMG;

	if ( bEnHead )
		pHvObj->paramConf.enabler  |= PARM_EN_HEADIMG;
	else
		pHvObj->paramConf.enabler  &= ~PARM_EN_HEADIMG;

	if ( bEnSmall )
			pHvObj->paramConf.enabler |= PARM_EN_QUADIMG;
	else
			pHvObj->paramConf.enabler &= ~PARM_EN_QUADIMG;

	memset( &header, 0, sizeof(header) );
	header.DataType = DTYP_CONF;
	header.DataId = 0;
	memcpy( &header.paramConf, &pHvObj->paramConf, sizeof(ParamConf) );
	len = sock_write (pHvObj->sock, (char *)&header, sizeof(header) );

	return len==sizeof(header);
}

DLLAPI BOOL CALLTYPE LPNR_COM_aync(HANDLE h, BOOL bEnable)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len, param = bEnable ? 1 : 0;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_COMASYN, param);
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	return len==sizeof(DataHeader);
}

DLLAPI BOOL CALLTYPE LPNR_COM_send(HANDLE h, const char *data, int size)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	DataHeader ctrlHdr;
	int len;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	CTRL_HEADER_INIT( ctrlHdr, CTRL_COMTX, 0);
	ctrlHdr.size = size;
	len = sock_write( pHvObj->sock, (const char*)&ctrlHdr, sizeof(ctrlHdr) );
	len += sock_write(pHvObj->sock, data, size);
	return len==sizeof(DataHeader);
}

DLLAPI int CALLTYPE LPNR_COM_iqueue(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;

	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;
	return RingElements(pHvObj);
}

DLLAPI int CALLTYPE LPNR_COM_peek(HANDLE h, char *RxData, int size)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	int  nb, i, pos;

	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;

	Ring_Lock(pHvObj);
	nb = RingElements(pHvObj);
	if ( size > nb )
		size = nb;
	pos = pHvObj->ring_head;
	for(i=0; i<nb; i++)
	{
		RxData[i] = pHvObj->ring_buf[pos];
		pos = NextPosit(pHvObj,pos);
	}
	Ring_Unlock(pHvObj);
	return nb;
}

DLLAPI int CALLTYPE LPNR_COM_read(HANDLE h, char *RxData, int size)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	int  nb, i;

	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;

	Ring_Lock(pHvObj);
	nb = RingElements(pHvObj);
	if ( size > nb )
		size = nb;
	for(i=0; i<nb; i++)
	{
		RxData[i] = pHvObj->ring_buf[pHvObj->ring_head];
		pHvObj->ring_head = NextHeadPosit(pHvObj);
	}
	Ring_Unlock(pHvObj);
	return nb;
}

DLLAPI int CALLTYPE LPNR_COM_remove(HANDLE h, int size)
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	int  nb, i;

	if ( !IS_VALID_OBJ(pHvObj) )
		return 0;

	Ring_Lock(pHvObj);
	nb = RingElements(pHvObj);
	if ( size > nb )
		size = nb;
	for(i=0; i<nb; i++)
		pHvObj->ring_head = NextHeadPosit(pHvObj);
	Ring_Unlock(pHvObj);
	return nb;
}

DLLAPI BOOL CALLTYPE LPNR_COM_clear(HANDLE h)
{
	PHVOBJ pHvObj = (PHVOBJ)h;

	if ( !IS_VALID_OBJ(pHvObj) )
		return FALSE;

	Ring_Lock(pHvObj);
	pHvObj->ring_head = pHvObj->ring_tail = 0;
	Ring_Unlock(pHvObj);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WORKING THREAD
#define IMG_BUFSIZE	(1920*1080*3)		// 6MB
#ifdef linux
void *lpnr_workthread_fxc(void *lpParameter)
#else
DWORD WINAPI lpnr_workthread_fxc(PVOID lpParameter)
#endif
{
	PHVOBJ pHvObj = (PHVOBJ) lpParameter;
	PVOID payload = malloc( IMG_BUFSIZE );
	int	   rlen;
	long   imgWidth, imgHeight, imgType;
	fd_set set;
	struct timeval val;
	int ret;
	DataHeader  dataHeader;
	DataHeader header;
	SOCKET m_sock = INVALID_SOCKET;
	BYTE soh[3] = { 0xaa, 0xbb, 0xcc };
	char *liveFrame = NULL;
	int	  nLiveSize = 0;
	int	  nLiveAlloc = 0;
	DWORD tickHeartBeat=0;
	DWORD tickStartProc = 0;

	TRACE_LOG(pHvObj,"<<< ===================   W O R K   T H R E A D   S T A R T   =====================>>>\n");
	pHvObj->status = STAT_RUN;
	pHvObj->tickLastHeared = 0;
	for(; pHvObj->status==STAT_RUN; )
	{
		if ( m_sock == INVALID_SOCKET )
		{
			TRACE_LOG(pHvObj,"reconnect camera IP %s port 6008...\n", pHvObj->strIP );
			pHvObj->enLink = RECONNECT;
			m_sock = sock_connect( pHvObj->strIP, 6008 );
			TRACE_LOG(pHvObj, "--> socket = 0x%x (%d)\n", m_sock, m_sock );
			if ( m_sock != INVALID_SOCKET )
			{
				DataHeader header;
				pHvObj->enLink = NORMAL;
				pHvObj->sock = m_sock;
				LPNR_SyncTime((HANDLE)pHvObj);
				// disable intermidiate files
				CTRL_HEADER_INIT( header, CTRL_IMLEVEL, 0  );
				sock_write( m_sock, (char *)&header, sizeof(header) );
				TRACE_LOG(pHvObj,"Connection established. download system time to camera.\n");
				NoticeHostEvent(pHvObj, EVT_ONLINE ); 
				pHvObj->tickLastHeared = 0;			// reset last heared time
				pHvObj->nLiveSize = 0;
			}
			else
			{
				Sleep(1000);
				continue;
			}
		}

		// check for sending heart-beat packet
		if ( GetTickCount() >= tickHeartBeat )
		{
			DataHeader hbeat;
			HBEAT_HEADER_INIT(hbeat);
			//TRACE_LOG(pHvObj,"send heartbeat packet to camera.\n");
			sock_write( m_sock, (const char *)&hbeat, sizeof(hbeat) );
			tickHeartBeat = GetTickCount() + 1000;
		}

		// check for input from camera
		FD_ZERO( &set );
		FD_SET( m_sock, &set );
		val.tv_sec = 0;
		val.tv_usec = 10000;
		ret = select( m_sock+1, &set, NULL, NULL, &val );
		if ( ret > 0 && FD_ISSET(m_sock, &set) )
		{
			int rc;
			if ( (rc=sock_read_n_bytes( m_sock, (char *)&dataHeader, sizeof(dataHeader) )) != sizeof(dataHeader) )
			{
				if ( rc<=0 )
				{
					TRACE_LOG(pHvObj,"Socket broken, close and reconnect...wsaerror=%d\n", WSAGetLastError() );
					sock_close( m_sock );
					m_sock = pHvObj->sock = INVALID_SOCKET;
					pHvObj->enLink = DISCONNECT;
					pHvObj->enOper = OP_IDLE;
					tickStartProc = 0;
					ReleaseData(pHvObj);
					NoticeHostEvent( pHvObj, EVT_OFFLINE );
					continue;
				}
				else
				{
					int nskip;
					TRACE_LOG(pHvObj, "读取数据帧头获得的长度是%d, 需要%d字节。重新同步到下一个帧头！\n",
							rc, sizeof(dataHeader) );
					nskip = sock_drain_until( m_sock, soh, 3 );
					TRACE_LOG(pHvObj,"--> 抛弃 %d 字节!\n", nskip);
				}
			}
			pHvObj->tickLastHeared = GetTickCount();
			if ( !IsValidHeader(dataHeader) )
			{
				int nskip;
				TRACE_LOG(pHvObj,"Invalid packet header received 0x%08X, re-sync to next header\n", dataHeader.DataType );
				nskip = sock_drain_until( m_sock, soh, 3 );
				TRACE_LOG(pHvObj,"--> skip %d bytes\n", nskip);
				continue;
			}	
			// read payload after header (if any)
			if ( dataHeader.size > 0 )
			{
				if ( dataHeader.size > IMG_BUFSIZE )
				{
					int nskip;
					// image size over buffer size ignore
					TRACE_LOG(pHvObj, "payload size %d is way too large. re-sync to next header\n", dataHeader.size);
					nskip = sock_drain_until( m_sock, soh, 3 );
					TRACE_LOG(pHvObj, "--> skip %d bytes\n", nskip);
					continue;
				}
				if ( (rlen=sock_read_n_bytes( m_sock, payload, dataHeader.size )) != dataHeader.size  )
				{
					int nskip;
					TRACE_LOG(pHvObj, "read payload DataType=0x%x, DataId=%d, expect %d bytes, only get %d bytes. --> ignored!\n", 
							dataHeader.DataType, dataHeader.DataId, dataHeader.size, rlen );
					nskip = sock_drain_until( m_sock, soh, 3 );
					TRACE_LOG(pHvObj,"--> drop this packet, skip %d bytes\n", nskip);
					//sock_close( m_sock );
					//m_sock = pHvObj->sock = INVALID_SOCKET;
					//pHvObj->enOper = OP_IDLE;
					continue;
				}
			}
			// receive a image file (JPEG or BMP)
			if ( dataHeader.DataType == DTYP_IMAGE )
			{
				// acknowledge it
				ACK_HEADER_INIT( header );
				sock_write( m_sock, (const char *)&header, sizeof(header) );
				// then process
				imgWidth = dataHeader.imgAttr.width;
				imgHeight = dataHeader.imgAttr.height;
				imgType = dataHeader.imgAttr.format;
				// IMAGE is a live frame
				if (dataHeader.DataId == IID_LIVE )
				{
					Mutex_Lock(pHvObj);
					if ( dataHeader.size > pHvObj->nLiveAlloc )
					{
						pHvObj->nLiveAlloc = (dataHeader.size + 1023)/1024 * 10240;		// roundup to 10K boundry
						pHvObj->pLiveImage = realloc( pHvObj->pLiveImage, pHvObj->nLiveAlloc );
					}
					pHvObj->nLiveSize = dataHeader.size;
					memcpy( pHvObj->pLiveImage, payload, dataHeader.size );
					Mutex_Unlock(pHvObj);
					NoticeHostEvent(pHvObj, EVT_LIVE );
				}
				else // if (dataHeader.DataId != IID_LIVE ) - other output images
				{	
					// Processed image
					// printf("received processed image: Id=%d, size=%d\r\n", dataHeader.DataId, dataHeader.size );
					switch( dataHeader.DataId )
					{
					case IID_CAP:
						Mutex_Lock(pHvObj);
						pHvObj->szBigImage = dataHeader.size;
						pHvObj->pBigImage = malloc( pHvObj->szBigImage );
						memcpy( pHvObj->pBigImage, payload, pHvObj->szBigImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj,"==> 接收到抓拍大图 - %s (%d bytes)\n", dataHeader.imgAttr.basename, dataHeader.size);
						if ( strcmp(dataHeader.imgAttr.basename,"vsnap.jpg") == 0 )
							NoticeHostEvent(pHvObj, EVT_SNAP );
						break;
					case IID_HEAD:
						Mutex_Lock(pHvObj);
						pHvObj->szHeadImage = dataHeader.size;
						pHvObj->pHeadImage = malloc( pHvObj->szHeadImage );
						memcpy( pHvObj->pHeadImage, payload, pHvObj->szHeadImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj,"==> 接收到车头图 (%d bytes)\n", dataHeader.size);
						break;
					case IID_T3RD:
						Mutex_Lock(pHvObj);
						pHvObj->szT3DImage = dataHeader.size;
						pHvObj->pT3DImage = malloc( pHvObj->szT3DImage );
						memcpy( pHvObj->pT3DImage, payload, pHvObj->szT3DImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj,"==> 接收到4/9解像度图 (%d bytes)\n", dataHeader.size);
						break;
					case IID_QUAD:
						Mutex_Lock(pHvObj);
						pHvObj->szQuadImage = dataHeader.size;
						pHvObj->pQuadImage = malloc( pHvObj->szQuadImage );
						memcpy( pHvObj->pQuadImage, payload, pHvObj->szQuadImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj,"==> 接收到1/4解像度图 (%d bytes)\n", dataHeader.size);
						break;
					case IID_PLRGB:
						Mutex_Lock(pHvObj);
						pHvObj->szSmallImage = dataHeader.size;
						pHvObj->pSmallImage = malloc( pHvObj->szSmallImage );
						memcpy( pHvObj->pSmallImage, payload, pHvObj->szSmallImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj, "==> 接收到车牌彩色图 (%d bytes)\n", dataHeader.size);
						break;
					case IID_PLBIN:
						Mutex_Lock(pHvObj);
						pHvObj->szBinImage = dataHeader.size;
						pHvObj->pBinImage = malloc( pHvObj->szBinImage );
						memcpy( pHvObj->pBinImage, payload, pHvObj->szBinImage );
						Mutex_Unlock(pHvObj);
						TRACE_LOG(pHvObj, "==> 接收到车牌二值图 (%d bytes)\n", dataHeader.size);
						break;
					default:
						// ignore for other images
						;
					}
				}  // else if not live image
			}
			else if ( dataHeader.DataType == DTYP_DATA )
			{
				switch (dataHeader.DataId )
				{
				case DID_BEGIN:
					ReleaseData(pHvObj);
					memset( &pHvObj->plateInfo, 0, sizeof(PlateInfo) );
					strcpy( pHvObj->strPlate, "  整牌拒识" );
					pHvObj->enOper = OP_RREPORT;
					break;
				case DID_END:
					pHvObj->enOper = OP_IDLE;
					tickStartProc = 0;
					// 设置其他车牌结果属性值
					// 0 为 信心度 0~100
					// 1 为 车头车尾信息 1 车头，0xff 车尾，0 未知
					// 2 为车身颜色代码 （不大可靠）
					pHvObj->plateInfo.MatchRate[3] = (BYTE)dataHeader.dataAttr.val[2];	// 车速
					pHvObj->plateInfo.MatchRate[4] = (BYTE)dataHeader.dataAttr.val[1];	// 触发源
					TRACE_LOG(pHvObj, "==>识别结束，数据接收完成!\n");
					NoticeHostEvent(pHvObj, EVT_DONE );
					break;
				case DID_PLATE:
					Mutex_Lock(pHvObj);
					memcpy( &pHvObj->plateInfo, &dataHeader.plateInfo, sizeof(PlateInfo) );
					switch( pHvObj->plateInfo.plateCode & 0x00ff )
					{
					case PLC_BLUE:
						strcpy( pHvObj->strPlate, "蓝" );
						break;
					case PLC_YELLOW:
						strcpy( pHvObj->strPlate, "黄" );
						break;
					case PLC_WHITE:
						strcpy( pHvObj->strPlate, "白" );
						break;
					case PLC_BLACK:
						strcpy( pHvObj->strPlate, "黑" );
						break;
					case PLC_GREEN:
						strcpy( pHvObj->strPlate, "绿" );
						break;
					case PLC_YELGREEN:
						strcpy( pHvObj->strPlate, "秋" );
						break;
					default:
						strcpy( pHvObj->strPlate, "蓝" );
					}
					strcat( pHvObj->strPlate, pHvObj->plateInfo.chNum );
					TRACE_LOG(pHvObj, "==> 接收到识别的车牌 【%s】\n", pHvObj->strPlate );
					pHvObj->plateInfo.MatchRate[5] = (BYTE)pHvObj->plateInfo.plateCode & 0x00ff;	// 车牌颜色代码
					pHvObj->plateInfo.MatchRate[6] = (BYTE)((pHvObj->plateInfo.plateCode>>8) & 0xff);	// 车牌类型代码
					Mutex_Unlock(pHvObj);
					break;
				case DID_TIMING:
					pHvObj->process_time = dataHeader.timeInfo.totalProcess;
					pHvObj->elapsed_time = dataHeader.timeInfo.totalElapsed;
					break;
				case DID_COMDATA:
					if ( dataHeader.size > 0 )
					{
						int  i=0;
						BYTE *rx_data = (BYTE *)payload;
						TRACE_LOG(pHvObj, "Receive COM port RX data (%d bytes) - save to ring buffer. Current ring elements=%d\n", dataHeader.size, RingElements(pHvObj));
						Ring_Lock(pHvObj);
						for( ; !IsRingFull(pHvObj) && dataHeader.size > 0; )
						{
							pHvObj->ring_buf[pHvObj->ring_tail] = rx_data[i++];
							pHvObj->ring_tail = NextTailPosit(pHvObj);
							dataHeader.size--;
						}
						Ring_Unlock(pHvObj);
					}
					break;
				case DID_EXTDIO:
					pHvObj->dio_val[0] = dataHeader.dataAttr.val[0] & 0xffff;
					pHvObj->dio_val[1] = dataHeader.dataAttr.val[1] & 0xffff;
					break;
				case DID_VERSION:
					memcpy(&pHvObj->verAttr, &dataHeader.verAttr, sizeof(VerAttr));
					NoticeHostEvent(pHvObj, EVT_VERINFO );
					break;
				case DID_CFGDATA:
					memcpy(&pHvObj->dataCfg, &dataHeader.dataAttr, sizeof(DataAttr));
					break;
				}	// switch (dataHeader.DataId )
			}	// else if ( dataHeader.DataType == DTYP_DATA )
			else if ( dataHeader.DataType == DTYP_EVENT )
			{
				switch(dataHeader.DataId)
				{
				case EID_TRIGGER:
					pHvObj->enOper = OP_PROCESS;
					tickStartProc = GetTickCount();
					pHvObj->enTriggerSource = (TRIG_SRC_E)dataHeader.evtAttr.param;
					TRACE_LOG(pHvObj, "==> 识别机触发识别 (%s)，开始处理!\n", GetTriggerSourceText(pHvObj->enTriggerSource) );
					NoticeHostEvent(pHvObj, EVT_FIRED );
					break;
				case EID_VLDIN:
					TRACE_LOG(pHvObj, "==> 车辆进入虚拟线圈识别区!\n");
					NoticeHostEvent(pHvObj, EVT_VLDIN );
					break;
				case EID_VLDOUT:
					TRACE_LOG(pHvObj, "==> 车辆离开虚拟线圈识别区!\n");
					NoticeHostEvent(pHvObj, EVT_VLDOUT );
					break;
				case EID_EXTDI:
					pHvObj->diParam = dataHeader.evtAttr.param;
					pHvObj->dio_val[0] = pHvObj->diParam & 0xffff;
					TRACE_LOG(pHvObj, "==> EXT DI 状态变化 0x%x -> 0x%x\n", 
							(pHvObj->diParam >> 16) & 0xffff, (pHvObj->diParam & 0xffff));
					NoticeHostEvent(pHvObj, EVT_EXTDI );
					break;
				}
			}
			else 	if ( dataHeader.DataType == DTYP_CONF )
			{
				memcpy(&pHvObj->paramConf, &dataHeader.paramConf, sizeof(ParamConf));
			}
			else 	if ( dataHeader.DataType == DTYP_EXCONF )
			{
				memcpy(&pHvObj->extParamConf, &dataHeader.extParamConf, sizeof(ExtParamConf));
				memcpy( pHvObj->label, &dataHeader.extParamConf.label, SZ_LABEL);
				pHvObj->label[SZ_LABEL-1] = '\0';
				TRACE_LOG(pHvObj, "收到扩展配置参数 - label=%s\n", pHvObj->label);
			}
			else 	if ( dataHeader.DataType == DTYP_CONF )
			{
				memcpy(&pHvObj->paramConf, &dataHeader.paramConf, sizeof(ParamConf));
			}
			else if ( dataHeader.DataType == DTYP_H264CONF )
			{
				memcpy( &pHvObj->h264Conf, &dataHeader.h264Conf, sizeof(H264Conf) );
			}
			else if ( dataHeader.DataType == DTYP_OSDCONF )
			{
				memcpy( &pHvObj->osdConf, &dataHeader.osdConf, sizeof(OSDConf) );
				if ( dataHeader.size==sizeof(OSDPayload) )
					sock_read_n_bytes(m_sock, &pHvObj->osdPayload, sizeof(OSDPayload));
			}
			else if ( dataHeader.DataType==DTYP_TEXT &&  dataHeader.DataId==TID_PLATENUM )
			{
				// 收到提前发送的车牌号码
				char *ptr;
				// 如果车牌号码有后缀 (车尾)，例如："蓝苏E12345(车尾)"，需要把后缀砍掉
				if ( (ptr=strchr(dataHeader.textAttr.text,'('))!=NULL)
				{
					*ptr = '\0';
					pHvObj->plateInfo.MatchRate[1] = 0xff;
				}
				strcpy(pHvObj->strPlate ,dataHeader.textAttr.text);
				NoticeHostEvent(pHvObj, EVT_PLATENUM );
				TRACE_LOG(pHvObj, "收到提前发送的车牌号码: %s\n", pHvObj->strPlate);
			}
			else if ( dataHeader.DataType==DTYP_ACK && dataHeader.DataId!=0 )
			{
				// 收到控制应答帧
				pHvObj->acked_ctrl_id = dataHeader.DataId;
				NoticeHostEvent(pHvObj, EVT_ACK );
				TRACE_LOG(pHvObj, "收到控制命令 %d的应答帧!\n", pHvObj->acked_ctrl_id);
			}
		}	// if ( ret > 0 && FD_ISSET(pHvObj->sock, &set) )
		if ( pHvObj->tickLastHeared && (GetTickCount() - pHvObj->tickLastHeared > 10000) )
		{
			// over 10 seconds not heared from LPNR camera. consider socket is broken 
			sock_close( m_sock );
			m_sock = pHvObj->sock = INVALID_SOCKET;
			pHvObj->enLink = DISCONNECT;
			pHvObj->nLiveSize = 0;
			ReleaseData(pHvObj);
			NoticeHostEvent(pHvObj, EVT_OFFLINE );
			TRACE_LOG(pHvObj,"Camera heart-beat not heared over 10 sec. reconnect.\n");
		}
		if ( pHvObj->enOper==OP_PROCESS && (GetTickCount() - tickStartProc) > 2000 )
		{
			TRACE_LOG(pHvObj,"LPNR 分析图片时间超过2秒，复位状态到闲置!\n");
			pHvObj->enOper = OP_IDLE;
		}
	}
	pHvObj->status = STAT_EXIT;
	closesocket( m_sock );
	m_sock = pHvObj->sock = INVALID_SOCKET;
	pHvObj->enLink = DISCONNECT;
	pHvObj->hThread = 0;
	FREE_BUFFER(pHvObj->pLiveImage);
	ReleaseData(pHvObj);
	free(payload);
	TRACE_LOG(pHvObj,"-----------------    W O R K    T H R E A D   E X I T   ------------------\n");
	return 0;
 }
 
#if defined linux && defined D_ENABLE_LPNR_TESTCODE
#include <termios.h>

static struct termios tios_save;

static int ttysetraw(int fd)
{
	struct termios ttyios;
	if ( tcgetattr (fd, &tios_save) != 0 )
		return -1;
	memcpy(&ttyios, &tios_save, sizeof(ttyios) );
	ttyios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                      |INLCR|IGNCR|ICRNL|IXON);
	ttyios.c_oflag &= ~(OPOST|OLCUC|ONLCR|OCRNL|ONOCR|ONLRET);
	ttyios.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	ttyios.c_cflag &= ~(CSIZE|PARENB);
	ttyios.c_cflag |= CS8;
	ttyios.c_cc[VMIN] = 1;
	ttyios.c_cc[VTIME] = 0;
	return tcsetattr (fd, TCSANOW, &ttyios);
}

static int ttyrestore(int fd)
{
	return tcsetattr(fd, TCSANOW, &tios_save);
}

int tty_ready( int fd )
{
	int	n=0;
 	fd_set	set;
	struct timeval tval = {0, 10000};		// 10 msec timed out

	FD_ZERO(& set);
	FD_SET(fd, &set);
	n = select( fd+1, &set, NULL, NULL, &tval );

	return n;
}

void lpnr_event_handle( HANDLE h, int evnt )
{
	PHVOBJ pHvObj = (PHVOBJ)h;
	int i, n, size;
	char *buf;
	char chIP[16];
	char rx_buf[1024];

	LPNR_GetMachineIP(h, chIP);
	switch (evnt)
	{
	case EVT_ONLINE:
		printf("LPNR camera (IP %s) goes ONLINE.\r\n", chIP );
		break;
	case EVT_OFFLINE:
		printf("LPNR camera IP %s goes OFFLINE.\r\n", pHvObj->strIP );
		break;
	case EVT_FIRED:
		printf("LPNR camera IP %s FIRED a image process.\r\n", chIP );
		break;
	case EVT_DONE:
		printf("LPNR camera IP %s processing DONE\r\n", pHvObj->strIP );
		size = LPNR_GetCapturedImageSize(h);
		if ( size == 0 ) size = 1024*1024;
		buf = malloc(size);
		LPNR_GetPlateNumber(h, buf);
		printf("\t plate number = %s\r\n", buf );
		n = LPNR_GetCapturedImage(h, buf );
		printf("\t captured image size is %d bytes.\r\n", n );
		n = LPNR_GetPlateColorImage(h, buf );
		printf("\t plate color image size is %d bytes.\r\n", n );
		n = LPNR_GetPlateBinaryImage(h, buf );
		printf("\t plate binary image size is %d bytes.\r\n", n );
		free( buf );
		LPNR_ReleaseData((HANDLE)h);
		break;
	case EVT_LIVE:
		LPNR_Lock(h);
		size = LPNR_GetLiveFrameSize(h);
		buf = malloc(size);
		LPNR_GetLiveFrame(h, buf);
		LPNR_Unlock(h);
		// do some thing for live frame (render on screen for example)., but do not do it in this thread context
		// let other render thread do the job. We don't want to block LPNR working thread for too long.
		printf("reveive a live frame size=%d bytes\r\n", size );
		free(buf);
		break;
	case EVT_SNAP:
		printf("snap image reveived, size=%d bytes\r\n", LPNR_GetCapturedImageSize(h));
		break;
	case EVT_ASYNRX:
		printf("Serial port data Rx (%d bytes) -- ", (n=LPNR_COM_iqueue(h)));
		LPNR_COM_read(h, rx_buf, sizeof(rx_buf));
		rx_buf[n] = '\0';
		printf("%s\r\n", rx_buf);
	}
}

/*
 *  after launched, user use keyboard to control the opeation. stdin is set as raw mode.
 *  therefore, any keystroke will be catched immediately without have to press <enter> key.
 *  'q' - quit
 *  'l' - soft trigger.
 */
int generate_random_string(char buf[], int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		buf[i] = 32 + (random() % 95);
	}
	return size;
}

int main( int argc, char *const argv[] )
{
	int ch;
	BOOL bQuit = FALSE;
	BOOL bOnline = FALSE;
	int		 lighton = 0;
	int     IRon = 0;
	int		fd;
	HANDLE hLPNR;
	int    n, pin=0, value=0, period;
	char tx_buf[64];

	if ( argc != 2 )
	{
		fprintf(stderr, "USUAGE: %s <ip addr>\n", argv[0] );
		return -1;
	}
	fd = 0;		// stdin
	ttysetraw(fd);
	if ( (hLPNR=LPNR_Init(argv[1])) == NULL )
	{
		ttyrestore(fd);
		fprintf(stderr, "Invalid LPNR IP address: %s\r\n", argv[1] );
		return -1;
	}
	LPNR_SetCallBack( hLPNR, lpnr_event_handle );

	srandom(time(NULL));

	for(;!bQuit;)
	{
		BOOL bOn = LPNR_IsOnline(hLPNR);
		if ( bOn != bOnline )
		{
			bOnline = bOn;
			printf("camera become %s\r\n", bOn ? "online" : "offline" );
		}
		if ( tty_ready(fd) && read(fd, &ch, 1)==1 )
		{
			ch &= 0xff;
			switch (ch)
			{
			case '0':
			case '1':
			case '2':
				pin = ch - '0';
				printf("choose pin number %d\r\n", pin);
				break;
			case 'a':		// must issue 'i' command first (once only)
				LPNR_COM_aync(hLPNR,TRUE);
				printf("enable COM port async Rx\r\n");
				break;
			case 'A':
				LPNR_COM_aync(hLPNR,FALSE);
				printf("disable COM port async Rx\r\n");
				break;
			case 'l':
				printf("manual trigger a recognition...\r\n");
				LPNR_SoftTrigger( hLPNR );
				break;
			case 'd':	// disable live
				printf("disable live feed.\r\n");
				LPNR_EnableLiveFrame( hLPNR, FALSE );
				break;
			case 'e':	// enable live
				printf("enable live feed.\r\n");
				LPNR_EnableLiveFrame( hLPNR, TRUE );
				break;
			case 'f':		// 'f' - flash light
				lighton = 1 - lighton;
				printf("light control, on/off=%d\r\n", lighton);
				LPNR_LightCtrl(hLPNR, lighton, 0);
				break;
			case 'F':		// IR cut
				IRon = 1 - IRon;
				printf("IR-cut control, on/off=%d\r\n", IRon);
				LPNR_IRCut(hLPNR, IRon);
				break;
			case 'i':		// initial COM port
				LPNR_COM_init(hLPNR,9600);
				printf("initial transparent COM port!\r\n");
				break;
			case 'n':
				printf("camera name (label) = %s\r\n",  LPNR_GetCameraLabel(hLPNR));
				break;
			case 'o':		// Ext DO
				value = 1 - value;
				LPNR_SetExtDO(hLPNR, pin, value);
				printf("outpput DO pin=%d, value=%d\r\n", pin, value);
				break;
			case 'p':		// pulse
			case 'P':
				n = random() % 10 + 1;
				period = ch=='p' ? 250 : 500;
				printf("output pulse at %d msec period for %d times on pin %d.\r\n",  period, n,  pin);
				LPNR_PulseOut(hLPNR, pin, period, n);
				break;
			case 'q':
				printf("Quit test program...\r\n");
				bQuit = TRUE;
				break;
			case 's':
			case 'S':
				printf("take s snap frame...\r\n");
				LPNR_TakeSnapFrame(hLPNR,ch=='S');
				break;
			case 't':
				printf("time sync with camera.\r\n");
				LPNR_SyncTime(hLPNR);
				break;
			case 'x':
				// generate random string Tx to COM
				n = generate_random_string(tx_buf, random()%60+1);
				LPNR_COM_send(hLPNR, tx_buf, n);
				tx_buf[n] = '\0';
				printf("Tx %d bytes: %s\r\n",  n, tx_buf);
				break;
			case 'u':	// turn off user OSD
				LPNR_UserOSDOff(hLPNR);
				break;
			case 'U':	// turn on user OSD
				LPNR_UserOSDOn(hLPNR,-5,0,2,40,0xFFFF00,80,"Overlay Line 1\nMiddle Line\nLast Line is the longest");
				break;
			case 't':		 // turn off time stamp OSD
				LPNR_SetOSDTimeStamp(hLPNR,FALSE,0,0);
				break;
			case 'T':
				LPNR_SetOSDTimeStamp(hLPNR,TRUE,0,0);
				break;
			default:
				printf("ch=%c (%d), ignored.\r\n", ch, ch );
				break;
			}
		}
	}	
	printf("terminate working thread and destruct hLPNR...\r\n");
	LPNR_Terminate(hLPNR);
	ttyrestore(fd);
	return 0;
}

#endif
