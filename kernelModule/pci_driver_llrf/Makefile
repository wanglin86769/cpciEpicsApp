ifneq ($(KERNELRELEASE),)

obj-m := pci_driver_llrf.o

else

KERNELDIR:=/lib/modules/$(shell uname -r)/build

PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

load:
	insmod ./pci_driver_llrf.ko
	mknod /dev/pci_llrf c 193 0
	chmod 666 /dev/pci_llrf
	
unload:
	rmmod pci_driver_llrf
	rm /dev/pci_llrf

endif
