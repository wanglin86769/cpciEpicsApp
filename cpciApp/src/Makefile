TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS BELOW HERE

DBD += cpciApp.dbd
cpciApp_DBD += base.dbd
cpciApp_DBD += asyn.dbd
cpciApp_DBD += cpciLLRF.dbd

# Build the IOC application
PROD_IOC = cpciApp

# cpciApp_registerRecordDeviceDriver.cpp derives from cpciApp.dbd
cpciApp_SRCS += cpciApp_registerRecordDeviceDriver.cpp
cpciApp_LIBS += asyn

cpciApp_SRCS += cpciAccess.c
cpciApp_SRCS += cpciLLRF.cpp

# Build the main IOC entry point where needed
cpciApp_SRCS_DEFAULT += cpciMain.cpp

# Finally link IOC to the EPICS Base libraries
cpciApp_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD EXTRA GNUMAKE RULES BELOW HERE

