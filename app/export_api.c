#include "export_api.h"
#include <stdio.h>
#include <stdlib.h>
#include "lpnr/rwlpnrapi.h"
#include "GuangDong_OSD.h"

static void Real_OSD_Text(int type, const char *text, int param);
typedef struct tagOSDInfo
{
	char siteName[128];     //�շ�վ
	char laneName[128];     //����
	char collector[128];    //�շ�Ա
	char vehicleType[128];  //����
	char vehicleClass[128]; //����
	char payMent[128];      //�շѽ��
}OSDTextS;

static OSDTextS Info = {
    { "�շ�վ��\t\r\n" },
	{ "������\t\r\n" },
	{ "�շ�Ա��\t\r\n" },
	{ "���ͣ�\t\r\n" },
	{ "���ࣺ\t\r\n" },
	{ "�շѽ�\t\r\n"}
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
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
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
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
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
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
		return -1;
	}
	Real_OSD_Text(2, (const char *)lpszTollTypeName, 0);
	return 0;
}

// ����Ӧ�ս��
 int  OSD_Fare(int nFare)
{
	char fee[128] = {0};
	int a = nFare / 100;
	int b = ( nFare % 100) / 10;
	printf("[OSD_Fare] Money :%d \r\n", nFare );
	// char tail[3] = { "Ԫ" };
	if (0 == could_run())
	{
		if (lpnr)
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
		return -1;
	}
	if ( nFare >= 0 )
	{
		// sprintf(fee, sizeof(fee), "%dԪ", a);
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
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
		return -1;
	}
	// �շ�վ��
	Real_OSD_Text(4, (const char *)lpszStaName, 0);
	// �շ�վID
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
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
		return -1;
	}
	if (lpszDateTime == NULL)
	{
		printf("���ö�ʱ�����������Ϊ�գ�ʹ��ϵͳʱ���ʱ..\r\n");
		LPNR_SyncTime(lpnr);
	}
	else{
		printf("��ȡ��ʱ��ֵ[%s]��׼����ʱ..\r\n", ptr );
		sscanf(ptr, "%04d%02d%02d%02d%02d%02d[.%03d]", &year, &mon, &day, &hour, &mins, &second, &mills);
		printf("��ȡ����ֵΪ%04d-%02d-%02d %02d:%02d:%02d[.%03d]", year, mon, day, hour, mins, second, mills );
		//LPNR_SyncForTime(lpnr, year, mon, day, hour, mins, second, mills);
	}
	return 0;
}

int  OSD_Clear()
{
	if (0 == could_run() )
	{
		if (lpnr)
			printf("����״̬�޷�����\r\n");
		else
			printf("LPNR��δ��ʼ�����޷�����OSD���������Ƚ��г�ʼ������..\r\n");
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
		//д�շ�Ա����
		if (param)
			sprintf(Info.collector, "�շ�Ա��%s\t\r\n", text);
		else
			sprintf(Info.collector, "�շ�Ա��\t\r\n");	
	}
	else if (type == 1)
	{
		sprintf( Info.vehicleType, "���ͣ�%s\t\r\n", text);
	}
	else if ( type == 2)
	{
		sprintf( Info.vehicleClass, "���ࣺ%s\t\r\n", text);
	}
	else if (type == 3)
	{
		sprintf(Info.payMent, "�շѽ�%s Ԫ\t\r\n", text);
	}
	else if (type == 4)
	{
		sprintf(Info.siteName, "�շ�վ��%s\t\r\n", text);
	}
	else if (type == 5)
	{
		sprintf(Info.laneName, "������%s\t\r\n", text);
	}
	else{
		printf("δʶ��ĵ�������..\r\n");
	}
	sprintf(TotalBuffer, "%s%s%s%s%s%s", Info.siteName, Info.laneName,
		Info.vehicleType, Info.vehicleClass, Info.collector, Info.payMent );
	printf("OSD Text :%s \r\n", TotalBuffer);
	LPNR_UserOSDOn(lpnr, -1, 100, 0, 48, 0xffffffff, 100, TotalBuffer);
}

