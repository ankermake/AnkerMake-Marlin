#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#define GCP_MODULE_00_INGENIC  0x00
#define GCP_MODULE_01_MARLIN   0x01
#define GCP_MODULE_02_NOZZLE   0x02
#define GCP_MODULE_03_MATERIAL 0x03

#define GCP_CMD_20_MSG_GET             0x20
#define GCP_CMD_21_PROBE_SET           0x21
#define GCP_CMD_22_LOG_UPLOAD          0x22
#define GCP_CMD_23_NO_MS               0x23
#define GCP_CMD_24_HWSW_VER            0x24
#define GCP_CMD_26_LOGO_LED            0x26
#define GCP_CMD_27_DOT_LED             0x27
#define GCP_CMD_28_ENV_T_ALARM         0x28
#define GCP_CMD_29_SN_WRITE            0x29
#define GCP_CMD_30_SN_READ             0x30
#define GCP_CMD_31_HEATER_OCI_RECOVERY 0x31
#define GCP_CMD_33_LEVELING_VAL_WRITE  0x33
#define GCP_CMD_34_LEVELING_VAL_READ   0x34
#define GCP_CMD_36_NOZZLE_POWER_ON    0x36

#define GCP_CMD_40_GET_PROBE_VALUE    0x40
#define GCP_CMD_41_SHOW_ADC_VALUE_ON  0x41
#define GCP_CMD_42_SHOW_ADC_VALUE_OFF 0x42
#define GCP_CMD_43_PID_AUTO_TURN      0x43
#define GCP_CMD_43_AUTO_OFFSET_START  0x44
#define GCP_CMD_45_M3001_DEAL         0x45
#define GCP_CMD_46_M3002_DEAL         0x46

#define GCP_CMD_F0_LED_IO_CTRL        0xF0
#define GCP_CMD_F1_TEMP_IO_CTRL       0xF1
#define GCP_CMD_F2_MODEL_FAN_IO_CTRL  0xF2
#define GCP_CMD_F3_LED_TEST           0xF3
#define GCP_CMD_F4_HEATER_FAN_IO_CTRL 0xF4

#endif /*__PROTOCOL_H__*/
