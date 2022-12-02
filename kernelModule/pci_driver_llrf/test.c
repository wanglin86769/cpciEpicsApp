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

#include "pci_driver_llrf.h"

/* Commands index */
#define CMD_HELP                    0
#define CMD_MULTI_REG_READ          1
#define CMD_MULTI_REG_WRITE         2
#define CMD_REG_READ                3
#define CMD_REG_WRITE               4
#define CMD_UINT8_REG_READ          5
#define CMD_UINT8_REG_WRITE         6
#define CMD_UINT16_REG_READ         7
#define CMD_UINT16_REG_WRITE        8
#define CMD_UINT32_REG_READ         9
#define CMD_UINT32_REG_WRITE        10
#define CMD_WAVEFORM_READ           11
#define CMD_EXIT                    12
#define CMD_NUM                     13

/* Device name */
#define DEVICE_NAME "/dev/pci_llrf"

/* Conversion between second and nano second */
#define BILLION  1000000000.0

/* List valid commands */
const char * cmdList[] = { "help", "mread", "mwrite", "read", "write", \
                           "read8", "write8", "read16", "write16", \
                           "read32", "write32", "wfread", "exit"};

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

int main(int argc, char* argv[])
{
    int fd;
    int i;
    char strInput[100];
	char *cmdString, *arg1, *arg2;
	int cmdIndex;
	char* stopstring;

	UINT32 addr, data, count;

    UINT8 retData8;
	UINT16 retData16;
	UINT32 retData32;
	UINT32 retMultipleData[10000];

    FILE * fdAllWaveforms;
	short waveformsBuffer[WAVEFORM_NUMBER*WAVEFORM_POINT]; /* 16 bits for each waveform point */

	UINT32 testData[9] = {0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666, 0x77777777, 0x88888888, 0x99999999};
	
	struct timespec start, end;
	double time_spent;
    
    printf("#####  Welcome to the shell utility for hardware registers access  #####\n");
    printf("#####     Please press \"help\" to list the supported commands     #####\n");
    
    if((fd = open(DEVICE_NAME, O_RDWR)) == ERROR)
    {
    	printf("Failed to open the device file!\n");
    	return ERROR;
    } 
    else
    {
    	printf("Succeed to obtain the file descriptor!\n");
    }
  
    printf("->");
    fflush(stdout);
  
    /* Infinite loop for shell command parsing */
    while(fgets(strInput, sizeof(strInput), stdin) !=0)
    {
		/* Replace the ENTER with \0 */
		for(i=0; i<sizeof(strInput); i++) {
			if(strInput[i] == 10) {
				strInput[i] = 0;
			}
		}

    	if(strInput[0] == 0)
	    {
    		printf("->");
		    fflush(stdout);
		    continue;
	    }
	 
	    /* Split command and parameters */
		strtok(strInput," ");
		cmdString = strInput;
		
		/* Find the corresponding command */
		for(int i=0; i<CMD_NUM; i++)
		{
			if(!strcmp(cmdList[i], cmdString))
		    {
				cmdIndex = i;
			    break;
		    }
			else if(i == CMD_NUM - 1)
			{
				cmdIndex = -1;
			}
		}
		
		switch (cmdIndex)
		{
			case CMD_HELP:
				printf("Valid commands:\n");
				printf("help                            - show valid commands\n");
				printf("mread    addr count             - read multiple registers of FPGA\n");
				printf("mwrite                          - write multiple registers of FPGA\n");
				printf("read     addr                   - read uint32 registers of FPGA\n");
				printf("write    addr data              - write uint32 registers of FPGA\n");
				printf("read8    addr                   - read uint8 registers of FPGA\n");
				printf("write8   addr data              - write uint8 registers of FPGA\n");
				printf("read16   addr                   - read uint16 registers of FPGA\n");
				printf("write16  addr data              - write uint16 registers of FPGA\n");
				printf("read32   addr                   - read uint32 registers of FPGA\n");
				printf("write32  addr data              - write uint32 registers of FPGA\n");
				printf("wfread                          - read all the waveforms from FPGA\n");
				printf("exit                            - quit");
				break;

			case CMD_MULTI_REG_READ:
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
				if(arg1 == NULL || arg2 == NULL)
				{
					printf("Command syntax error!");
					break;
				}
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				count = (UINT32)strtoul(arg2, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				for(i=0; i<count; i++) {
					uint32Read(fd, addr + i * 4, &retMultipleData[i]);
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_MULTI_REG_READ: the elapsed time is %f nano seconds\n", time_spent);
				
				printf("\n");
				printf("FPGA multiple registers read:\n");
				for(i=0; i<count; i++) {
					printf("addr: 0x%08X", addr + i * 4);
					printf("\t");
					printf("data: 0x%08X", retMultipleData[i]);
					printf("\n");
				}
				break;
				
			case CMD_MULTI_REG_WRITE:
				/*for(i=0; i<128; i++) {
					uint32Write(fd, i * 4, testData[i % 9]);
				}*/
				
				clock_gettime(CLOCK_REALTIME, &start);
				
				/* Register initialization 
				uint32Write(fd, 4, 1);
				uint32Write(fd, 20, 8000);
				uint32Write(fd, 24, 90.0 / 180 * 32768);
				uint32Write(fd, 44, 4096);
				uint32Write(fd, 52, 4096);
				uint32Write(fd, 60, 4096);
				uint32Write(fd, 68, 49);
				uint32Write(fd, 76, 300);
				uint32Write(fd, 80, 600);
				uint32Write(fd, 84, 20000);
				uint32Write(fd, 100, 180.0 / 480 * 4294967296);
				uint32Write(fd, 132, 7);
				uint32Write(fd, 136, 6000 * 1000);
				uint32Write(fd, 140, 0.006 * 100000);
				uint32Write(fd, 144, 2);
				uint32Write(fd, 148, 1);*/
				
				uint32Write(fd, 4, 1);     //on_off 
				uint32Write(fd, 12, 0);     //loop_en
				uint32Write(fd, 20, 8000);    //sp_amp 
				uint32Write(fd, 24, 0.0 / 180 * 32768);  //sp_phase
				uint32Write(fd, 28, 500);         //beam_amp
				uint32Write(fd, 32, 0.0 / 180 * 32768);  //beam_phase
				uint32Write(fd, 36, 10);         //kp
				uint32Write(fd, 40, 5);          //ki
				uint32Write(fd, 44, 4096);    //ad_adj_real
				uint32Write(fd, 52, 4096);     //da_adj_real
				uint32Write(fd, 60, 4096);      //da2_adj_real
				uint32Write(fd, 68, 49);       //n_jump
				uint32Write(fd, 76, 300);         //start_cnt
				uint32Write(fd, 80, 600);      //end_cnt
				uint32Write(fd, 84, 20000);    //out_max_amp
				uint32Write(fd, 100, 180.0 / 480 * 4294967296);
				uint32Write(fd, 104, 10000);   //loop_delay_count
				uint32Write(fd, 124, 2000);     //freq_kp
				uint32Write(fd, 132, 7);               //rfgate_ram_jiange
				uint32Write(fd, 136, 6000 * 1000);   //Qset
				uint32Write(fd, 140, 0.006 * 100000);   //Overdrive
				uint32Write(fd, 144, 2);    //train_mode
				uint32Write(fd, 148, 0);   //triger_src
				uint32Write(fd, 156, 11184810);  //RF_count 
				
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_MULTI_REG_WRITE: the elapsed time is %f nano seconds\n", time_spent);
                
				break;

			case CMD_REG_READ:
				arg1 = strtok(NULL, " ");
				if(arg1 == NULL)
				{
					printf("Command syntax error!");
					break;
				}	
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint32Read(fd, addr, &retData32) != OK)
				{
					printf("\n");
					printf("main(): uint32Read() returns ERROR!\n");
					break;
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_REG_READ: the elapsed time is %f nano seconds\n", time_spent);
				
				printf("\n");
				printf("FPGA uint32 register read:\n");
				printf("addr = 0x%08X, data = 0x%08X\n", addr, retData32);
				break;
			
			case CMD_REG_WRITE:
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
				if(arg1 == NULL || arg2 == NULL)
				{
					printf("Command syntax error!");
					break;
				}
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				data = (UINT32)strtoul(arg2, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint32Write(fd, addr, data) != OK)
				{
					printf("\n");
					printf("main(): uint32Write() returns ERROR!\n");
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_REG_WRITE: the elapsed time is %f nano seconds\n", time_spent);
				
				printf("\n");
				printf("FPGA uint32 register write:\n");
				printf("addr = 0x%08X, data = 0x%08X\n", addr, data);
				break;

			case CMD_UINT8_REG_READ:
				arg1 = strtok(NULL, " ");
				if(arg1 == NULL)
				{
					printf("Command syntax error!");
					break;
				}	
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint8Read(fd, addr, &retData8) != OK)
				{
					printf("\n");
					printf("main(): uint8Read() returns ERROR!\n");
					break;
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT8_REG_READ: the elapsed time is %f nano seconds\n", time_spent);
                
				printf("\n");
				printf("FPGA uint8 register read:\n");
				printf("addr = 0x%08X, data = 0x%02X\n", addr, retData8);
				break;
			
			case CMD_UINT8_REG_WRITE:
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
				if(arg1 == NULL || arg2 == NULL)
				{
					printf("Command syntax error!");
					break;
				}
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
		        data = (UINT32)strtoul(arg2, &stopstring, 0);
		        
		        clock_gettime(CLOCK_REALTIME, &start);
				if(uint8Write(fd, addr, data) != OK)
				{
					printf("\n");
					printf("main(): uint8Write() returns ERROR!\n");
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT8_REG_WRITE: the elapsed time is %f nano seconds\n", time_spent);
                
                printf("\n");
				printf("FPGA uint8 register write:\n");
				printf("addr = 0x%08X, data = 0x%02X\n", addr, data);
				break;
			
			case CMD_UINT16_REG_READ:
				arg1 = strtok(NULL, " ");
				if(arg1 == NULL)
				{
					printf("Command syntax error!");
					break;
				}	
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint16Read(fd, addr, &retData16) != OK)
				{
					printf("\n");
					printf("main(): uint16Read() returns ERROR!\n");
					break;
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT16_REG_READ: the elapsed time is %f nano seconds\n", time_spent);
                
				printf("\n");
				printf("FPGA uint16 register read:\n");
				printf("addr = 0x%08X, data = 0x%04X\n", addr, retData16);
				break;
			
			case CMD_UINT16_REG_WRITE:
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
				if(arg1 == NULL || arg2 == NULL)
				{
					printf("Command syntax error!");
					break;
				}
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
		        data = (UINT32)strtoul(arg2, &stopstring, 0);
		        
		        clock_gettime(CLOCK_REALTIME, &start);
				if(uint16Write(fd, addr, data) != OK)
				{
					printf("\n");
					printf("main(): uint16Write() returns ERROR!\n");
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT16_REG_WRITE: the elapsed time is %f nano seconds\n", time_spent);
                
                printf("\n");
				printf("FPGA uint16 register write:\n");
				printf("addr = 0x%08X, data = 0x%04X\n", addr, data);
				break;
			
			case CMD_UINT32_REG_READ:
				arg1 = strtok(NULL, " ");
				if(arg1 == NULL)
				{
					printf("Command syntax error!");
					break;
				}	
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint32Read(fd, addr, &retData32) != OK)
				{
					printf("\n");
					printf("main(): uint32Read() returns ERROR!\n");
					break;
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT32_REG_READ: the elapsed time is %f nano seconds\n", time_spent);
                
				printf("\n");
				printf("FPGA uint32 register read:\n");
				printf("addr = 0x%08X, data = 0x%08X\n", addr, retData32);
				break;
			
			case CMD_UINT32_REG_WRITE:
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
				if(arg1 == NULL || arg2 == NULL)
				{
					printf("Command syntax error!");
					break;
				}
				addr = (UINT32)strtoul(arg1, &stopstring, 0);
				data = (UINT32)strtoul(arg2, &stopstring, 0);
				
				clock_gettime(CLOCK_REALTIME, &start);
				if(uint32Write(fd, addr, data) != OK)
				{
					printf("\n");
					printf("main(): uint32Write() returns ERROR!\n");
				}
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_UINT32_REG_WRITE: the elapsed time is %f nano seconds\n", time_spent);
                
                printf("\n");
				printf("FPGA uint32 register write:\n");
				printf("addr = 0x%08X, data = 0x%08X\n", addr, data);
				break;

			case CMD_WAVEFORM_READ:
			    clock_gettime(CLOCK_REALTIME, &start);
				waveformRead(fd, WAVEFORM_OFFSET, WAVEFORM_LENGTH, (char *)waveformsBuffer);
				clock_gettime(CLOCK_REALTIME, &end);
				time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
                printf("CMD_WAVEFORM_READ: the elapsed time is %f nano seconds\n", time_spent);
				
				fdAllWaveforms = fopen("./waveformData.txt","w");
	            for(int j=0; j<WAVEFORM_POINT; j++)
	            {
	                fprintf(fdAllWaveforms,"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\n", waveformsBuffer[j], waveformsBuffer[1*4096+j], waveformsBuffer[2*4096+j], waveformsBuffer[3*4096+j], waveformsBuffer[4*4096+j], waveformsBuffer[5*4096+j], waveformsBuffer[6*4096+j], waveformsBuffer[7*4096+j], waveformsBuffer[8*4096+j], waveformsBuffer[9*4096+j], waveformsBuffer[10*4096+j], waveformsBuffer[11*4096+j], waveformsBuffer[12*4096+j], waveformsBuffer[13*4096+j]);
	            }
	            fclose(fdAllWaveforms);
				break;
				
			case CMD_EXIT:
				printf("Exiting...\n");
				close(fd);
				return OK;
				break;
			
			default:
				printf("Invalid command!\n");
				break;
		}
		
		printf("\n->");
		fflush(stdout);
   }

    close(fd);
    return OK;
}
