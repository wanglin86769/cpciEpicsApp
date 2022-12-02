/** 
 * shell.c - cPCI Linux driver test module 
 * 
 * Author: Lin Wang
 * Email: wanglin@ihep.ac.cn
 * Create date: November 2nd, 2022
 */ 

/* Includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h> // Time measurement

#include "cpciDefs.h"

                           
int openDevice(char *deviceName) 
{
    int fd;
    if((fd = open(deviceName, O_RDWR)) == ERROR)
    {
    	printf("open(): failed to open the device file %s!\n", deviceName);
    } 
    return fd;
}


int closeDevice(int fd)
{
    return close(fd);
}


/* UINT8 register data read */
STATUS uint8Read(int fd, UINT32 addr, UINT8 *retData) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioctl(fd, RD_VALUE_8, ioValue);
    *retData = ioValue->value_8;
    return OK;
}


/* UINT8 register data write */
STATUS uint8Write(int fd, UINT32 addr, UINT8 data) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioValue->value_8 = data;
    ioctl(fd, WR_VALUE_8, ioValue);
    return OK;
}


/* UINT16 register data read */
STATUS uint16Read(int fd, UINT32 addr, UINT16 *retData) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioctl(fd, RD_VALUE_16, ioValue);
    *retData = ioValue->value_16;
    return OK;
}


/* UINT16 register data write */
STATUS uint16Write(int fd, UINT32 addr, UINT16 data) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioValue->value_16 = data;
    ioctl(fd, WR_VALUE_16, ioValue);
    return OK;
}


/* UINT32 register data read */
STATUS uint32Read(int fd, UINT32 addr, UINT32 *retData) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioctl(fd, RD_VALUE_32, ioValue);
    *retData = ioValue->value_32;
    return OK;
}


/* UINT32 register data write */
STATUS uint32Write(int fd, UINT32 addr, UINT32 data) 
{
    IO_VALUE _ioValue;
    IO_VALUE *ioValue = &_ioValue;
    ioValue->offset = addr;
    ioValue->value_32 = data;
    ioctl(fd, WR_VALUE_32, ioValue);
    return OK;
}


/* Waveform read, this function copy waveform data byte by byte, the speed is too slow */
STATUS waveformReadOld(int fd, UINT32 addr, int length, char *buffer) 
{
    IO_WAVEFORM _ioWaveform;
    IO_WAVEFORM *ioWaveform = &_ioWaveform;
    ioWaveform->offset = addr;
    ioWaveform->length = length;
    ioWaveform->buffer = buffer;
    ioctl(fd, RD_WAVEFORM, ioWaveform);
    return OK;
}


/* Waveform read, this function copy waveform data 4-bytes by 4-bytes */
STATUS waveformRead(int fd, UINT32 addr, int length, char *buffer) 
{
    if(length % 4 != 0) {
        return ERROR;
    }
    int count_of_4bytes = length / 4;
	for(int i=0; i<count_of_4bytes; i++) {
	    if(uint32Read(fd, addr + i * 4, (UINT32 *)buffer + i) != OK) {
	        printf("waveformRead(): uint32Read() returns ERROR!\n");
	        return ERROR;
	    }
	}
    return OK;
}
