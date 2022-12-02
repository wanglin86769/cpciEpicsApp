/*
 * cpciLLRF.h
 *
 * EPICS driver support written using asynPortDriver in Linux for LLRF cPCI FPGA board.
 *
 * Author: Lin Wang
 *
 * Created November 14, 2022
 */

#include "asynPortDriver.h"


#define DEVICE_NAME "/dev/pci_llrf"
#define INVALID_OFFSET 0xFFFFFFFF

/* Addresses for 14 waveforms are consecutive */
#define WAVEFORM_OFFSET 0x00040000 /* Waveform offset in FPGA */
#define WAVEFORM_NUMBER 14 /* 14 waveforms in total */
#define WAVEFORM_POINT 4096 /* 4096 points for each waveform */
#define WAVEFORM_DATA_BYTE 2 /* 16 bits for each point */
#define WAVEFORM_LENGTH    WAVEFORM_NUMBER*WAVEFORM_POINT*WAVEFORM_DATA_BYTE

#define POLLING_PERIOD_IN_SECOND 1.0

#define PI 3.14159


typedef struct _PCI_REG_INFO
{
    char name[50];
    epicsUInt32 offset;
    asynParamType type;
} PCI_REG_INFO;


class cpciLLRF: public asynPortDriver {
public:
    cpciLLRF(const char *portName);

    virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    
    virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    
    epicsUInt32 regNameToOffset(const char *name);

    void pollerThread(void);

protected:
    static PCI_REG_INFO registers[];
    static int regCount;
    int fd;

    /**** Waveform buffer ****/
    short waveformBuffer[WAVEFORM_NUMBER*WAVEFORM_POINT]; /* 16 bits for each waveform point */

    /**** Waveform raw data from FPGA ****/
    const short *waveform_0 = waveformBuffer;  // Pickup 2 amplitude
    const short *waveform_1 = waveformBuffer + WAVEFORM_POINT * 1;  // Pickup 2 phase
    const short *waveform_2 = waveformBuffer + WAVEFORM_POINT * 2;  // Pickup 1 amplitude
    const short *waveform_3 = waveformBuffer + WAVEFORM_POINT * 3;  // Pickup 1 phase
    const short *waveform_4 = waveformBuffer + WAVEFORM_POINT * 4;  // Channel 1 forward I
    const short *waveform_5 = waveformBuffer + WAVEFORM_POINT * 5;  // Channel 1 forward Q
    const short *waveform_6 = waveformBuffer + WAVEFORM_POINT * 6;  // Channel 1 reverse I
    const short *waveform_7 = waveformBuffer + WAVEFORM_POINT * 7;  // Channel 1 reverse Q
    const short *waveform_8 = waveformBuffer + WAVEFORM_POINT * 8;  // Channel 2 forward I
    const short *waveform_9 = waveformBuffer + WAVEFORM_POINT * 9;  // Channel 2 forward Q
    const short *waveform_10 = waveformBuffer + WAVEFORM_POINT * 10;  // Channel 2 reverse I
    const short *waveform_11 = waveformBuffer + WAVEFORM_POINT * 11;  // Channel 2 reverse Q
    const short *waveform_12 = waveformBuffer + WAVEFORM_POINT * 12;  // DAC amplitude
    const short *waveform_13 = waveformBuffer + WAVEFORM_POINT * 13;  // DAC phase

    /**** EPICS waveform data ****/
    epicsFloat64 waveform_CAV2_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV2_phase[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV1_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV1_phase[WAVEFORM_POINT];

    epicsFloat64 waveform_fwd1_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_fwd1_phase[WAVEFORM_POINT];
    epicsFloat64 waveform_fwd1_power[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl1_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl1_phase[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl1_power[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV_VSWR1[WAVEFORM_POINT];

    epicsFloat64 waveform_fwd2_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_fwd2_phase[WAVEFORM_POINT];
    epicsFloat64 waveform_fwd2_power[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl2_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl2_phase[WAVEFORM_POINT];
    epicsFloat64 waveform_rfl2_power[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV_VSWR2[WAVEFORM_POINT];

    epicsFloat64 waveform_CAV_inpower[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV_fwdpower[WAVEFORM_POINT];
    epicsFloat64 waveform_CAV_rflpower[WAVEFORM_POINT];

    epicsFloat64 waveform_DAC_amp[WAVEFORM_POINT];
    epicsFloat64 waveform_DAC_phase[WAVEFORM_POINT];

    /**** asynPortDriver parameters for waveform ****/
    int _waveform_CAV2_amp;
    int _waveform_CAV2_phase;
    int _waveform_CAV1_amp;
    int _waveform_CAV1_phase;

    int _waveform_fwd1_amp;
    int _waveform_fwd1_phase;
    int _waveform_fwd1_power;
    int _waveform_rfl1_amp;
    int _waveform_rfl1_phase;
    int _waveform_rfl1_power;
    int _waveform_CAV_VSWR1;

    int _waveform_fwd2_amp;
    int _waveform_fwd2_phase;
    int _waveform_fwd2_power;
    int _waveform_rfl2_amp;
    int _waveform_rfl2_phase;
    int _waveform_rfl2_power;
    int _waveform_CAV_VSWR2;

    int _waveform_CAV_inpower;
    int _waveform_CAV_fwdpower;
    int _waveform_CAV_rflpower;

    int _waveform_DAC_amp;
    int _waveform_DAC_phase;

    /**** asynPortDriver parameters for waveform single point position ****/
    int _waveform_single_point_position;

    /**** asynPortDriver parameters for waveform single point value ****/
    int _waveform_single_point_CAV2_amp;
    int _waveform_single_point_CAV2_phase;
    int _waveform_single_point_CAV1_amp;
    int _waveform_single_point_CAV1_phase;

    int _waveform_single_point_fwd1_amp;
    int _waveform_single_point_fwd1_phase;
    int _waveform_single_point_fwd1_power;
    int _waveform_single_point_rfl1_amp;
    int _waveform_single_point_rfl1_phase;
    int _waveform_single_point_rfl1_power;
    int _waveform_single_point_CAV_VSWR1;

    int _waveform_single_point_fwd2_amp;
    int _waveform_single_point_fwd2_phase;
    int _waveform_single_point_fwd2_power;
    int _waveform_single_point_rfl2_amp;
    int _waveform_single_point_rfl2_phase;
    int _waveform_single_point_rfl2_power;
    int _waveform_single_point_CAV_VSWR2;

    int _waveform_single_point_CAV_inpower;
    int _waveform_single_point_CAV_fwdpower;
    int _waveform_single_point_CAV_rflpower;

    int _waveform_single_point_DAC_amp;
    int _waveform_single_point_DAC_phase;

private:
    const double rfl1_k = 0.00000000016526;
    const double rfl2_k = 0.00000000015767;
    const double rfl3_k = 0.00000000015818;
    const double rfl4_k = 0.00000000015818;
    
    const double fwd1_k = 0.00000000015818;
    const double fwd2_k = 0.00000000015652;
    const double fwd3_k = 0.00000000015818;
    const double fwd4_k = 0.00000000015818;

	const double rfl1_b = 74.1;
	const double rfl2_b = 74.1;
	const double rfl3_b = 74.1;
	const double rfl4_b = 74.1;
	
    const double fwd1_b = 74.1;
    const double fwd2_b = 74.1;
    const double fwd3_b = 74.1;
    const double fwd4_b = 74.1;
};
