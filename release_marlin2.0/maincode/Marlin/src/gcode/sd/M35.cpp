
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

#include "../../inc/MarlinConfig.h"

#if ENABLED(MULTI_VOLUME)

#include "../gcode.h"
#include "../../sd/cardreader.h"

/**
 * M35: Choose SD- card or U disk
 * harley  2021 -9 -16 
 */
void GcodeSuite::M35() {

    if (parser.seen('S'))
    {
       if(parser.value_bool())
       {
        card.changeMedia(&card.media_driver_usbFlash);  
        SERIAL_ECHOLNPGM("select U-disk"); 
       }
       else
       {
        card.changeMedia(&card.media_driver_sdcard);
        SERIAL_ECHOLNPGM("select SD-card"); 
       }
    }
    else
    {
       if(card.getMedia()==&card.media_driver_usbFlash)
       {
         SERIAL_ECHOLNPGM("using U-disk"); 
       }
       else if(card.getMedia()==&card.media_driver_sdcard)
       {
         SERIAL_ECHOLNPGM("using SD-card");   
       }
    }
}

#endif // MULTI_VOLUME
