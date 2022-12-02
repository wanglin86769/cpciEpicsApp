/**
 * pci_driver_llrf.h -- Definitions for cPCI Linux driver
 *                      Ubuntu 20.04 LTS
 *                      Linux kernel 5.15.0
 *
 * Author: Lin Wang (CSNS)
 * Email:  wanglin@ihep.ac.cn
 * Date:   November 1, 2022
 *
 */
 
#ifndef CPCI_DEFS_H
#define CPCI_DEFS_H


typedef int STATUS;

/* Type definitions */
typedef unsigned char UINT8;
typedef char INT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned int UINT32;
typedef int INT32;

#define OK 0
#define ERROR -1


/**
 *	Definitions for ioctl()
 */ 
typedef struct _IO_VALUE
{
	UINT32 offset;
	UINT8 value_8;
    UINT16 value_16;
    UINT32 value_32;
} IO_VALUE;

typedef struct _IO_WAVEFORM
{
	UINT32 offset;
    int length;
    char *buffer;
} IO_WAVEFORM;


#define RD_VALUE_8   _IOR('a', 'a', IO_VALUE *)
#define WR_VALUE_8   _IOW('a', 'b', IO_VALUE *)

#define RD_VALUE_16  _IOR('a', 'c', IO_VALUE *)
#define WR_VALUE_16  _IOW('a', 'd', IO_VALUE *)

#define RD_VALUE_32  _IOR('a', 'e', IO_VALUE *)
#define WR_VALUE_32  _IOW('a', 'f', IO_VALUE *)

#define RD_WAVEFORM  _IOR('a', 'g', IO_WAVEFORM *)


#endif
