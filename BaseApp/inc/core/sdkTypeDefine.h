#ifndef __SDKTYPEDEFINE_H
#define __SDKTYPEDEFINE_H

//#include "machinecfg.h"

#undef  NULL
#define NULL        0

#define REG8(addr)          (*(volatile UINT8 *) (addr))
#define REG16(addr)          (*(volatile UINT16 *)(addr))
#define REG32(addr)          (*(volatile UINT32 *)(addr))

//------------------------------------------------------------------
//                        TypeDefs
//------------------------------------------------------------------
#if 0
typedef    unsigned char            UINT8;    ///<unsigned char
typedef    signed char       INT8;    ///< char

typedef    unsigned short        UINT16;    ///<unsigned char
typedef    signed short      INT16;    ///<short

typedef unsigned int      UINT32;    ///<unsigned int
typedef    signed int        INT32;    ///<int

typedef unsigned char            BOOL;    ///<BOOL

typedef unsigned int      uint32_t;
typedef unsigned short    uint16_t;
typedef unsigned char     uint8_t;

typedef signed short             int16_t;
typedef signed int                 int32_t;
typedef signed char             int8_t;
#endif

typedef unsigned int             U32;
typedef unsigned short         U16;
typedef unsigned char         U8;
typedef  short                 S16;
typedef  int                 S32;
typedef char              S8;
typedef unsigned long long         U64;

typedef unsigned int             u32;
typedef unsigned short         u16;
typedef unsigned char         u8;
typedef  short                 s16;
typedef  int                 s32;
typedef  char                 s8;
typedef  long long             s64;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;


#ifndef bool
typedef unsigned char  bool;
#endif

#ifndef BOOL
typedef unsigned int BOOL;
#endif

#ifndef false 
#define false 0
#endif

#ifndef true 
#define true  1

#endif



typedef unsigned int loff_t;
typedef unsigned int mm_segment_t;


typedef     int     DQFILE; 



#define BIT_0     (0x1ul << 0)
#define BIT_1     (0x1ul << 1)
#define BIT_2     (0x1ul << 2)
#define BIT_3     (0x1ul << 3)
#define BIT_4     (0x1ul << 4)
#define BIT_5     (0x1ul << 5)
#define BIT_6    (0x1ul << 6)
#define BIT_7     (0x1ul << 7)
#define BIT_8     (0x1ul << 8)
#define BIT_9     (0x1ul << 9)
#define BIT_10   (0x1ul << 10)
#define BIT_11     (0x1ul << 11)
#define BIT_12     (0x1ul << 12)
#define BIT_13     (0x1ul << 13)
#define BIT_14     (0x1ul << 14)
#define BIT_15     (0x1ul << 15)
#define BIT_16     (0x1ul << 16)
#define BIT_17     (0x1ul << 17)
#define BIT_18     (0x1ul << 18)
#define BIT_19     (0x1ul << 19)
#define BIT_20     (0x1ul << 20)
#define BIT_21     (0x1ul << 21)
#define BIT_22     (0x1ul << 22)
#define BIT_23     (0x1ul << 23)
#define BIT_24     (0x1ul << 24)
#define BIT_25     (0x1ul << 25)
#define BIT_26     (0x1ul << 26)
#define BIT_27     (0x1ul << 27)
#define BIT_28     (0x1ul << 28)
#define BIT_29     (0x1ul << 29)
#define BIT_30     (0x1ul << 30)
#define BIT_31     (0x1ul << 31)


#endif
