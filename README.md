# _Firmware updates Over The Air_
This project is to enable remote firmware updates using **ESP32**, **STM32F103C8T6** and **Geogle Firebase**.The **ESP32** connects to **WiFi**, downloads the bootloader and firmware code, and communicates with the STM32.
![Screenshot 2024-02-01 104155](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/b0d02c6f-00ba-4343-941e-dbdb82eed83a)

# _Table of Contents_
1. [Introduction](#Introduction)
2. [Technology Overview](#Technology-Overview)
3. [How To Use](#How-To-Use)

## _**Introduction**_
This project facilitates firmware updates over-the-air (FOTA) for an STM32F103 microcontroller using an ESP32 and Google Firebase. Key components include WiFi connectivity, Firebase authentication, and MQTT communication.<br /> 

## _Technology Overview_
_**1- IoT Integration**_ <br />
**1.1 WiFi and MQTT** <br /> 
Leveraged the WiFi network to connect to the MQTT server "test.mosquitto.org." Subscribed to various topics on the broker representing commands to the master branch in the microcontroller tree.<br />
**1.2 Firebase Integration** <br /> 
Connected to Firebase to download the BootloaderApplication with an unlimited license, storing it in the ESP flash memory.<br />
**1.3 NodeRED for Web Server** <br /> 
Utilized NodeRED to create a web server based on a Python script (GUI). This server allows users to configure commands visually.<br />
**1.4 Command Handling** <br /> 
**1.4.1 File Transmission** <br /> 
Implemented file transmission by dividing files into packets with application base addresses and command numbers, ensuring acknowledgments for reliability.<br /> 
**1.4.2 Flash Erasure** <br />  
Allowed users to send commands for erasing flash by specifying base page numbers and the number of pages through NodeRED.<br /> 
**1.4.3 Security Measures** <br />  
Implemented measures to apply specific levels of protection on flash pages to safeguard applications from potential threats.<br /> 

_**2- Communication Protocols**_ <br /> 
**UART**<br /> 
Utilized UART for communication between the ESP and master microcontroller.<br />  Employed UART to form a tree of microcontrollers. Each node in the tree is controlled through the UART bus, sending commands and updating applications.<br /> 

_**3- Cybersecurity**_ <br /> 
Implemented the RSA encryption algorithm in C to ensure secure communication. This included key generation, string and numeric conversion, encryption, and decryption.<br /> 

_**4- Bootloader**_ <br /> 
**4.1 Command Set** <br /> 
Developed various commands in the bootloader, such as writing into memory, changing read protection levels, jumping to addresses, erasing flash, obtaining identification numbers, getting help, obtaining versions, and reading protection levels.<br /> 
**4.2 Bootloader Logic** <br /> 
Specified the last page in flash memory to determine the bootloader and application status, ensuring proper application execution after resets.<br /> 

_**5- LinkerScript and Startup Code**_ <br /> 
**5.1 LinkerScript Usage** <br /> 
Defined code sizes for bootloaders and applications, including flags for monitoring their status in memory.<br /> 
**5.2 Startup Code** <br /> 
Used generated startup code to access the reset handler and stack pointer, facilitating the execution of new code on microcontrollers.<br /> 

_**6- FreeRTOS**_ <br /> 
Integrated FreeRTOS for efficient task scheduling and management within the system.<br /> 

_**7- Git Collaboration**_ <br /> 
Collaborated on GitHub, utilizing Git for version control, opening and fixing issues, creating new commits, and ensuring a smooth collaborative workflow.<br /> 

_**8- Doxygen Documentation**_ <br /> 
Employed Doxygen to generate comprehensive documentation, ensuring clarity and ease of understanding for future development and maintenance.<br /> 

## How To Use
+ If you want to connect your bootloader with pythonScript that named as **Shehab.py** to communicate with bootloader througth uart without ESP32.<br />
  configure bootloader.h with "python = 1 and ESP = 0" and you can use all main features.<br />

+ If you want to use it as Firmware Over The Air<br />
  configure bootloader.h with "python = 0 and ESP = 1"<br />
  > [NOTE]
  > We dtermine this specific functions to be commiuncate between ESP and STM because the main target of FOTA is uploading applications and erase them and determine thoer protection     
    level.<br />
  > Any other specification you can move into flash in freely movements with **Shehab.py**.<br />

1. Create any project and generate its binary file. Edited its name to **Application.txt**.<br />
2. Use pythonscript that It's named "test.py" in **BinaryFile** folder to track whole file and generates **ApplicationEdited.bin** without any spaces or new lines.<br />
3. Upload **ApplicationEdited.bin** to firebase storage.<br />
![Firebase](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/71ba20b2-ecf9-4a2e-a61c-69737b538e13)
4. **ESP32** will track whole file and save it in it's memory. and wait to action option from web dashboard.<br />
![Dash board](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/8e8f0287-d119-4696-b5f4-07b5df8d2bdf)
5. Press any button that determine your choice.<br />
  - If you pressed **Write Into Memory**: **ESP32** will transmit file to Master MCU.<br />
  - If you pressed **Erase Specific Pages** you must go to **Node Red Layput** as shown:<br />
![NodeRed Layout](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/a5c9a45e-0a03-4c8c-92cb-1ae178b81508)
  - And press double click to MQTT In under name **FOTA/Commends** then enter your page number then press **Deploy**. <br />
  - Press double click to MQTT In under name **FOTA/Commends** then enter your amount of pages then press **Deploy**.<br />
  - Check your this specific page.<br />
    
