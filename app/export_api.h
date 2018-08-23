#ifndef EXPORT_HEAD_H
#define EXPORT_HEAD_H

#include "wintype.h"

#ifdef __cplusplus
extern "C" {
#endif  
int  OSD_Clear();
int  OSD_Lane(const char *lpszStaName, const char * lpszLaneId);
int  OSD_Collector(const char * lpszCollectorId, BOOL bAddOffDuty);
int  OSD_VehClass(const char * lpszVehClassName);
int  OSD_VehType(const char * lpszTollTypeName);
// 叠加应收金额
int  OSD_Fare(int nFare);
int  OSD_DateTime(const char * lpszDateTime);
//123
#ifdef __cplusplus
}
#endif

#endif
