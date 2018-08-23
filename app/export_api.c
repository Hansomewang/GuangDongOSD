#include "export_api.h"
#include <stdio.h>
#include <stdlib.h>
#include "lpnr/rwlpnrapi.h"
#include "GuangDong_OSD.h"

static void Real_OSD_Text(int type, const char *text, int param);
typedef struct tagOSDInfo
{
	char siteName[128];     //收费站
	char laneName[128];     //车道
	char collector[128];    //收费员
	char vehicleType[128];  //车型
	char vehicleClass[128]; //车类
	char payMent[128];      //收费金额
}OSDTextS;

static OSDTextS Info = {
    { "收费站：\t\r\n" },
	{ "车道：\t\r\n" },
	{ "收费员：\t\r\n" },
	{ "车型：\t\r\n" },
	{ "车类：\t\r\n" },
	{ "收费金额：\t\r\n"}
};

static int could_run()
{
    return (lpnr != NULL && on_line == 1) ? 1 : 0;
}

int  OSD_Close()
{
	if( lpnr )
	{
		LPNR_UserOSDOff( lpnr );
		LPNR_Terminate( lpnr );
		printf("Close Lpnr Success..\r\n");
		lpnr = NULL;
		return 0;
	}else{
		printf("lpnr handle null, can not failed..\r\n");
		return -1;
	}
}

int  OSD_Collector(const char * lpszCollectorId, BOOL bAddOffDuty)
{
	int show = bAddOffDuty > 0 ? 0 : 1;
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	
	Real_OSD_Text(0, (const char *)lpszCollectorId, show );
	return 0;
}

 int  OSD_VehClass(const char * lpszVehClassName)
{
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	Real_OSD_Text(1, (const char *)lpszVehClassName, 0);
	return 0;
}

 int  OSD_VehType(const char * lpszTollTypeName)
{
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	Real_OSD_Text(2, (const char *)lpszTollTypeName, 0);
	return 0;
}

// 叠加应收金额
 int  OSD_Fare(int nFare)
{
	char fee[128] = {0};
	int a = nFare / 100;
	int b = ( nFare % 100) / 10;
	printf("[OSD_Fare] Money :%d \r\n", nFare );
	// char tail[3] = { "元" };
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	if ( nFare >= 0 )
	{
		// sprintf(fee, sizeof(fee), "%d元", a);
		sprintf(fee, "%d", a );
		printf("[OSD_Fare] set Money %s \r\n", fee);
	}
	else{
		printf("[OSD_Fare] set Money empty \r\n" );
		strcpy(fee, "  ");
	}
	Real_OSD_Text(3, (const char *)fee, 0);
	return 0;
}

int  OSD_Lane(const char *lpszStaName, const char *lpszLaneId)
{
	printf("Input:[%s],[%s]", lpszStaName, lpszLaneId);
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	// 收费站名
	Real_OSD_Text(4, (const char *)lpszStaName, 0);
	// 收费站ID
	Real_OSD_Text(5, (const char *)lpszLaneId, 0);
	return 0;
}

 int  OSD_DateTime(const char * lpszDateTime)
{
	int year, mon, day, hour, mins, second, mills;
	const char *ptr = (const char *)lpszDateTime;
	if (0 == could_run())
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	if (lpszDateTime == NULL)
	{
		printf("调用对时函数输入参数为空，使用系统时间对时..\r\n");
		LPNR_SyncTime(lpnr);
	}
	else{
		printf("获取到时间值[%s]，准备对时..\r\n", ptr );
		sscanf(ptr, "%04d%02d%02d%02d%02d%02d[.%03d]", &year, &mon, &day, &hour, &mins, &second, &mills);
		printf("获取到的值为%04d-%02d-%02d %02d:%02d:%02d[.%03d]", year, mon, day, hour, mins, second, mills );
		//LPNR_SyncForTime(lpnr, year, mon, day, hour, mins, second, mills);
	}
	return 0;
}

int  OSD_Clear()
{
	if (0 == could_run() )
	{
		if (lpnr)
			printf("离线状态无法调用\r\n");
		else
			printf("LPNR还未初始化，无法进行OSD操作，请先进行初始化操作..\r\n");
		return -1;
	}
	LPNR_UserOSDOn(lpnr, -1, 100, 0, 48, 0xffffffff, 100, "" );
	return 0;
}

static void Real_OSD_Text(int type, const char *text, int param)
{
	char buffer[128] = { 0 };
	char TotalBuffer[256] = { 0 };
	int i = 0;

	if (type == 0)
	{
		//写收费员名字
		if (param)
			sprintf(Info.collector, "收费员：%s\t\r\n", text);
		else
			sprintf(Info.collector, "收费员：\t\r\n");	
	}
	else if (type == 1)
	{
		sprintf( Info.vehicleType, "车型：%s\t\r\n", text);
	}
	else if ( type == 2)
	{
		sprintf( Info.vehicleClass, "车类：%s\t\r\n", text);
	}
	else if (type == 3)
	{
		sprintf(Info.payMent, "收费金额：%s 元\t\r\n", text);
	}
	else if (type == 4)
	{
		sprintf(Info.siteName, "收费站：%s\t\r\n", text);
	}
	else if (type == 5)
	{
		sprintf(Info.laneName, "车道：%s\t\r\n", text);
	}
	else{
		printf("未识别的叠加数据..\r\n");
	}
	sprintf(TotalBuffer, "%s%s%s%s%s%s", Info.siteName, Info.laneName,
		Info.vehicleType, Info.vehicleClass, Info.collector, Info.payMent );
	printf("OSD Text :%s \r\n", TotalBuffer);
	LPNR_UserOSDOn(lpnr, -1, 100, 0, 48, 0xffffffff, 100, TotalBuffer);
}

