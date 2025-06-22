#ifndef CORE_INC_MJ838_ADXL367_ADXL367_H_
#define CORE_INC_MJ838_ADXL367_ADXL367_H_

#include "main.h"

#if USE_ADXL367	// if this particular device is active

typedef struct	// struct describing the ADXL367 functionality
{
    uint16_t (*Read)(uint8_t reg_addr, uint8_t size);  // reads up to 2 bytes from accelerometer register
    void (*Write)(const uint8_t reg_addr, const uint8_t size, uint16_t value);  // writes up to 2 bytes to accelerometer register (by value)
} adxl367_t;

adxl367_t* adxl367_ctor(void);	// the ADXL367 constructor

// ADXL367 register map
// #define REGISTER_NAME REGISTER_ADDRESS, NUMBER_OF_BYTES  // comment
#define DEVID_AD 0x00, 1 // Analog Devices device ID - 0xAD
#define DEVID_MST 0x01, 1 // Analog Devices MEMS device ID - 0x1D
#define PART_ID 0x02, 1 // device ID - 0xF7
#define REV_ID 0x03, 1 //  product revision ID - 0x03
#define SERIAL_NUMBER 0x04, 3 // 31-bit product serial number
#define XDATA_H 0x08, 1 // X-Axis MSB
#define YDATA_H 0x09, 1 // Y-Axis MSB
#define ZDATA_H 0x0A, 1 // Z-Axis MSB
#define STATUS 0x0B, 1 // Status
#define FIFO_ENTRIES_L 0x0C, 1 // number of valid data samples present in the FIFO buffer, LSB
#define FIFO_ENTRIES_H 0x0D, 1 // number of valid data samples present in the FIFO buffer, MSB
#define XDATA 0x0E, 2 // X-Axis Data H
//#define XDATA_L 0x0F // X-Axis Data L
#define YDATA 0x10, 2 // Y-Axis Data H
//#define YDATA_L 0x11 // Y-Axis Data L
#define ZDATA 0x12, 2 // Z-Axis Data H
//#define ZDATA_L 0x13 // Z-Axis Data L
#define TEMP 0x14, 2 // Temperature Data H
//#define TEMP_L 0x15 // Temperature Data L
#define EX_ADC 0x16, 2 // External ADC Data H
//#define EX_ADC_L 0x17 // External ADC Data L
#define I2C_FIFO_DATA 0x18, 1  // I2C Slave FIFO Data
#define SOFT_RESET 0x1F, 1 // Soft Reset
#define THRESH_ACT 0x20, 2 // Activity Threshold H
//#define THRESH_ACT_L 0x21 // Activity Threshold L
#define TIME_ACT 0x22, 1 // Activity Time
#define THRESH_INACT 0x23, 2 // Inactivity Threshold H
//#define THRESH_INACT_L 0x24 // Inactivity Threshold L
#define TIME_INACT 0x25, 2 // Inactivity Time H
//#define TIME_INACT_L 0x26 // Inactivity Time L
#define ACT_INACT_CTL 0x27, 1 // Axis Enable Control for Activity and Inactivity Detection
#define FIFO_CTL 0x28, 1 // FIFO Control
#define FIFO_SAMPLES 0x29, 1 // Samples in FIFO
#define INT_MAP1_L 0x2A, 1 // Interrupt Mapping Control 1
#define INT_MAP2_L 0x2B, 1 // Interrupt Mapping Control 2
#define FILTER_CTL 0x2C, 1 // Filter Control
#define POWER_CTL 0x2D, 1 // Power Control
#define SELF_TEST 0x2E, 1 // Self-Test
#define TAP_THRESH 0x2F, 1 // Tap Threshold
#define TAP_DUR 0x30, 1 // Tap Duration
#define TAP_LATENT 0x31, 1 // Tap Latency
#define TAP_WINDOW 0x32, 1 // Tap Window
#define X_OFFSET 0x33, 1 // X-Axis Offset Correction
#define Y_OFFSET 0x34, 1 // Y-Axis Offset Correction
#define Z_OFFSET 0x35, 1 // Z-Axis Offset Correction
#define X_SENS 0x36, 1 // X-Axis Sensitivity Adjustment
#define Y_SENS 0x37, 1 // Y-Axis Sensitivity Adjustment
#define Z_SENS 0x38, 1 // Z-Axis Sensitivity Adjustment
#define TIMER_CTL 0x39, 1
#define INTMAP1_H 0x3A, 1 // Interrupt Mapping Control 1
#define INTMAP2_H 0x3B, 1 // Interrupt Mapping Control 2
#define ADC_CTL 0x3C, 1 // ADC Control - 0xC0
#define TEMP_CTL 0x3D, 1 // Temperature Control
#define TEMP_ADC_OVER_THRSH 0x3E, 2 // Temperature ADC Over-Temperature Threshold
//#define TEMP_ADC_OVER_THRSH_L 0x3F // Temperature ADC Over-Temperature Threshold
#define TEMP_ADC_UNDER_THRSH 0x40, 2 // Temperature ADC Under-Temperature Threshold
#define TEMP_ADC_UNDER_THRSH_L 0x41 // Temperature ADC Under-Temperature Threshold
#define TEMP_ADC_TIMER 0x42, 1 // Temperature ADC Measurement Timer
#define AXIS_MASK 0x43, 1 // Axis Control for Single/Double Tap
#define STATUS_COPY 0x44, 1 // Status Copy
#define STATIS2 0x45, 1 // Status 2 - 0x40

#endif

#endif /* CORE_INC_MJ838_ADXL367_ADXL367_H_ */