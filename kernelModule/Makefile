DIRS += pci_driver_llrf

default:
	-for d in $(DIRS); do (cd $$d; $(MAKE)); done

clean:
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

load:
	-for d in $(DIRS); do (cd $$d; $(MAKE) load ); done
	
unload:
	-for d in $(DIRS); do (cd $$d; $(MAKE) unload ); done