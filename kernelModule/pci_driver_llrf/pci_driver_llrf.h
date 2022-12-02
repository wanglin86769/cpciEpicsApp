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


typedef int STATUS;

/* Type definitions */
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;

#define OK 0
#define ERROR -1

/* Addresses for 14 waveforms are consecutive */
#define WAVEFORM_OFFSET 0x00040000 /* Waveform offset in FPGA */
#define WAVEFORM_NUMBER 14 /* 14 waveforms in total */
#define WAVEFORM_POINT 4096 /* 4096 points for each waveform */
#define WAVEFORM_DATA_BYTE 2 /* 16 bits for each point */
#define WAVEFORM_LENGTH    WAVEFORM_NUMBER*WAVEFORM_POINT*WAVEFORM_DATA_BYTE


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
