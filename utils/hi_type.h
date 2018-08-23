/*
 * hi_types.h
 * redefine many data types used in many Hi SDK based application.
 */
 
 #ifndef __HI_TYPE_H__
 #define __HI_TYPE_H__
 
typedef unsigned char  HI_U8;
typedef unsigned short  HI_U16;
typedef unsigned int    HI_U32;

typedef signed char     HI_S8;
typedef short						HI_S16;
typedef int							HI_S32;
#ifdef WIN32
typedef __int64         HI_U64;
typedef __int64         HI_S64;
#else
typedef unsigned long long       HI_U64;
typedef signed long long         HI_S64;
#endif

typedef char              HI_CHAR;
#define HI_VOID           void

typedef enum {
    HI_FALSE = 0,
    HI_TRUE  = 1,
} HI_BOOL;


#endif
 