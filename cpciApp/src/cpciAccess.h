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
#ifndef CPCI_ACCESS_H
#define CPCI_ACCESS_H


#include "cpciDefs.h"


int openDevice(char *deviceName);

int closeDevice(int fd);

STATUS uint8Read(int fd, UINT32 addr, UINT8 *retData);

STATUS uint8Write(int fd, UINT32 addr, UINT8 data);

STATUS uint16Read(int fd, UINT32 addr, UINT16 *retData);

STATUS uint16Write(int fd, UINT32 addr, UINT16 data);

STATUS uint32Read(int fd, UINT32 addr, UINT32 *retData);

STATUS uint32Write(int fd, UINT32 addr, UINT32 data);

STATUS waveformRead(int fd, UINT32 addr, int length, char *buffer);


#endif
