/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#include "../../inc/MarlinConfigPre.h"

#if HAS_EXTRUDERS

#include "../gcode.h"

#if ENABLED(EVT_HOMING_5X)
  bool e_is_absolute=true;
#endif
/**
 * M82: Set E codes absolute (default)
 */
void GcodeSuite::M82() {
     set_e_absolute();
     #if ENABLED(EVT_HOMING_5X)
      e_is_absolute=true;
     #endif
      }

/**
 * M83: Set E codes relative while in Absolute Coordinates (G90) mode
 */
void GcodeSuite::M83() { 
    set_e_relative();
     #if ENABLED(EVT_HOMING_5X)
      e_is_absolute=false;
     #endif    
     }

 #if ENABLED(EVT_HOMING_5X)
    bool get_e_is_absolute()
    {
      return e_is_absolute;
    }
 void GcodeSuite::M89()
 {
     if(e_is_absolute)
     {
      SERIAL_ECHOLNPGM("e mode:absolute!");  
     }
     else
     {
      SERIAL_ECHOLNPGM("e mode:relative!");  
     }
 }
 #endif
#endif // HAS_EXTRUDERS
