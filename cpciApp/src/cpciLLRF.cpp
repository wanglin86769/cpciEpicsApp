/*
 * cpciLLRF.cpp
 *
 * EPICS driver support written using asynPortDriver in Linux for LLRF cPCI FPGA board.
 *
 * Author: Lin Wang
 *
 * Created November 14, 2022
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>
#include <epicsExport.h>

#include "cpciLLRF.h"

extern "C" {
    #include "cpciAccess.h"
}


/*
 *
 * FPGA register definition
 *
 */
PCI_REG_INFO cpciLLRF::registers[] = {
    //      Register name                      Offset               Data type
    // Read/Write registers
    {       "lock_en",                          0,                  asynParamInt32          },
    {       "on_off", 	                        4,                  asynParamInt32          },
    {       "RF_PW_CW",                         8,                  asynParamInt32          },
    {       "loop_en", 	                        12,                 asynParamInt32          },
    {       "tune_loop_en", 	                16,                 asynParamInt32          },
    {       "sp_amp", 	                        20,                 asynParamInt32          },
    {       "sp_phase", 	                    24,                 asynParamFloat64        },
    {       "beam_amp", 	                    28,                 asynParamInt32          },
    {       "beam_phase",	                    32,                 asynParamFloat64        },
    {       "kp",	                            36,                 asynParamInt32          },
    {       "ki",	                            40,                 asynParamInt32          },
    {       "ad_adj_real",	                    44,                 asynParamFloat64        },
    {       "ad_adj_imag",	                    48,                 asynParamFloat64        },
    {       "da_adj_real", 	                    52,                 asynParamFloat64        },
    {       "da_adj_imag", 	                    56,                 asynParamFloat64        },
    {       "da_adj_real2", 	                60,                 asynParamFloat64        },
    {       "da_adj_imag2", 	                64,                 asynParamFloat64        },
    {       "n_jump",       	                68,                 asynParamInt32          },
    {       "n_jump_delay", 	                72,                 asynParamInt32          },
    {       "start_cnt",       	                76,                 asynParamInt32          },
    {       "end_cnt", 	                        80,                 asynParamInt32          },
    {       "Out_max_I",	                    84,                 asynParamInt32          },
    {       "Out_min_I", 	                    88,                 asynParamInt32          },
    {       "Out_max_Q", 	                    92,                 asynParamInt32          },
    {       "Out_min_Q", 	                    96,                 asynParamInt32          },
    {       "sp_freq",                     	    100,                asynParamInt32          },
    {       "Loop_Delay_count", 	            104,                asynParamInt32          },
    {       "Kip_start_point", 	                108,                asynParamInt32          },
    {       "Kip_end_point", 	                112,                asynParamInt32          },
    {       "Ki_I_rise_point", 	                116,                asynParamInt32          },
    {       "Ki_Q_rise_point", 	                120,                asynParamInt32          },
    {       "freq_kp", 	                        124,                asynParamInt32          },
    {       "beam_delay",                       128,                asynParamInt32          },
    {       "rfgate_ram_jiange", 	            132,                asynParamInt32          },
    {       "Qset",                 	        136,                asynParamInt32          },
    {       "Overdrive",            	        140,                asynParamFloat64        },
    {       "train_mode",            	        144,                asynParamInt32          },
    {       "triger_src",	                    148,                asynParamInt32          },
    {       "triger_period",	                152,                asynParamInt32          },
    {       "RF_count",              	        156,                asynParamInt32          },
    {       "init_set",              	        160,                asynParamInt32          },
    {       "end_set",               	        164,                asynParamInt32          },
    {       "powup_time",            	        168,                asynParamInt32          },
    {       "powup_step",            	        172,                asynParamInt32          },
    {       "powup_hold_time",       	        176,                asynParamInt32          },
    {       "powup_protect_times",   	        180,                asynParamInt32          },
    {       "powdown_time",          	        184,                asynParamInt32          },
    {       "powdown_step",          	        188,                asynParamInt32          },
    {       "powdown_hold_time",             	192,                asynParamInt32          },
    {       "powdown_protect_times", 	        196,                asynParamInt32          },
    {       "PRT_Start_count",            	    200,                asynParamInt32          },
    {       "PRT_End_count",         	        204,                asynParamInt32          },
    {       "SP_P",                  	        208,                asynParamFloat64        },
    {       "SP_P_1",                	        212,                asynParamFloat64        },
    {       "SP_P_2",                	        216,                asynParamFloat64        },
    {       "SP_P_3",                	        220,                asynParamFloat64        },
    {       "Ch_beam_Hold",          	        224,                asynParamInt32          },
    {       "Ch_VSWR_Hold",          	        228,                asynParamFloat64        },
    {       "SP_Ch_sum_PRT_Num",     	        232,                asynParamInt32          },
    {       "manual_Clear",          	        236,                asynParamInt32          },
    {       "VSWR_EN",               	        240,                asynParamInt32          },
    {       "VSWR_Delay_Num",        	        244,                asynParamInt32          },
    {       "k0_forward",            	        248,                asynParamInt32          },
    {       "k0_reverse",            	        252,                asynParamInt32          },
    {       "k1_forward",            	        256,                asynParamInt32          },
    {       "k1_reverse",            	        260,	            asynParamInt32          },
    {       "k2_forward",            	        264,	            asynParamInt32          },
    {       "k2_reverse",            	        268,	            asynParamInt32          },
    {       "k3_forward",           	        272,	            asynParamInt32          },
    {       "k3_reverse",            	        276,	            asynParamInt32          },
    {       "state_wr_reg",	                    508,	            asynParamInt32          },

    // Read-only registers
    {		"rd_reg_state",						508,				asynParamInt32			},
	{		"lock_en_state",					512,				asynParamInt32			},
	{		"on_off_state",						516,				asynParamInt32			},
	{		"loop_en_state",					520,				asynParamInt32			},
	{		"tune_loop_en_state",				524,				asynParamInt32			},
	{		"o_sp_amp_state",					528,				asynParamInt32			},
	{		"sp_amp_state",						532,				asynParamInt32			},
	{		"sp_phase_state",					536,				asynParamFloat64		},
	{		"s1_Ch0_PRT_Num_state",				540,				asynParamInt32			},
	{		"s1_Ch1_PRT_Num_state",				544,				asynParamInt32			},
	{		"s1_Ch2_PRT_Num_state",				548,				asynParamInt32			},
	{		"s1_Ch3_PRT_Num_state",				552,				asynParamInt32			},
	{		"s1_Ch_sum_PRT_Num_state",			556,				asynParamInt32			},
	{		"Ch0_VSWR_SUM_Counter_state",		560,				asynParamInt32			},
	{		"Ch1_VSWR_SUM_Counter_state",		564,				asynParamInt32			},
	{		"Ch2_VSWR_SUM_Counter_state",		568,				asynParamInt32			},
	{		"Ch3_VSWR_SUM_Counter_state",		572,				asynParamInt32			},
	{		"high_Wattcher_state",				576,				asynParamInt32			},
	{		"VSWR_PRT_forvere_out_state",		580,				asynParamInt32			},
	{		"freq_cal_state",					584,				asynParamFloat64		},
};

int cpciLLRF::regCount = sizeof(registers) / sizeof(PCI_REG_INFO);


static const char *driverName="cpciLLRF";
void pollerThreadC(void *drvPvt);


cpciLLRF::cpciLLRF(const char *portName)
   : asynPortDriver(portName,
                    1, /* maxAddr */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynFloat64Mask | asynFloat64ArrayMask,  /* Interrupt mask */
                    0, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0) /* Default stack size*/
{
    asynStatus status;
    const char *functionName = "cpciLLRF";
    int dummy;
    
    fd = openDevice((char *)DEVICE_NAME);
    if(fd == -1) {
        printf("cpciLLRF::cpciLLRF: openDevice() with DEVICE_NAME %s failed\n", DEVICE_NAME);
    } else {
        printf("cpciLLRF::cpciLLRF: openDevice() success - DEVICE_NAME %s - fd %d\n", DEVICE_NAME, fd);
    }
        
    /**** Register parameters ****/
    for(int i=0; i<regCount; i++) {
        createParam(registers[i].name, registers[i].type, &dummy);
    }

    /**** Waveform parameters ****/
    createParam("waveform_CAV2_amp", asynParamFloat64Array, &_waveform_CAV2_amp);
    createParam("waveform_CAV2_phase", asynParamFloat64Array, &_waveform_CAV2_phase);
    createParam("waveform_CAV1_amp", asynParamFloat64Array, &_waveform_CAV1_amp);
    createParam("waveform_CAV1_phase", asynParamFloat64Array, &_waveform_CAV1_phase);

    createParam("waveform_fwd1_amp", asynParamFloat64Array, &_waveform_fwd1_amp);
    createParam("waveform_fwd1_phase", asynParamFloat64Array, &_waveform_fwd1_phase);
    createParam("waveform_fwd1_power", asynParamFloat64Array, &_waveform_fwd1_power);
    createParam("waveform_rfl1_amp", asynParamFloat64Array, &_waveform_rfl1_amp);
    createParam("waveform_rfl1_phase", asynParamFloat64Array, &_waveform_rfl1_phase);
    createParam("waveform_rfl1_power", asynParamFloat64Array, &_waveform_rfl1_power);
    createParam("waveform_CAV_VSWR1", asynParamFloat64Array, &_waveform_CAV_VSWR1);

    createParam("waveform_fwd2_amp", asynParamFloat64Array, &_waveform_fwd2_amp);
    createParam("waveform_fwd2_phase", asynParamFloat64Array, &_waveform_fwd2_phase);
    createParam("waveform_fwd2_power", asynParamFloat64Array, &_waveform_fwd2_power);
    createParam("waveform_rfl2_amp", asynParamFloat64Array, &_waveform_rfl2_amp);
    createParam("waveform_rfl2_phase", asynParamFloat64Array, &_waveform_rfl2_phase);
    createParam("waveform_rfl2_power", asynParamFloat64Array, &_waveform_rfl2_power);
    createParam("waveform_CAV_VSWR2", asynParamFloat64Array, &_waveform_CAV_VSWR2);

    createParam("waveform_CAV_inpower", asynParamFloat64Array, &_waveform_CAV_inpower);
    createParam("waveform_CAV_fwdpower", asynParamFloat64Array, &_waveform_CAV_fwdpower);
    createParam("waveform_CAV_rflpower", asynParamFloat64Array, &_waveform_CAV_rflpower);

    createParam("waveform_DAC_amp", asynParamFloat64Array, &_waveform_DAC_amp);
    createParam("waveform_DAC_phase", asynParamFloat64Array, &_waveform_DAC_phase);

    /**** Waveform single point position ****/
    createParam("waveform_single_point_position", asynParamInt32, &_waveform_single_point_position);

    /**** Waveform single point value ****/
    createParam("waveform_single_point_CAV2_amp", asynParamFloat64, &_waveform_single_point_CAV2_amp);
    createParam("waveform_single_point_CAV2_phase", asynParamFloat64, &_waveform_single_point_CAV2_phase);
    createParam("waveform_single_point_CAV1_amp", asynParamFloat64, &_waveform_single_point_CAV1_amp);
    createParam("waveform_single_point_CAV1_phase", asynParamFloat64, &_waveform_single_point_CAV1_phase);

    createParam("waveform_single_point_fwd1_amp", asynParamFloat64, &_waveform_single_point_fwd1_amp);
    createParam("waveform_single_point_fwd1_phase", asynParamFloat64, &_waveform_single_point_fwd1_phase);
    createParam("waveform_single_point_fwd1_power", asynParamFloat64, &_waveform_single_point_fwd1_power);
    createParam("waveform_single_point_rfl1_amp", asynParamFloat64, &_waveform_single_point_rfl1_amp);
    createParam("waveform_single_point_rfl1_phase", asynParamFloat64, &_waveform_single_point_rfl1_phase);
    createParam("waveform_single_point_rfl1_power", asynParamFloat64, &_waveform_single_point_rfl1_power);
    createParam("waveform_single_point_CAV_VSWR1", asynParamFloat64, &_waveform_single_point_CAV_VSWR1);

    createParam("waveform_single_point_fwd2_amp", asynParamFloat64, &_waveform_single_point_fwd2_amp);
    createParam("waveform_single_point_fwd2_phase", asynParamFloat64, &_waveform_single_point_fwd2_phase);
    createParam("waveform_single_point_fwd2_power", asynParamFloat64, &_waveform_single_point_fwd2_power);
    createParam("waveform_single_point_rfl2_amp", asynParamFloat64, &_waveform_single_point_rfl2_amp);
    createParam("waveform_single_point_rfl2_phase", asynParamFloat64, &_waveform_single_point_rfl2_phase);
    createParam("waveform_single_point_rfl2_power", asynParamFloat64, &_waveform_single_point_rfl2_power);
    createParam("waveform_single_point_CAV_VSWR2", asynParamFloat64, &_waveform_single_point_CAV_VSWR2);

    createParam("waveform_single_point_CAV_inpower", asynParamFloat64, &_waveform_single_point_CAV_inpower);
    createParam("waveform_single_point_CAV_fwdpower", asynParamFloat64, &_waveform_single_point_CAV_fwdpower);
    createParam("waveform_single_point_CAV_rflpower", asynParamFloat64, &_waveform_single_point_CAV_rflpower);

    createParam("waveform_single_point_DAC_amp", asynParamFloat64, &_waveform_single_point_DAC_amp);
    createParam("waveform_single_point_DAC_phase", asynParamFloat64, &_waveform_single_point_DAC_phase);
    
    /**** Local parameter initialization ****/
    setIntegerParam(_waveform_single_point_position, 0);

    /* Create the thread that read the waveforms from hardware in the background */
    status = (asynStatus)(epicsThreadCreate("cpciLLRFTask",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)::pollerThreadC,
                          this) == NULL);
    if (status) {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }
}


void pollerThreadC(void *drvPvt)
{
    cpciLLRF *pPvt = (cpciLLRF *)drvPvt;

    pPvt->pollerThread();
}


void cpciLLRF::pollerThread(void)
{
    epicsUInt32 wfReady = 0;
    const epicsUInt32 offset = 508;
    int status;
    int position;

    /* Loop forever */
    while(1) {
        epicsThreadSleep(POLLING_PERIOD_IN_SECOND);

        #ifdef false
        // Read the wavefrom ready flag
        status = uint32Read(fd, offset, &wfReady);
        if(status != 0) {
            printf("pollerThread(): uint32Read return error");
            return;
        }
        
        printf("**************************\n");
        printf("wfReady 0: %d\n", wfReady);

        if(!wfReady) continue;

        /**** Waveform data in FPGA is ready ****/

        // Clear the wavefrom ready flag
        status = uint32Write(fd, offset, 0);
        if(status != 0) {
            printf("pollerThread(): uint32Write return error");
            return;
        }
        
        epicsUInt32 wfReady1;
        status = uint32Read(fd, offset, &wfReady1);
        if(status != 0) {
            printf("pollerThread(): uint32Read return error");
            return;
        }
        printf("wfReady 1: %d\n", wfReady1);
        printf("**************************\n");
        #endif

        //struct timespec start, end;
	    //double time_spent;
	    //const int BILLION = 1000000000;
        //clock_gettime(CLOCK_REALTIME, &start);
        
        // Read waveform raw data from FPGA
        status = waveformRead(fd, WAVEFORM_OFFSET, WAVEFORM_LENGTH, (char *)waveformBuffer);
        if(status != 0) {
            printf("pollerThread(): waveformRead return error");
            return;
        }   
          
        for(int i = 0; i < WAVEFORM_POINT; i++) {
            waveform_CAV2_amp[i] = 1.0 * waveform_0[i];
            waveform_CAV2_phase[i] = 1.0 * waveform_1[i] / 32768 * 180;
            waveform_CAV1_amp[i] = 1.0 * waveform_2[i];
            waveform_CAV1_phase[i] = 1.0 * waveform_3[i] / 32768 * 180;

            waveform_fwd1_amp[i] = 1.0 * sqrt(pow(waveform_4[i], 2) + pow(waveform_5[i], 2));
            waveform_fwd1_phase[i] = 1.0 * atan2((double)waveform_5[i], (double)waveform_4[i]) * 180 / PI;
            waveform_fwd1_power[i] = 1.0 * (pow(waveform_4[i], 2) + pow(waveform_5[i], 2)) * fwd1_k * pow(10, 1.0 * fwd1_b / 10) / 1000;
            waveform_rfl1_amp[i] = 1.0 * sqrt(pow(waveform_6[i], 2) + pow(waveform_7[i], 2));;
            waveform_rfl1_phase[i] = 1.0 * atan2((double)waveform_7[i], (double)waveform_6[i]) * 180 / PI;
            waveform_rfl1_power[i] = 1.0 * (pow(waveform_6[i], 2) + pow(waveform_7[i], 2)) * rfl1_k * pow(10, 1.0 * rfl1_b / 10) / 1000;
            waveform_CAV_VSWR1[i] = 1.0 * (sqrt(waveform_fwd1_power[i]) + sqrt(waveform_rfl1_power[i])) / (sqrt(waveform_fwd1_power[i]) - sqrt(waveform_rfl1_power[i]));

            waveform_fwd2_amp[i] = 1.0 * sqrt(pow(waveform_8[i], 2) + pow(waveform_9[i], 2));
            waveform_fwd2_phase[i] = 1.0 * atan2((double)waveform_9[i], (double)waveform_8[i]) * 180 / PI;
            waveform_fwd2_power[i] = 1.0 * (pow(waveform_8[i], 2) + pow(waveform_9[i], 2)) * fwd2_k * pow(10, 1.0 * fwd2_b / 10) / 1000;
            waveform_rfl2_amp[i] = 1.0 * sqrt(pow(waveform_10[i], 2) + pow(waveform_11[i], 2));;
            waveform_rfl2_phase[i] = 1.0 * atan2((double)waveform_11[i], (double)waveform_10[i]) * 180 / PI;
            waveform_rfl2_power[i] = 1.0 * (pow(waveform_10[i], 2) + pow(waveform_11[i], 2)) * rfl2_k * pow(10, 1.0 * rfl2_b / 10) / 1000;
            waveform_CAV_VSWR2[i] = 1.0 * (sqrt(waveform_fwd2_power[i]) + sqrt(waveform_rfl2_power[i])) / (sqrt(waveform_fwd2_power[i]) - sqrt(waveform_rfl2_power[i]));

            waveform_CAV_inpower[i] = waveform_fwd1_power[i] + waveform_fwd2_power[i] - waveform_rfl1_power[i] - waveform_rfl2_power[i];
            waveform_CAV_fwdpower[i] = waveform_fwd1_power[i] + waveform_fwd2_power[i];
            waveform_CAV_rflpower[i] = waveform_rfl1_power[i] + waveform_rfl2_power[i];
            
            waveform_DAC_amp[i] = 1.0 * waveform_12[i];;
            waveform_DAC_phase[i] = 1.0 * waveform_13[i] / 32768 * 180;
        }  

        doCallbacksFloat64Array(waveform_CAV2_amp, WAVEFORM_POINT, _waveform_CAV2_amp, 0);
        doCallbacksFloat64Array(waveform_CAV2_phase, WAVEFORM_POINT, _waveform_CAV2_phase, 0);
        doCallbacksFloat64Array(waveform_CAV1_amp, WAVEFORM_POINT, _waveform_CAV1_amp, 0);
        doCallbacksFloat64Array(waveform_CAV1_phase, WAVEFORM_POINT, _waveform_CAV1_phase, 0);

        doCallbacksFloat64Array(waveform_fwd1_amp, WAVEFORM_POINT, _waveform_fwd1_amp, 0);
        doCallbacksFloat64Array(waveform_fwd1_phase, WAVEFORM_POINT, _waveform_fwd1_phase, 0);
        doCallbacksFloat64Array(waveform_fwd1_power, WAVEFORM_POINT, _waveform_fwd1_power, 0);
        doCallbacksFloat64Array(waveform_rfl1_amp, WAVEFORM_POINT, _waveform_rfl1_amp, 0);
        doCallbacksFloat64Array(waveform_rfl1_phase, WAVEFORM_POINT, _waveform_rfl1_phase, 0);
        doCallbacksFloat64Array(waveform_rfl1_power, WAVEFORM_POINT, _waveform_rfl1_power, 0);
        doCallbacksFloat64Array(waveform_CAV_VSWR1, WAVEFORM_POINT, _waveform_CAV_VSWR1, 0);

        doCallbacksFloat64Array(waveform_fwd2_amp, WAVEFORM_POINT, _waveform_fwd2_amp, 0);
        doCallbacksFloat64Array(waveform_fwd2_phase, WAVEFORM_POINT, _waveform_fwd2_phase, 0);
        doCallbacksFloat64Array(waveform_fwd2_power, WAVEFORM_POINT, _waveform_fwd2_power, 0);
        doCallbacksFloat64Array(waveform_rfl2_amp, WAVEFORM_POINT, _waveform_rfl2_amp, 0);
        doCallbacksFloat64Array(waveform_rfl2_phase, WAVEFORM_POINT, _waveform_rfl2_phase, 0);
        doCallbacksFloat64Array(waveform_rfl2_power, WAVEFORM_POINT, _waveform_rfl2_power, 0);
        doCallbacksFloat64Array(waveform_CAV_VSWR2, WAVEFORM_POINT, _waveform_CAV_VSWR2, 0);

        doCallbacksFloat64Array(waveform_CAV_inpower, WAVEFORM_POINT, _waveform_CAV_inpower, 0);
        doCallbacksFloat64Array(waveform_CAV_fwdpower, WAVEFORM_POINT, _waveform_CAV_fwdpower, 0);
        doCallbacksFloat64Array(waveform_CAV_rflpower, WAVEFORM_POINT, _waveform_CAV_rflpower, 0);

        doCallbacksFloat64Array(waveform_DAC_amp, WAVEFORM_POINT, _waveform_DAC_amp, 0);
        doCallbacksFloat64Array(waveform_DAC_phase, WAVEFORM_POINT, _waveform_DAC_phase, 0);

        /**** Get position from parameter library ****/
        getIntegerParam(_waveform_single_point_position, &position);

        if(position >= 0 && position < WAVEFORM_POINT) {
            /**** Set waveform single point value for the specified position ****/
            setDoubleParam(_waveform_single_point_CAV2_amp, waveform_CAV2_amp[position]);
            setDoubleParam(_waveform_single_point_CAV2_phase, waveform_CAV2_phase[position]);
            setDoubleParam(_waveform_single_point_CAV1_amp, waveform_CAV1_amp[position]);
            setDoubleParam(_waveform_single_point_CAV1_phase, waveform_CAV1_phase[position]);

            setDoubleParam(_waveform_single_point_fwd1_amp, waveform_fwd1_amp[position]);
            setDoubleParam(_waveform_single_point_fwd1_phase, waveform_fwd1_phase[position]);
            setDoubleParam(_waveform_single_point_fwd1_power, waveform_fwd1_power[position]);
            setDoubleParam(_waveform_single_point_rfl1_amp, waveform_rfl1_amp[position]);
            setDoubleParam(_waveform_single_point_rfl1_phase, waveform_rfl1_phase[position]);
            setDoubleParam(_waveform_single_point_rfl1_power, waveform_rfl1_power[position]);
            setDoubleParam(_waveform_single_point_CAV_VSWR1, waveform_CAV_VSWR1[position]);

            setDoubleParam(_waveform_single_point_fwd2_amp, waveform_fwd2_amp[position]);
            setDoubleParam(_waveform_single_point_fwd2_phase, waveform_fwd2_phase[position]);
            setDoubleParam(_waveform_single_point_fwd2_power, waveform_fwd2_power[position]);
            setDoubleParam(_waveform_single_point_rfl2_amp, waveform_rfl2_amp[position]);
            setDoubleParam(_waveform_single_point_rfl2_phase, waveform_rfl2_phase[position]);
            setDoubleParam(_waveform_single_point_rfl2_power, waveform_rfl2_power[position]);
            setDoubleParam(_waveform_single_point_CAV_VSWR2, waveform_CAV_VSWR2[position]);

            setDoubleParam(_waveform_single_point_CAV_inpower, waveform_CAV_inpower[position]);
            setDoubleParam(_waveform_single_point_CAV_fwdpower, waveform_CAV_fwdpower[position]);
            setDoubleParam(_waveform_single_point_CAV_rflpower, waveform_CAV_rflpower[position]);

            setDoubleParam(_waveform_single_point_DAC_amp, waveform_DAC_amp[position]);
            setDoubleParam(_waveform_single_point_DAC_phase, waveform_DAC_phase[position]);

            callParamCallbacks();
        }
        //clock_gettime(CLOCK_REALTIME, &end);
        //time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
        //printf("Waveform read and process: the elapsed time is %f nano seconds\n", time_spent);   
    }
}


asynStatus cpciLLRF::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    static const char *functionName = "readInt32";
    int function = pasynUser->reason;
    int status = 0;
    
    const char *paramName;
    epicsUInt32 offset;
    epicsInt32 regData;
    epicsInt32 convertedData;

    /* Waveform single point position */
    if (function == _waveform_single_point_position) {
        return getIntegerParam(function, value);
    }
    
    /* Fetch the parameter name */
    getParamName(function, &paramName);
    
    /* Fetch the register offset from the register name */
    offset = regNameToOffset(paramName);
    if(offset == INVALID_OFFSET) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: function=%d - register %s is not defined",
                  driverName, functionName, function, paramName);
        return asynError;
    }
    
    status = uint32Read(fd, offset, (epicsUInt32 *) &regData);
    
    if(strcmp(paramName, "sp_freq") == 0) {
        convertedData = 1.0 * regData / pow(2, 32) * 480000000 - 180000000;
    } else if (strcmp(paramName, "Qset") == 0) {
        convertedData = 1.0 * regData / 1000;
    } else if (strcmp(paramName, "powup_time") == 0) {
        convertedData = 1.0 * regData * 20 * pow(2, 16) / pow(10, 9);
    } else if (strcmp(paramName, "powdown_time") == 0) {
        convertedData = 1.0 * regData * 20 * pow(2, 16) / pow(10, 9);
    } else if (strcmp(paramName, "powup_hold_time") == 0) {
        convertedData = 1.0 * regData * 20 * pow(2, 16) / pow(10, 9);
    } else if (strcmp(paramName, "powdown_hold_time") == 0) {
        convertedData = 1.0 * regData * 20 * pow(2, 16) / pow(10, 9);
    } else if (strcmp(paramName, "PRT_Start_count") == 0) {
        convertedData = 1.0 * regData * 6.25 / 1000;
    } else if (strcmp(paramName, "PRT_End_count") == 0) {
        convertedData = 1.0 * regData * 6.25 / 1000;
    } else {
        //printf("%s    register others\n", functionName);
        convertedData = regData;
    }
    
    *value = convertedData;
    
    /* Set the parameter in the parameter library. */
    setIntegerParam(function, *value);
    
    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    return (status == 0) ? asynSuccess : asynError;
}


asynStatus cpciLLRF::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    const char* functionName = "writeInt32";
    int function = pasynUser->reason;
    int status = 0;
    
    const char *paramName;
    epicsUInt32 offset;
    epicsInt32 regData;
    epicsInt32 convertedData;

    /* Waveform single point position */
    if(function == _waveform_single_point_position) {
        return setIntegerParam(function, value);
    }
    
    /* Fetch the parameter name */
    getParamName(function, &paramName);
    
    /* Set the parameter in the parameter library. */
    setIntegerParam(function, value);

    /* Fetch the register offset from the register name */
    offset = regNameToOffset(paramName);
    if(offset == INVALID_OFFSET) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: function=%d - register %s is not defined",
                  driverName, functionName, function, paramName);
        return asynError;
    }

    if(strcmp(paramName, "sp_freq") == 0) {
        convertedData = 1.0 * (180000000 + value) / 480000000 * pow(2, 32);
    } else if (strcmp(paramName, "Qset") == 0) {
        convertedData = value * 1000;
    } else if (strcmp(paramName, "powup_time") == 0) {
        convertedData = 1.0 * value * pow(10, 9) / (20 * pow(2, 16));
    } else if (strcmp(paramName, "powdown_time") == 0) {
        convertedData = 1.0 * value * pow(10, 9) / (20 * pow(2, 16));
    } else if (strcmp(paramName, "powup_hold_time") == 0) {
        convertedData = 1.0 * value * pow(10, 9) / (20 * pow(2, 16));
    } else if (strcmp(paramName, "powdown_hold_time") == 0) {
        convertedData = 1.0 * value * pow(10, 9) / (20 * pow(2, 16));
    } else if (strcmp(paramName, "PRT_Start_count") == 0) {
        convertedData = 1.0 * value * 1000 / 6.25;
    } else if (strcmp(paramName, "PRT_End_count") == 0) {
        convertedData = 1.0 * value * 1000 / 6.25;
    } else if (strcmp(paramName, "manual_Clear") == 0) {
    	convertedData = value;
        struct timespec start, end;
	    double time_spent;
        // Manually clear the permanent block
        if(value == 1) {
            setIntegerParam(function, 1);
            status = uint32Write(fd, offset, 1);
            if(status != 0) return asynError;

			const int BILLION = 1000000000;
            clock_gettime(CLOCK_REALTIME, &start);
            epicsThreadSleep(0.5); // Delay 500 ms
            clock_gettime(CLOCK_REALTIME, &end);
            time_spent = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
            printf("cpciLLRF::writeInt32: the sleep time is %f nano seconds\n", time_spent);

            setIntegerParam(function, 0);
            status = uint32Write(fd, offset, 0);
            if(status != 0) return asynError;

            callParamCallbacks();
            asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                  "%s:%s: status=%d, function=%d, name=%s, offset=%d, value=%d\n",
                  driverName, functionName, status, function, paramName, offset, value);
            return asynSuccess;
        }
    } else {
        convertedData = value;
    }
    
    regData = convertedData;
    status = uint32Write(fd, offset, (epicsUInt32) regData);

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    if(status == 0) {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                  "%s:%s: status=%d, function=%d, name=%s, offset=%d, value=%d\n",
                  driverName, functionName, status, function, paramName, offset, value);
    }
    else {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: status=%d, function=%d, name=%s, offset=%d, value=%d\n",
                  driverName, functionName, status, function, paramName, offset, value);
    }
        
    return (status == 0) ? asynSuccess : asynError;
}


asynStatus cpciLLRF::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
    static const char *functionName = "readFloat64";
    int function = pasynUser->reason;
    int status = 0;
    
    const char *paramName;
    epicsUInt32 offset;
    epicsInt32 regData;
    epicsFloat64 convertedData;
    
    /* Fetch the parameter name */
    getParamName(function, &paramName);
    
    /* Fetch the register offset from the register name */
    offset = regNameToOffset(paramName);
    if(offset == INVALID_OFFSET) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: function=%d - register %s is not defined",
                  driverName, functionName, function, paramName);
        return asynError;
    }
    
    status = uint32Read(fd, offset, (epicsUInt32 *) &regData);
    
    if(strcmp(paramName, "sp_phase") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "beam_phase") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "ad_adj_real") == 0) {
        convertedData = 1.0 * regData / 4096;
    } else if (strcmp(paramName, "ad_adj_imag") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "da_adj_real") == 0) {
        convertedData = 1.0 * regData / 4096;
    } else if (strcmp(paramName, "da_adj_imag") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "da_adj_real2") == 0) {
        convertedData = 1.0 * regData / 4096;
    } else if (strcmp(paramName, "da_adj_imag2") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "Overdrive") == 0) {
        convertedData = 1.0 * regData / 100000;
    } else if (strcmp(paramName, "SP_P") == 0) {
    	double temp;
    	temp = sqrt(1.0 * regData / 1024 * fwd1_k / rfl1_k);
        convertedData = (temp + 1) / (temp - 1);
    } else if (strcmp(paramName, "SP_P_1") == 0) {
        double temp;
    	temp = sqrt(1.0 * regData / 1024 * fwd2_k / rfl2_k);
        convertedData = (temp + 1) / (temp - 1);
    } else if (strcmp(paramName, "SP_P_2") == 0) {
        double temp;
    	temp = sqrt(1.0 * regData / 1024 * fwd3_k / rfl3_k);
        convertedData = (temp + 1) / (temp - 1);
    } else if (strcmp(paramName, "SP_P_3") == 0) {
        double temp;
    	temp = sqrt(1.0 * regData / 1024 * fwd4_k / rfl4_k);
        convertedData = (temp + 1) / (temp - 1);
    } else if (strcmp(paramName, "Ch_VSWR_Hold") == 0) {
        convertedData = 1.0 * regData * 8192 * (fwd1_k * pow(10, 1.0 * fwd1_b / 10));
    } else if (strcmp(paramName, "sp_phase_state") == 0) {
        convertedData = 1.0 * regData / 32768 * 180;
    } else if (strcmp(paramName, "freq_cal_state") == 0) {
        convertedData = 1.0 * regData * 480 / pow(2, 32);
    } else {
        convertedData = regData;
    }

    *value = convertedData;

    /* Set the parameter in the parameter library. */
    setDoubleParam(function, *value);
    
    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    return (status == 0) ? asynSuccess : asynError;
}


asynStatus cpciLLRF::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    const char* functionName = "writeFloat64";
    int function = pasynUser->reason;
    int status = 0;
    
    const char *paramName;
    epicsUInt32 offset;
    epicsInt32 regData;
    epicsInt32 convertedData;
    
    /* Fetch the parameter name */
    getParamName(function, &paramName);

    /* Set the parameter in the parameter library. */
    setDoubleParam(function, value);
    
    /* Fetch the register offset from the register name */
    offset = regNameToOffset(paramName);
    if(offset == INVALID_OFFSET) {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: function=%d - register %s is not defined",
                  driverName, functionName, function, paramName);
        return asynError;
    }
    
    if(strcmp(paramName, "sp_phase") == 0) {
        convertedData = 1.0 * value / 180 * 32768;
    } else if (strcmp(paramName, "beam_phase") == 0) {
        convertedData = 1.0 * value / 180 * 32768;
    } else if (strcmp(paramName, "ad_adj_real") == 0) {
        convertedData = 1.0 * value * 4096;
    } else if (strcmp(paramName, "ad_adj_imag") == 0) {
        convertedData = 1.0 * value / 180 * 32768;
    } else if (strcmp(paramName, "da_adj_real") == 0) {
        convertedData = 1.0 * value * 4096;
    } else if (strcmp(paramName, "da_adj_imag") == 0) {
        convertedData = 1.0 * value / 180 * 32768;
    } else if (strcmp(paramName, "da_adj_real2") == 0) {
        convertedData = 1.0 * value * 4096;
    } else if (strcmp(paramName, "da_adj_imag2") == 0) {
        convertedData = 1.0 * value / 180 * 32768;
    } else if (strcmp(paramName, "Overdrive") == 0) {
        convertedData = 1.0 * value * 100000;
    } else if (strcmp(paramName, "SP_P") == 0) {
        convertedData = 1.0 * pow(1.0 * (value + 1) / (value - 1), 2) * rfl1_k / fwd1_k * 1024;
    } else if (strcmp(paramName, "SP_P_1") == 0) {
        convertedData = 1.0 * pow(1.0 * (value + 1) / (value - 1), 2) * rfl2_k / fwd2_k * 1024;
    } else if (strcmp(paramName, "SP_P_2") == 0) {
        convertedData = 1.0 * pow(1.0 * (value + 1) / (value - 1), 2) * rfl3_k / fwd3_k * 1024;
    } else if (strcmp(paramName, "SP_P_3") == 0) {
        convertedData = 1.0 * pow(1.0 * (value + 1) / (value - 1), 2) * rfl4_k / fwd4_k * 1024;
    } else if (strcmp(paramName, "Ch_VSWR_Hold") == 0) {
        convertedData = 1.0 * value / (fwd1_k * pow(10, 1.0 * fwd1_b / 10)) / 8192;
    } else {
        convertedData = value;
    }

    regData = convertedData;
    status = uint32Write(fd, offset, (epicsUInt32) regData);

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    if(status == 0) {
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
                  "%s:%s: status=%d, function=%d, name=%s, offset=%d, value=%f\n",
                  driverName, functionName, status, function, paramName, offset, value);
    }
    else {
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
                  "%s:%s: status=%d, function=%d, name=%s, offset=%d, value=%f\n",
                  driverName, functionName, status, function, paramName, offset, value);
    }
        
    return (status == 0) ? asynSuccess : asynError;
}


epicsUInt32 cpciLLRF::regNameToOffset(const char *name)
{
    epicsUInt32 offset = INVALID_OFFSET;
    for(int i=0; i<regCount; i++) {
        if(strcmp(name, registers[i].name) == 0) {
            offset = registers[i].offset;
            break;
        }
    }
    return offset;
}


/* Configuration routine.  Called directly, or from the iocsh function below */

extern "C" {

/** EPICS iocsh callable function to call constructor for the testAsynPortDriver class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxPoints The maximum  number of points in the volt and time arrays */
int cpciLLRFConfigure(const char *portName)
{
    new cpciLLRF(portName);
    return(asynSuccess);
}


/* EPICS iocsh shell commands */

static const iocshArg initArg0 = { "portName", iocshArgString};
static const iocshArg * const initArgs[] = { &initArg0 };
static const iocshFuncDef initFuncDef = {"cpciLLRFConfigure", 1, initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    cpciLLRFConfigure(args[0].sval);
}

void cpciLLRFRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(cpciLLRFRegister);

}

