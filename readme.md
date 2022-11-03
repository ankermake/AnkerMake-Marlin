## AnkerMake M5 Firmware

AnkerMake M5 Firmware is a fork of version 2.0.x bugfix of the open-source Marlin Firmware adapted to the AnkerMake M5 high-speed printer. 

### Main features of the AnkerMake M5 Firmware

  1. 5× high-speed printing adaptation and optimization.
  2. Automatic leveling and automatic Z-offset calibration.
  3. Adapted the DMA data receiving and transmitting modes, driven by the Marlin Arduino serial port.
  4. G-code packet communication and error correction mechanisms between the upper computer and Marlin system.
  5. Local AI compatibility when monitoring AnkerMake prints.

### Compiling

  1. Installing Marlin (PlatformIO with VS Code)

  2.   https://marlinfw.org/docs/basics/install_platformio_vscode.html

  3. Prevent the Arduino library from being automatically updated to the online version
     
     1. Ensure that VS Code does not connect to the network when compiling Marlin.
     
     2. Delete the framework-arduinosttm32 in the C:\Users\xxx\.platformio\packages directory.
     
     3. Unzip the framework-arduinosttm32.7z to the C:\Users\xxx\.platformio\packages directory.
     
     4. Compile Marlin.
### License

AnkerMake's firmware is licensed under a [General Public License](https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/LICENSE). The GPL comes with both rights and obligations. The GPL ensures that if you use any part of this software in any way (even behind a web server), your software must be released under the same license.

**About Marlin License:**

​        Marlin is published under the [GPL license](https://github.com/MarlinFirmware/Marlin/blob/bugfix-2.1.x/LICENSE) because we believe in open development. The GPL comes with both rights and obligations. Whether you use the Marlin firmware as the driver for your open or closed-source product, you must keep Marlin open, and you must provide your compatible Marlin source code to end users upon request. The most straightforward way to comply with the Marlin license is to make a fork of Marlin on GitHub, perform your modifications, and direct users to your modified fork.