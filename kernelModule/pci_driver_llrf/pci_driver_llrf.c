/**
 * pci_driver_llrf.c -- cPCI Linux driver
 *                      Ubuntu 20.04 LTS
 *                      Linux kernel 5.15.0
 *
 *	      This driver handles single board;
 *        This driver handles only register access, without interrupt or DMA interface.
 *
 * Author: Lin Wang (CSNS)
 * Email:  wanglin@ihep.ac.cn
 * Date:   November 1, 2022
 *
 */
 
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>

#include <asm/siginfo.h>
#include <linux/pid.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/pid_namespace.h>

#include <linux/ktime.h> /* Time measurement */

#include "pci_driver_llrf.h"


/* Read and write registers */
#define	   REG_READ8(addr)             (*(volatile UINT8 *)(addr))
#define    REG_WRITE8(addr, value)     (*((volatile UINT8 *)(addr))=(value))
#define	   REG_READ16(addr)            (*(volatile UINT16 *)(addr))
#define	   REG_WRITE16(addr, value)    (*((volatile UINT16 *)(addr))=(value))
#define	   REG_READ32(addr)            (*(volatile UINT32 *)(addr))
#define	   REG_WRITE32(addr, value)    (*((volatile UINT32 *)(addr))=(value))


typedef struct _PCI_DEV_CSR_INFO
{
	
	UINT16 vendorId;
	UINT16 deviceId;
	UINT16 command;
	UINT16 status;
	
	UINT32 classRevision;
	UINT8 revisionId;
	UINT8 classProg;
	UINT16 classDevice;

	UINT8 cacheLineSize;
	UINT8 latencyTimer;
	UINT8 headerType;
	UINT8 bist;

	UINT32 baseAddress0;
	UINT32 baseAddress1;
	UINT32 baseAddress2;
	UINT32 baseAddress3;
	UINT32 baseAddress4;
	UINT32 baseAddress5;

	UINT32 cardBusCIS;
	UINT16 subsystemVendorId;
	UINT16 subsystemDeviceId;

	UINT32 romAddress;

	UINT8 capabilityList;

	UINT8 interruptLine;
	UINT8 interruptPin;
	UINT8 minGnt;
	UINT8 maxLat;
} PCI_DEV_CSR_INFO;


typedef struct _PCI_BAR_INFO
{
	unsigned long start;
	unsigned long end;
	long rang;
	long flag;
	void __iomem *remapAddr;    // The virtual memory address mapping to the start of BAR
} PCI_BAR_INFO;


typedef struct _PCI_DEV_INFO
{
	PCI_DEV_CSR_INFO pciDevCSRInfo;
	PCI_BAR_INFO pciBarInfoList[6];
} PCI_DEV_INFO;


/* Device name*/
#define DEVICE_NAME "pci_driver_llrf"

/* Driver major number */
#define PCI_MAJOR 193

/* For the LLRF board, BAR 2 is used to access registers of the on-board FPGA */
#define BAR_INDEX 2

/* PCI device information */
PCI_DEV_INFO *pciDevInfo;

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lin Wang");
MODULE_DESCRIPTION("Implements register access interface for LLRF cPCI board");


/* PCI device ID table */
static struct pci_device_id pciIdTb[] = {
	{ PCI_DEVICE(0x10b5, 0x9056) },
	{ 0, }  // Terminating entry with zero value
};
MODULE_DEVICE_TABLE(pci, pciIdTb);


/**
 *	Read the first 64 bytes of PCI device configuration space register
 */
static STATUS getPciDevCSRInfo(struct pci_dev *dev, PCI_DEV_INFO *localPciDevInfo)
{
    pci_read_config_word(dev, PCI_VENDOR_ID, &(localPciDevInfo->pciDevCSRInfo.vendorId));
	pci_read_config_word(dev, PCI_DEVICE_ID, &(localPciDevInfo->pciDevCSRInfo.deviceId));
	pci_read_config_word(dev, PCI_COMMAND, &(localPciDevInfo->pciDevCSRInfo.command));
	pci_read_config_word(dev, PCI_STATUS, &(localPciDevInfo->pciDevCSRInfo.status));

	pci_read_config_dword(dev, PCI_CLASS_REVISION, &(localPciDevInfo->pciDevCSRInfo.classRevision));
	pci_read_config_byte(dev, PCI_REVISION_ID, &(localPciDevInfo->pciDevCSRInfo.revisionId));
	pci_read_config_byte(dev, PCI_CLASS_PROG, &(localPciDevInfo->pciDevCSRInfo.classProg));
	pci_read_config_word(dev, PCI_CLASS_DEVICE, &(localPciDevInfo->pciDevCSRInfo.classDevice));

	pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &(localPciDevInfo->pciDevCSRInfo.cacheLineSize));
	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &(localPciDevInfo->pciDevCSRInfo.latencyTimer));
	pci_read_config_byte(dev, PCI_HEADER_TYPE, &(localPciDevInfo->pciDevCSRInfo.headerType));
	pci_read_config_byte(dev, PCI_BIST, &(localPciDevInfo->pciDevCSRInfo.bist));

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &(localPciDevInfo->pciDevCSRInfo.baseAddress0));
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, &(localPciDevInfo->pciDevCSRInfo.baseAddress1));
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &(localPciDevInfo->pciDevCSRInfo.baseAddress2));
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_3, &(localPciDevInfo->pciDevCSRInfo.baseAddress3));
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_4, &(localPciDevInfo->pciDevCSRInfo.baseAddress4));
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_5, &(localPciDevInfo->pciDevCSRInfo.baseAddress5));

	pci_read_config_dword(dev, PCI_CARDBUS_CIS, &(localPciDevInfo->pciDevCSRInfo.cardBusCIS));

	pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID, &(localPciDevInfo->pciDevCSRInfo.subsystemVendorId));
	pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &(localPciDevInfo->pciDevCSRInfo.subsystemDeviceId));

	pci_read_config_dword(dev, PCI_ROM_ADDRESS, &(localPciDevInfo->pciDevCSRInfo.romAddress));

    pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &(localPciDevInfo->pciDevCSRInfo.capabilityList));

	pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &(localPciDevInfo->pciDevCSRInfo.interruptLine));
	pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &(localPciDevInfo->pciDevCSRInfo.interruptPin));

	pci_read_config_byte(dev, PCI_MIN_GNT, &(localPciDevInfo->pciDevCSRInfo.minGnt));
	pci_read_config_byte(dev, PCI_MAX_LAT, &(localPciDevInfo->pciDevCSRInfo.maxLat));

	return OK;
}


/**
 *	Print PCI device configuration space register
 */
static void printPciDevCSRInfo(PCI_DEV_INFO *localPciDevInfo)
{
	printk("******PciDevCSRInfo******\n");
	printk("\t vendorId = 0x%x\n", localPciDevInfo->pciDevCSRInfo.vendorId);
	printk("\t deviceId = 0x%x\n", localPciDevInfo->pciDevCSRInfo.deviceId);
	printk("\t command = 0x%x\n", localPciDevInfo->pciDevCSRInfo.command);
	printk("\t status = 0x%x\n", localPciDevInfo->pciDevCSRInfo.status);
	printk("\t classRevision = 0x%x\n", localPciDevInfo->pciDevCSRInfo.classRevision);
	printk("\t revisionId = 0x%x\n", localPciDevInfo->pciDevCSRInfo.revisionId);
	printk("\t classProg = 0x%x\n", localPciDevInfo->pciDevCSRInfo.classProg);
	printk("\t classDevice = 0x%x\n", localPciDevInfo->pciDevCSRInfo.classDevice);
	printk("\t cacheLineSize = 0x%x\n", localPciDevInfo->pciDevCSRInfo.cacheLineSize);
	printk("\t latencyTimer = 0x%x\n", localPciDevInfo->pciDevCSRInfo.latencyTimer);
	printk("\t headerType = 0x%x\n", localPciDevInfo->pciDevCSRInfo.headerType);
	printk("\t bist = 0x%x\n", localPciDevInfo->pciDevCSRInfo.bist);
	printk("\t baseAddress0 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress0);
	printk("\t baseAddress1 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress1);
	printk("\t baseAddress2 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress2);
	printk("\t baseAddress3 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress3);
	printk("\t baseAddress4 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress4);
	printk("\t baseAddress5 = 0x%x\n", localPciDevInfo->pciDevCSRInfo.baseAddress5);
	printk("\t cardBusCIS = 0x%x\n", localPciDevInfo->pciDevCSRInfo.cardBusCIS);
	printk("\t subsystemVendorId = 0x%x\n", localPciDevInfo->pciDevCSRInfo.subsystemVendorId);
	printk("\t subsystemDeviceId = 0x%x\n", localPciDevInfo->pciDevCSRInfo.subsystemDeviceId);
	printk("\t romAddress = 0x%x\n", localPciDevInfo->pciDevCSRInfo.romAddress);
    printk("\t capabilityList = 0x%x\n", localPciDevInfo->pciDevCSRInfo.capabilityList);
	printk("\t interruptLine = 0x%x\n", localPciDevInfo->pciDevCSRInfo.interruptLine);
	printk("\t interruptPin = 0x%x\n", localPciDevInfo->pciDevCSRInfo.interruptPin);
	printk("\t minGnt = 0x%x\n", localPciDevInfo->pciDevCSRInfo.minGnt);
	printk("\t maxLat = 0x%x\n", localPciDevInfo->pciDevCSRInfo.maxLat);
}


/**
 *	Read the six I/O regions information
 */
static STATUS getPciBarInfo(struct pci_dev *dev, PCI_DEV_INFO *localPciDevInfo)
{
	int i = 0;
	for (i = 0; i < 6; i++)
	{
		localPciDevInfo->pciBarInfoList[i].start = pci_resource_start(dev, i);
		localPciDevInfo->pciBarInfoList[i].end = pci_resource_end(dev, i);
		localPciDevInfo->pciBarInfoList[i].rang = localPciDevInfo->pciBarInfoList[i].end - localPciDevInfo->pciBarInfoList[i].start;
		localPciDevInfo->pciBarInfoList[i].flag = pci_resource_flags(dev, i);
	}

	return OK;
}


/**
 *	Print the six I/O regions information
 */
static void printPciBarInfo(PCI_DEV_INFO *localPciDevInfo)
{
    int i;
    printk("******PciBarInfo******\n");
	for (i = 0; i < 6; i++)
	{
		printk("\t Bar%dInfo\n", i);
		printk("\t\t start = 0x%lx\n", localPciDevInfo->pciBarInfoList[i].start);
		printk("\t\t end = 0x%lx\n", localPciDevInfo->pciBarInfoList[i].end);
		printk("\t\t rang = 0x%lx\n", localPciDevInfo->pciBarInfoList[i].rang);
		printk("\t\t flag = 0x%lx\n", localPciDevInfo->pciBarInfoList[i].flag);

		printk("\t\t flag type = %s\n",(localPciDevInfo->pciBarInfoList[i].flag & IORESOURCE_MEM) ? "mem" : "port");
	}
}


/**
 *	Print PCI device information
 */
static void printPciDevInfo(PCI_DEV_INFO *localPciDevInfo)
{
	printPciDevCSRInfo(localPciDevInfo);
	printPciBarInfo(localPciDevInfo);
}


/**
 *	Map the PCI device I/O region to the CPU virtual memory space
 */
static STATUS mapPciBar(struct pci_dev *dev, PCI_DEV_INFO *localPciDevInfo)
{
	localPciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr = pci_ioremap_bar(dev, BAR_INDEX);
	if(!localPciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr)
    {
        printk("bar %d ioremap error!\n", BAR_INDEX);
        return ERROR;
    }

	return OK;
}


/**
 *	File operation: open()
 */
static int pci_open(struct inode *inode_p, struct file *file_p)
{
	printk("pci_driver_llrf - open was called!\n");  
    return OK;
}


/**
 *	File operation: read()
 */
static ssize_t pci_read(struct file *file_p, char __user *buf, size_t length, loff_t *f_pos)  
{
    int ret;
    printk("pci_driver_llrf - read was called!\n");  
  
    ret = copy_to_user(buf, pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + 0, length);
    if(ret) {
                printk("pci_read - copy_to_user failed!\n"); 
            }

    return length;
}


/**
 *	File operation: write()
 */
static ssize_t pci_write(struct file *file_p, const char __user *buf, size_t length, loff_t *f_pos)  
{    
    int ret;
    printk("pci_driver_llrf - write was called!\n");  
    
    ret = copy_from_user(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + 0, buf, length);
    if(ret) {
                printk("pci_write - copy_from_user failed!\n"); 
            }
      
    return length;  
}


/**
 *	File operation: close()
 */ 
static int pci_close(struct inode *inode_p, struct file *file_p)  
{  
    printk("pci_driver_llrf - close was called!\n");  
    return 0;  
}


/**
 *	File operation: ioctl()
 */ 
static long int pci_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	IO_VALUE _ioValue;
	IO_VALUE * ioValue = &_ioValue;
    IO_WAVEFORM _ioWaveform;
    IO_WAVEFORM * ioWaveform = &_ioWaveform;
    UINT32 offset;
    UINT32 length;
    char *buffer;
    ktime_t start_time, stop_time, elapsed_time;

	switch(cmd) {
		case RD_VALUE_8:
            ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_8 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            ioValue->value_8 = REG_READ8(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset);
            ret = copy_to_user((IO_VALUE *) arg, ioValue, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_8 - copy_to_user failed!\n"); 
                return ERROR;
            }
            //printk("RD_VALUE_8    addr = 0x%08X    data = 0x%02X\n", ioValue->offset, ioValue->value_8); 
			break;

        case WR_VALUE_8:
			ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
			if(ret) {
                printk("pci_ioctl - WR_VALUE_8 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            REG_WRITE8(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset, ioValue->value_8);
            //printk("WR_VALUE_8    addr = 0x%08X    data = 0x%02X\n", ioValue->offset, ioValue->value_8); 
			break;

        case RD_VALUE_16:
            ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_16 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            ioValue->value_16 = REG_READ16(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset);
            ret = copy_to_user((IO_VALUE *) arg, ioValue, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_16 - copy_to_user failed!\n"); 
                return ERROR;
            }
            //printk("RD_VALUE_16    addr = 0x%08X    data = 0x%04X\n", ioValue->offset, ioValue->value_16); 
			break;

        case WR_VALUE_16:
			ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
			if(ret) {
                printk("pci_ioctl - WR_VALUE_16 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            REG_WRITE16(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset, ioValue->value_16);
            //printk("WR_VALUE_16    addr = 0x%08X    data = 0x%04X\n", ioValue->offset, ioValue->value_16); 
			break;

        case RD_VALUE_32:
            ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_32 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            ioValue->value_32 = REG_READ32(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset);
            ret = copy_to_user((IO_VALUE *) arg, ioValue, sizeof(IO_VALUE));
            if(ret) {
                printk("pci_ioctl - RD_VALUE_32 - copy_to_user failed!\n"); 
                return ERROR;
            }
            //printk("RD_VALUE_32    addr = 0x%08X    data = 0x%08X\n", ioValue->offset, ioValue->value_32); 
			break;

        case WR_VALUE_32:
			ret = copy_from_user(ioValue, (IO_VALUE *)arg, sizeof(IO_VALUE));
			if(ret) {
                printk("pci_ioctl - WR_VALUE_32 - copy_from_user failed!\n"); 
                return ERROR;
            }
            offset = ioValue->offset;
            REG_WRITE32(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset, ioValue->value_32);
            //printk("WR_VALUE_32    addr = 0x%08X    data = 0x%08X\n", ioValue->offset, ioValue->value_32); 
			break;

        case RD_WAVEFORM:
            ret = copy_from_user(ioWaveform, (IO_WAVEFORM *)arg, sizeof(IO_WAVEFORM));
            if(ret) {
                printk("pci_ioctl - RD_WAVEFORM - copy_from_user failed!\n");
                return ERROR; 
            }
            offset = ioWaveform->offset;
            length = ioWaveform->length;
            buffer = ioWaveform->buffer;
            
            start_time = ktime_get(); /* Time measurement */
			ret = copy_to_user(buffer, pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + offset, length);
			stop_time = ktime_get(); /* Time measurement */
			elapsed_time = ktime_sub(stop_time, start_time);
            //printk("RD_WAVEFORM - The elapsed time of copy_to_user() is %lld nano seconds\n", ktime_to_ns(elapsed_time));
			
			if(ret) {
                printk("pci_ioctl - RD_WAVEFORM - copy_to_user failed!\n"); 
                return ERROR;
            }
            //printk("RD_WAVEFORM    offset = 0x%08X    length = %d\n", offset, length); 
			break;

        default:
            printk("Unknown ioctl command!\n");
            break;
	}

	return OK;
}


/* File_operations structure declaration */  
static struct file_operations pci_fops = {  
    .owner   = THIS_MODULE,  
    .open    = pci_open,
    .read    = pci_read,  
    .write   = pci_write, 
    .release = pci_close,
    .unlocked_ioctl = pci_ioctl,
};  
  

/**
 *	This probing function gets called during execution of pci_register_driver() for already existing devices
 *  or later if a new device gets inserted
 */ 
static int driver_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int ret = 0;
	/*int t;*/
	printk("pci_driver_llrf - driver_probe was called!\n"); 	

    ret = pci_enable_device(dev);
	if(ret)
	{
		printk("pci_enable_device() error!\n");
		return ret;
	}

    ret = pci_request_regions(dev, DEVICE_NAME);
    if(ret)
    {
        printk("PCI request regions error!\n");
        pci_disable_device(dev);
        return ret;
    }
		
	pciDevInfo = kmalloc(sizeof(PCI_DEV_INFO), GFP_KERNEL);
    if(!pciDevInfo)
    {
        printk("In %s, kmalloc error!", __func__);
        pci_release_regions(dev);
        pci_disable_device(dev);
        return ERROR;
    }

    ret = getPciDevCSRInfo(dev, pciDevInfo);
	if(ret == ERROR)
	{
        kfree(pciDevInfo);
        pci_release_regions(dev);
        pci_disable_device(dev);
		return ERROR;
	}
	
	ret = getPciBarInfo(dev, pciDevInfo);
	if(ret == ERROR)
	{
        kfree(pciDevInfo);
        pci_release_regions(dev);
        pci_disable_device(dev);
		return ERROR;
	}

	ret = mapPciBar(dev, pciDevInfo);
	if(ret == ERROR)
	{
		kfree(pciDevInfo);
        pci_release_regions(dev);
        pci_disable_device(dev);
        return ERROR;
	}

    printPciDevInfo(pciDevInfo);

    /**
     *  On success, register_chrdev returns 0 if major is a number other then 0, 
     *  otherwise Linux will choose a major number and return the chosen value.
     */ 
    ret = register_chrdev(PCI_MAJOR, DEVICE_NAME, &pci_fops);  
    if(ret < 0)
	{
		kfree(pciDevInfo);
        pci_release_regions(dev);
        pci_disable_device(dev);
        return ERROR;
	}
	    
    printk("Probe succeeds!\n");
	
	/**
     *  Test purpose
     */ 
	/*REG_WRITE32(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + 0x0, 0x12345678);
	t = REG_READ32(pciDevInfo->pciBarInfoList[BAR_INDEX].remapAddr + 0x0);
	printk("reg read  + 0x00: %x\n", t);*/

    return OK;
}


/**
 *	The remove() function gets called whenever a device being handled by this driver is removed
 *  (either during deregistration of the driver or when it’s manually pulled out of a hot-pluggable slot)
 */ 
static void driver_remove(struct pci_dev *dev)
{
	pci_release_regions(dev);
	pci_disable_device(dev);
	printk("Device is removed successfully!\n");
}


/**
 *	PCI driver structure
 */ 
static struct pci_driver pci_driver = {
	.name = DEVICE_NAME,
	.id_table = pciIdTb,
	.probe = driver_probe,
	.remove = driver_remove,
};


static int __init pci_llrf_init(void)
{
    printk("pci_driver_llrf - pci_llrf_init was called!\n");
      
    /**
     *  PCI device drivers call pci_register_driver() during their initialization with a pointer to a structure
     *  describing the driver (struct pci_driver) 
     */ 
    return pci_register_driver(&pci_driver);
}


static void __exit pci_llrf_exit(void)
{
    printk("pci_driver_llrf - pci_llrf_exit was called!\n");
	/* Deletes the driver structure from the list of registered PCI drivers */  
	pci_unregister_driver(&pci_driver);
	
    /* Unregister and destroy a cdev */
    unregister_chrdev(PCI_MAJOR, DEVICE_NAME);

	if(pciDevInfo != NULL)
	{
    	kfree(pciDevInfo);
	}
}


module_init(pci_llrf_init);
module_exit(pci_llrf_exit);

