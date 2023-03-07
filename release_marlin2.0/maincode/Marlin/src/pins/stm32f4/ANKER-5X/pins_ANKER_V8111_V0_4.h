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

#include "../env_validate.h"

// USB Flash Drive support
#define HAS_OTG_USB_HOST_SUPPORT

//Used to detect the hardware version number and which chip is used by the hardware
#if ENABLED(BOARD_CONFIGURE)
  #define BOARD_VERSION_GPIO                GPIOA
  #define BOARD_VERSION_PIN                 PIN0   
  #define BOARD_CHIP_GPIO                   GPIOA 
  #define BOARD_CHIP_PIN                    PIN1   
#endif

// Use one of these or SDCard-based Emulation will be used
#if NO_EEPROM_SELECTED
  //#define SRAM_EEPROM_EMULATION                 // Use BackSRAM-based EEPROM emulation
  #define FLASH_EEPROM_EMULATION                  // Use Flash-based EEPROM emulation
#endif

#if ENABLED(FLASH_EEPROM_EMULATION)
  // Decrease delays and flash wear by spreading writes across the
  // 128 kB sector allocated for EEPROM emulation.
  #define FLASH_EEPROM_LEVELING
#endif

// Avoid conflict with TIMER_TONE
#define STEP_TIMER                            10

#if HOTENDS > 2 || E_STEPPERS > 2
  #error "ANKER_V8111_V1_0 supports up to 2 hotends / E-steppers."
#endif

#ifdef NOZZLE_AS_PROBE 
  #define NOZZLE_BOARD_PWR_PIN              PC5
  #define NOZZLE_BOARD_PWR_STATE            HIGH
  #define PROVE_CONTROL_PIN                 PC10
  #define PROVE_CONTROL_STATE               HIGH
  #define HOMING_RISE_SPEED                 40*60     //mm/min
  // Z Probe (when not Z_MIN_PIN)
  #ifndef Z_MIN_PROBE_PIN
     #if MACCHINE == MOTOR_5X_DVT_USE_G_SENSOR
      //#define Z_MIN_PROBE_PIN                  PD1
     #else
       #define Z_MIN_PROBE_PIN              PB0
       #define Z_MIN_PROBE_STATE            HIGH
     #endif
  #endif
#else
  #define HOMING_RISE_SPEED                 40*60     //mm/min
  // Z Probe (when not Z_MIN_PIN)
  #ifndef Z_MIN_PROBE_PIN
    #define Z_MIN_PROBE_PIN                  PD1
  #endif
#endif

#if ENABLED(ANKER_NOZZLE_BOARD)
  #define ANKER_NOZZLE_BOARD_DEFAULT_THRESHOLD 400
  #define ANKER_Z_SENSORLESS_PROBE_VALUE       800
  #define ANKER_NOZZLE_BOARD_DEFAULT_FIREPROOF0  195 //0--255   195--70C   208--60
  #define ANKER_NOZZLE_BOARD_DEFAULT_FIREPROOF1  195
#endif

#if ENABLED(MOTOR_EN_CONTROL) 
  #define MOTOR_EN_PIN                      PA7
  #define MOTOR_EN_STATE                    HIGH
#endif

#if ENABLED(HEATER_EN_CONTROL) 
  #define HEATER_EN_PIN                     PC2
  #define HEATER_EN_STATE                   HIGH
#endif

#if ENABLED(ANKER_LEVEING)
  #define ANKER_LEVEING_DELAY_BEFORE_PROBING_TRUE  100//1000
  #define ANKER_LEVEING_DELAY_BEFORE_PROBING  500//5000
#endif

#if ENABLED(HANDSHAKE)
  #define HANDSHAKE_SDO                     PC4
  #define HANDSHAKE_SCK                     PB0
  #define HANDSHAKE_STATE                   HIGH
  #define HANDSHAKE_TIME                    5000//ms
#endif

#if ENABLED(HOMING_BACKOFF)
  #define HOMING_5X_BACKOFF_MM  { 0, 20, 0 }  // (mm) Backoff from endstops before  homing
#endif

#if ENABLED(WS1_HOMING_5X)
  #define HOMING_DUAL_Z_RISZ  0     // (mm) Ascent distance between return to zero
  #define HOMING_PROBE_Z_RISE 0     // (mm) The distance that the nozzle rises after moving to the middle point
#endif

#if ENABLED(EVT_HOMING_5X)
  // #define ANKER_Z_SAFE_HOMING_X_POINT 185  // X point for Z homing
  // #define ANKER_Z_SAFE_HOMING_Y_POINT 237.8  // Y point for Z homing
  // #define ANKER_Z_AFTER_HOMING        2     // (mm) Halfway up the Z
  // #define ANKER_HOMING_SCRIPT  "G92 E0\nG1 X35 Y237.8 E10 F3000\nG1 X117.5 Y237 E15 F3000\nG1 E10 F3600\nG92 E0\n"

  #define ANKER_Z_SAFE_HOMING_X_POINT 50  // X point for Z homing
  #define ANKER_Z_SAFE_HOMING_Y_POINT 237  // Y point for Z homing
  #define ANKER_Z_AFTER_HOMING        2     // (mm) Halfway up the Z6.2
  #ifndef ANKER_Z_HOMING_SCRIPT
   #define ANKER_Z_HOMING_SCRIPT "G1 X1 Y252 F7200\n"
  #endif
  #define ANKER_HOMING_SCRIPT_ABSOLUTE  "G92 E0\nG1 X130 Y237 F3000\nG1 E15 F3000\nG1 X225 Y237 E21 F3000\nG1 Z10 E11 F1200\nG1 E6 F3600\nG4 S10\nG1 Z0.2 F600\nG1 Y236 F1000\nG1 X140 Y236 F1000\nG92 E0\n"
  #define ANKER_HOMING_SCRIPT_NO_ABSOLUTE "G92 E0\nG1 X130 Y237 F3000\nG1 E15 F3000\nG1 X225 Y237 E6 F3000\nG1 Z10 E-10 F1200\nG1 E-5 F3600\nG4 S10\nG1 Z0.2 F600\nG1 Y236 F1000\nG1 X140 Y236 F1000\nG92 E0\n"
  #define PROBE_HOMING_BUMP_MM      { 0, 0, 2 } 
#endif

#if ENABLED(ANKER_ANLIGN)
 #define ALIGN_PER_RESET
 #define ANLIGN_RISE 1       //ascent
 #define ANLIGN_NUM  6
 #define ANLIGN_ALLOWED  0.1  //Error allowed
 #define ANLIGN_MAX_VALUE 2  
 #define SCREW_DISTANCE  334  //mm
#endif

#ifdef SENSORLESS_HOMING
 #define ANKER_SENSORLESS_HOMING_BACKOFF_MM  { 5, 5, 0 }  // (mm) Backoff from endstops before  homing
#endif

#if ENABLED(NO_MOTION_BEFORE_HOMING)
   #define DISABLED_MOTION_BEFORE_PIN    PC15
   #define DISABLED_MOTION_BEFORE_STATE  LOW
#endif
//
// Limit Switches
//
#ifdef SENSORLESS_HOMING
  #define X_STOP_PIN                  PC14  // X-STOP
  #define Y_STOP_PIN                  PE10  // Y-STOP
#else
  #define X_STOP_PIN                  PC15  // X-STOP
  #define Y_STOP_PIN                  PE11  // Y-STOP
#endif

#if ENABLED(Z_MULTI_ENDSTOPS)
 #if ENABLED(USE_Z_SENSORLESS) 
    #define Z_MIN_PIN                        PD14  // Z1-STOP
    #define Z_MAX_PIN                        PB10  // Z2-STOP
  #else
    #undef Z_MIN_ENDSTOP_INVERTING
    #undef Z2_MIN_ENDSTOP_INVERTING
    #define Z_MIN_ENDSTOP_INVERTING true // Set to true to invert the logic of the endstop.
    #define Z2_MIN_ENDSTOP_INVERTING true // Set to true to invert the logic of the endstop.
    #define Z_MIN_PIN                          PE1  // Z1-STOP
    #define Z_MAX_PIN                          PB3  // Z2-STOP
  #endif
#else
  #define Z_STOP_PIN                         PE1//PB3  // Z1-STOP
#endif

#define X_ENABLE_PIN                       PE4
#define X_STEP_PIN                         PE3
#define X_DIR_PIN                          PE6
#ifndef X_CS_PIN
  #define X_CS_PIN                         PD1
#endif

#define Y_ENABLE_PIN                       PE7
#define Y_STEP_PIN                         PB2
#define Y_DIR_PIN                          PE8
#ifndef Y_CS_PIN
  #define Y_CS_PIN                         PD0
#endif

#define Z_ENABLE_PIN                       PD3
#define Z_STEP_PIN                         PC9
#define Z_DIR_PIN                          PB7

#define Z2_ENABLE_PIN                      PD2
#define Z2_STEP_PIN                        PC8
#define Z2_DIR_PIN                         PB5

// #define Z_ENABLE_PIN                       PD2
// #define Z_STEP_PIN                         PC8
// #define Z_DIR_PIN                          PB5

// #define Z2_ENABLE_PIN                      PD3
// #define Z2_STEP_PIN                        PC9
// #define Z2_DIR_PIN                         PB7

#define E0_ENABLE_PIN                      PA8
#define E0_STEP_PIN                        PD15
#define E0_DIR_PIN                         PA15

//
// Temperature Sensors
//
#define TEMP_BED_PIN                        PA4   // TB
#define TEMP_0_PIN                          PC0   // TH0
//#define TEMP_BOARD_PIN                      PA0   // T BOARD
//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
  #define HEATER_0_PIN                      PE5   // Heater0
#endif

#ifndef HEATER_BED_PIN
  #define HEATER_BED_PIN                    PB15//PB4   // Hotbed
  #if ENABLED(COMPATIBLE_0_2AND_0_3)
   #define HEATER_BED_PIN2                   PB4
  #endif
#endif

#define HEATER_0_INVERTING true
#define HEATER_BED_INVERTING true
#define HEATER_BED_CTRL_2_PIN PB12
#define HEATER_BED_CTRL_2_INVERTING false

#ifndef FAN_PIN
  #define FAN_PIN                           PE12   // Fan0
#endif
#ifndef FAN1_PIN
  #define FAN1_PIN                          PE13   // Fan1
#endif
// #ifndef FAN2_PIN
//   #define FAN2_PIN                       PB13  // Fan2
// #endif
// #ifndef FAN3_PIN
//   #define FAN3_PIN                       PB14  // Fan3
// #endif

#ifdef TMC_USE_SW_SPI
  #define TMC_SW_MOSI                       PD4     
  #define TMC_SW_MISO                       PD5    
  #define TMC_SW_SCK                        PD6    
#endif

#if HAS_TMC_UART
  /**
   * TMC2208/TMC2209 stepper drivers
   *
   * Hardware serial communication ports.
   * If undefined software serial is used according to the pins below
   */

  #define X_SERIAL_TX_PIN                   PC13
  #define X_SERIAL_RX_PIN        X_SERIAL_TX_PIN

  #define Y_SERIAL_TX_PIN                   PE9
  #define Y_SERIAL_RX_PIN        Y_SERIAL_TX_PIN

  #define Z_SERIAL_TX_PIN                   PD7
  #define Z_SERIAL_RX_PIN        Z_SERIAL_TX_PIN

  #define Z2_SERIAL_TX_PIN                  PE0
  #define Z2_SERIAL_RX_PIN       Z2_SERIAL_TX_PIN

  #define E0_SERIAL_TX_PIN                  PC12
  #define E0_SERIAL_RX_PIN      E0_SERIAL_TX_PIN
  // Reduce baud rate to improve software serial reliability
  #define TMC_BAUD_RATE                    19200
#endif
