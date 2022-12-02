# A simple EPICS driver example for PCI devices using asynPortDriver on Linux

This is an EPICS IOC developed for the LLRF system of a small-scale accelerator facility at CSNS. The LLRF board is a customized cPCI FPGA board with PCI 9056 bridge, the control interface is register access and waveform read with 1 second update period, no interrupt or DMA is needed.

## Use case

This example is only useful for simple customized PCI devices that does not need interrupt and DMA, and only one MMIO (Memory mapped I/O) BAR in used to access the FPGA registers.

## Hardware platform

* One 6U cPCI crate
* One CPU board: MIC-3396 6U CompactPCI with 4th Generation Intel Processor
* One LLRF I/O board: customized cPCI FPGA board

## Software environment

* Phoebus 4.6.6
* EPICS 3.15.9
* asyn R4-43
* Ubuntu 20.04 LTS
* Linux kernel 5.15.0
* gcc 9.4.0

## Design

### Requirement

The requirement for the LLRF board merely includes register access and waveform read, the update period of waveforms is 1 second. Therefore, no interrupt or DMA is needed. For part of the registers, a conversion formula is needed on the IOC side.

### Software/hardware interface definition

To simplify the implementation of hardware and software, all the registers including integer values as well as control/status bits in FPGA are defined as 32-bit signed value.

### Implementation

In short, the driver part is implemented as kernel module rather than UIO (Userspace I/O), and the IOC part is implemented using asynPortDriver.

The implementation includes Linux kernel module PCI driver, user-space PCI driver API, EPICS driver support using asynPortDriver and EPICS record.

The read/write operation between user space and kernel space is implemented using ioctl() system call.

### Source code structure

```
cpciEpicsApp/kernelModule/pci_driver_llrf/pci_driver_llrf.c: Linux kernel module for PCI devices
cpciEpicsApp/kernelModule/pci_driver_llrf/pci_driver_llrf.h
cpciEpicsApp/kernelModule/pci_driver_llrf/test.c: A customized command-line tool to test PCI register access
cpciEpicsApp/iocBoot/iocCpciApp/st.cmd
cpciEpicsApp/cpciApp/cpciLLRF.db
cpciEpicsApp/cpciApp/opi/llrf.bob
cpciEpicsApp/cpciApp/src/cpciAccess.c: User-space API for Linux kernel module
cpciEpicsApp/cpciApp/src/cpciAccess.h
cpciEpicsApp/cpciApp/src/cpciDefs.h: Data structure and ioctl() command definitions
cpciEpicsApp/cpciApp/src/cpciLLRF.cpp: EPICS driver support using asynPortDriver
cpciEpicsApp/cpciApp/src/cpciLLRF.h
cpciEpicsApp/cpciApp/src/cpciLLRF.dbd
```

### Architecture

![Alt text](docs/screenshots/architecture.png?raw=true "Title")

## How to run

```
$ cd cpciEpicsApp/kernelModule
$ make
# make load
```
```
$ cd cpciEpicsApp
$ make
$ cd iocBoot/iocCpciApp
$ ./st.cmd
```

### Phoebus OPI

![Alt text](docs/screenshots/opi.png?raw=true "Title")

## How to apply to other PCI devices

### Step 1. Specify BAR index and vendor/device ID

```
cpciEpicsApp/kernelModule/pci_driver_llrf/pci_driver_llrf.c
```

![Alt text](docs/screenshots/bar_vendor_device.png?raw=true "Title")

### Step 2. Define FPGA registers

```
cpciEpicsApp/cpciApp/src/cpciLLRF.cpp
```

![Alt text](docs/screenshots/register.png?raw=true "Title")

### Step 3. Specify conversion formula between FPGA register and EPICS record if any

```
cpciEpicsApp/cpciApp/src/cpciLLRF.cpp
```

![Alt text](docs/screenshots/formula.png?raw=true "Title")

## Debug method

### Kernel log Info

Logs printed in kernel can be displayed using "dmesg".

```
$ dmesg -wH
```

![Alt text](docs/screenshots/dmesg.png?raw=true "Title")

### Command line tool

```
cpciEpicsApp/kernelModule/pci_driver_llrf/test.c
```

![Alt text](docs/screenshots/shell.png?raw=true "Title")

Waveform read and save

![Alt text](docs/screenshots/waveform_read.png?raw=true "Title")

![Alt text](docs/screenshots/waveform_data.png?raw=true "Title")
