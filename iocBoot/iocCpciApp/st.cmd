#!../../bin/linux-x86_64/cpciApp

## You may have to change cpci to something else
## everywhere it appears in this file

< envPaths

epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","65536")

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/cpciApp.dbd"
cpciApp_registerRecordDeviceDriver pdbbase

cpciLLRFConfigure("cpciLLRF")

## Load record instances
dbLoadRecords "db/cpciLLRF.db", "SYS=FACILITY1_ACC_LRF, SUB=LLRF, PORT=cpciLLRF, ADDR=0, TIMEOUT=1"

## Set this to see messages from mySub
#var mySubDebug 1

## Run this to trace the stages of iocInit
#traceIocInit

asynSetTraceMask("", 0, 0x2)

cd "${TOP}/iocBoot/${IOC}"
iocInit

asynSetTraceMask("cpciLLRF", 0, 0x29)
