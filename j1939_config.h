/******************************************************************************
 *
 * This file contains macros used in the J1939 library for QT
 *
******************************************************************************/

#ifndef J1939_CONFIG_H
#define J1939_CONFIG_H

#define ID_PRIORITY_MASK                 0x1C000000
#define EXTENDED_DATA_MASK               0x02000000
#define DATA_PAGE_MASK                   0x01000000
#define PDU_FORMAT_MASK                  0x00FF0000
#define PDU_SPECIFIC_MASK                0x0000FF00
#define SOURCE_ADRESS_MASK               0x000000FF
#define PGN_MASK                         0x00FFFF00
#define ADR_MASK                         0x000000FF
#define DTC_BITMASK                      0x00000001
#define FMI_MASK                         0x0000001F

/******************************************************************************
 *
 * Defined variables for data reception and transmission.
 *
******************************************************************************/

//Receive:
#define LINEAR_DISPLACEMENT_PGN          0xF032
#define ENGINE_TEMPERATURE_PGN           0xFEEE
#define VEHICLE_POSITION_PGN             0xFEE9
#define VEHICLE_ORIENTATION_PGN          0xFFFF

//Transmit:
#define HEATER_SP_PGN                    0xF037
#define TREAD_POS_PGN                    0xFFF8

//PGN65226 DM1 - Active Diagnostic Trouble Codes:
#define DM1_PGN                          0xFECA

/******************************************************************************
 *
 * Byte placement for data reception and transmission.
 * Indicates starting bit and length
 *
******************************************************************************/

// Receive:
#define LINEAR_DISPLACEMENT_LSB           1
#define LINEAR_DISPLACEMENT_MSB           0
#define ENGINE_TEMPERATURE_B              0
#define VEHICLE_POSITION_X_LSB            1
#define VEHICLE_POSITION_X_MSB            0
#define VEHICLE_POSITION_Y_LSB            3
#define VEHICLE_POSITION_Y_MSB            2
#define VEHICLE_ORIENTATION_X_LSB         2
#define VEHICLE_ORIENTATION_X_MSB         1

// Transmit:
#define HEATER_SETPOINT_BYTE              0
#define TREAD_POS_BYTE8                   4

/******************************************************************************
 *
 * Device address for identification
 *
******************************************************************************/

#define LINEAR_ADR                        0x48
#define TEMP_ADR                          0x50
#define POS_ADR                           0x63

/*****************************************************************************/

#define TEST_PGN                          0xFFBB

#define DM2_PGN                           0xFECB
#define DM3_PGN                           0xFECC
#define DM4_TEST_PGN                      0xFFF0

#define FUEL_GAUGE_DTC                    0xBEBA
#define TEMPERATURE_DTC                   0xBEEF
#define TACHOMETER_DTC                    0xBEB0

#define DTC_NO_FAULTS                     0x80
#define DTC_NO_NEW_FAULTS                 0x00

#define BYTE_DATA_PER_PACKET              0x08
#define EMPTY_PAYLOAD                     0x00
#define ECU_PRIORITY_LEVEL                0x06
#define EXTENDED_DATA_BIT                 0x00
#define DATA_PAGE_BIT                     0x00
#define ECU_SOURCE_ADDRESS                0x01
#define FMI_POS                           0x04

#define PRIORITY_SHIFT_POSITION           26
#define EXTENDED_DATA_SHIFT_POSITION      25
#define DATA_PAGE_SHIFT_POSITION          24
#define PDU_FORMAT_SHIFT_POSITION         16
#define PGN_SHIFT_POSITION                8

#define MSB_SHIFT_POSITION                8
#define BIT_CHECK_BEGINNING               0
#define BIT_CHECK_END                     7

#endif // J1939_CONFIG_H
