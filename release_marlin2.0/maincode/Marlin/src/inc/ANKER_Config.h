/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/*******************************Version number management****************************/
//Motherboard Version Management
#define MAIN_BOARD_V8111_V0_2            "V8111_MAIN_V0.2"
#define MAIN_BOARD_V8111_V0_3            "V8111_MAIN_V0.3"

#define MAIN_BOARD_VERSION               MAIN_BOARD_V8111_V0_2

//Motherboard chip management
#define STM32F407VGT6_CHIP              "STM32F407VGT6"
#define GD32F407VGT6_CHIP               "GD32F407VGT6"

#define MAIN_BOARD_CHIP                  GD32F407VGT6_CHIP

/*******************************Printer Profile Management****************************/
#define MACCHINE_5X_ID                     1000
#define MOTOR_5X_DVT                       1200

//Profile selection
#define MACCHINE   MOTOR_5X_DVT
//Motherboard temperature sensor configuration
#if (MACCHINE >=1000)&&(MACCHINE <2000)   //USE 5X
  #define MACCHINE_ID MACCHINE_5X_ID
#endif
/*******************************Functional API Configuration****************************/
#if MACCHINE == MOTOR_5X_DVT
    #ifndef SHORT_BUILD_VERSION
     #define SHORT_BUILD_VERSION            "V8111_V2.0.65"
    #endif
    #define ANKER_MAKE_API           1 //gcode
    #define ANKER_GCODE_CHECK        0
    #define PROVE_CONTROL            1       
    #define WS1_HOMING_5X            1
    #define EVT_HOMING_5X            1 //Cancel the left zeroing and move directly to the center point for zeroing
    #define REPORT_LEVEL_PORT        1
    #define USE_KELI_MOTOR           0 //Compatible with keli motors
    #define PROBE_COLSE_FAN1         0 //When the strain gauge returns to zero, turn off the cooling fan when pressing down
    #define MOTOR_EN_CONTROL         1 //Motor drive power control
    #define HEATER_EN_CONTROL        1 //Printhead Power Control
    #define HANDSHAKE                0 //Detect the presence of the printhead plate
    #define ACCELERATION_CONTROL     1 //ACCELERATION control
    #define HOMING_BACKOFF           0 //The distance to retreat from the end point before returning to zero
    #define ANKER_QUEUE              1
    #define ANKER_LOG_DEBUG          0 //Used to print logs for debugging
    #define READ_BUFFER_SIZE         1 //read BLOCK_BUFFER_SIZE  MAX_CMD_SIZE  BUFSIZE  
    #define BOARD_CONFIGURE          1 //Used to detect the hardware version number and which chip is used by the hardware
    #define PROBE_TEST               0
    #define COMPATIBLE_0_2AND_0_3    1 //Compatible with motherboards version 0.2 and 0.3
    #define PHOTO_Z_LAYER            1 //Photo function for each layer
    #define ANKER_PAUSE_FUNC         1 //Anker pause function enable/disable
    #define ANKER_MULTIORDER_PACK    1 //anekr multi order in one packet in once communication
    #define ANKER_LEVEING            1 //Leveling Correction Program
    #define ANKER_LEVEING_FADE       1
    #define ANKER_ANLIGN             1 //Z axis automatic alignment
    #define ANKER_TMC_SET            1 //For TMC driver settings
    #define ANKER_NOZZLE_BOARD       1 //
    #define ANKER_M_CMDBUF           1 //
    #define NO_CHECK_Z_HOMING        1 //Does not detect whether Z is zeroed
    #define ANKER_FIX_ENDSTOPR       1//Used to resolve limit switch conflicts on motherboards
    #define USE_Z_SENSORLESS         1 //
    #define USE_Z_SENSORLESS_AS_PROBE  0 //
    #define TMC_AUTO_CONFIG          0// Enable TMC2209 silent mode auto-configuration function
    #define ANKER_NOZZLE_PROBE_OFFSET 1 //
    #define ANKER_PRINT_SLOWDOWN     0 //this is for the machine to prevent to print slowdown 
    #define ANKER_LIN_PARAMETER      1 //this is for lin_advance
    #define ANKER_TEMP_WATCH         1
    #define ANKER_VALIDATE_HOMING_ENDSTOPS 1
    #define ANKER_TEMP_ADC_FILTER    1
#endif

/*******************************Error detection****************************/
#if HANDSHAKE
    #if PROVE_CONTROL==0
     #error "HANDSHAKE needs to be enabled PROVE_CONTROL"
    #endif
    #if HEATER_EN_CONTROL==0
     #error "HANDSHAKE needs to be enabled HEATER_EN_CONTROL"
    #endif
#endif


/*******************************header file management****************************/

// #if BOARD_CONFIGURE==(1)
//    //#include "../feature/anker/sys.h"
//    #include "../feature/anker/board_configure.h"
// #endif



