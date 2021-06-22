# ESP32 Debugging Guide  
Welcome to the wonderful world of ESP32 debugging. A *supposedly* simple process that has caused me to lose too many hours of sleep...  

Nice general guide: https://docs.platformio.org/en/latest/plus/debug-tools/esp-prog.html

# Hardware  
 - ESP32 DevkitC (or something similar):    
<p align="center"><img align="center" width="400px" src="https://docs.espressif.com/projects/esp-idf/en/v3.1.5/_images/esp32-devkitc-functional-overview1.jpg"/></p>   

 - ESP Prog:  
<p align="center"><img align="center" width="400px" src="https://docs.espressif.com/projects/espressif-esp-iot-solution/en/latest/_images/modules.png"/></p>   

 - Some jumpers  
 - One (or two) USBs  

# Setup  
**Installing Drivers**
 - Connect ESP Prog    
 - Install FTDI driver (use the executable on the right!): https://ftdichip.com/drivers/vcp-drivers/  
 - Zadig: https://zadig.akeo.ie/    
	- Run    
	- Options->List All Devices     
	- Dropdown: select "Dual RS232-HS (Interface 0)"    
	            This programmer runs two serial interfaces (one for debug, one for programming). Have to select which one we are using for programming. This is interface 0 on ESP Prog and ESP Dev Kit.  
	- Driver: WinUSB (v...)    
	- Click "Replace Driver"    
	- Disconnect and reconnect  
 - In Device Manager, should see one device under COM ports and another under "Universal Serial Bus devices".  
	-> If this is not the case, need to repeat the zadig steps  

**Wiring**  
If using for just debugging:  
![Wiring connections](../resources/img/ESP32-Debugging-debugging-connections.png?raw=true)

If using for programming and debugging:  
![Wiring connections](../resources/img/ESP32-Debugging-all-connections.png?raw=true)

*NOTE:* On the normal dev board, there is a resistor (R17) connecting the onboard serial programmer to serial 0 on the ESP32. This has to be removed for serial programming from the ESP Prog to work.  
	-> If you do not care to do this, you will have to upload through USB to the device like normal. Everything should mostly be the same, just need two USB cables.   
	-> Also, if doing this, DO NOT connect the 3.3V from programmer -> ESP32.  

ESP Dev Kit schematic: https://dl.espressif.com/dl/schematics/esp32_devkitc_v4-sch-20180607a.pdf  

**PlatformIO Setup**  
platformio.ini:  

    platform = espressif32
    board = esp32dev
    framework = arduino
    upload_port = COM6
    monitor_port = COM6
    debug_tool = esp-prog
    ;upload_protocol = esp-prog  ;This may be a thing when uploading using the ESP Prog for programming
    monitor_speed = 115200

# Debugging Process  
To do the debugging:  
 - Left bar -> debugging tab  
 - Click the green triangle with "PIO Debug" next to it  
    NOTE: there is sometimes a stupidity where it "fails to find the task". If this happens, have to run "build". Don't have to let it finish, can cancel immediately.  
 - Go to "DEBUG CONSOLE" tab in console area  
 - Add some breakpoints, add some watches, etc.  

To view serial while debugging: should be able to click the "Open Serial" icon on the bottom bar (the plugin looking thing).  
