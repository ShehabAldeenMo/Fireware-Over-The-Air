#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <Firebase_ESP_Client.h>
#include "LittleFS.h"


/**************************************************************************/
#define TWOBYTES                                16
#define APPLICATION_BASE_ADDRESS                0x08004400
#define ADDRESS_INDEX                           2
#define PACKET_LENGTH_FILE_TRANS                64
#define ACK                                     10
#define FLASH_PAYLOAD_WRITING_PASSED            0x01

void SendFileToBootloader(void);
/**************************************************************************/

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the SD card interfaces setting and mounting
#include <addons/SDHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "NewSignal"
#define WIFI_PASSWORD "Encrpted#753951456"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCnu0Gv4eHBS3YngCYb6wSKASRG-p9XUPY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "shehabmohammed2002@gmail.com"
#define USER_PASSWORD "123456"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-70b84.appspot.com"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{

    Serial.begin(115200);
    Serial2.begin(115200);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    multi.addAP(WIFI_SSID, WIFI_PASSWORD);
    multi.run();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
        if (millis() - ms > 10000)
            break;
#endif
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // The WiFi credentials are required for Pico W
    // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    /* Assign download buffer size in byte */
    // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
    // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);

    // if use SD card, mount it.
    //SD_Card_Mounting(); // See src/addons/SDHelper.h
}

// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
    if (info.status == firebase_fcs_download_status_init)
    {
        Serial.printf("Downloading file %s (%d) to %s\n", info.remoteFileName.c_str(), info.fileSize, info.localFileName.c_str());
    }
    else if (info.status == firebase_fcs_download_status_download)
    {
        Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == firebase_fcs_download_status_complete)
    {
        Serial.println("Download completed\n");
    }
    else if (info.status == firebase_fcs_download_status_error)
    {
        Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
    }
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nDownload file...\n");

        // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "ApplicationEdited.bin" /* path of remote file stored in the bucket */, "/update.bin" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, fcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
            SendFileToBootloader();
    }
}

void SendFileToBootloader(void){
  char Buf[2]={0};
  char Packet[71]={0};
  char CounetrByte = 0 ;
  char HexData = 0 ;
  int  Address = APPLICATION_BASE_ADDRESS ;
  
  File file = LittleFS.open("/update.bin", "r");
  
  while(file.available()){
      CounetrByte = 0 ;
      Packet[0] = 70;
      Packet[1] = 0x16;
      *(int*)(&Packet[ADDRESS_INDEX]) = Address ;
      Packet[6] = 64;
            
      while ( CounetrByte < 64 ){
        file.readBytes(Buf,2);
        HexData = strtol(Buf, NULL, TWOBYTES);
        Packet[CounetrByte+7] = HexData ;
        Serial.printf("\nPacket[%i]=%x",CounetrByte+7,Packet[CounetrByte+7]);
        CounetrByte++; 
      }
      
      for (char i = 0 ; i < 7 ; i++){
        Serial2.write(Packet[i]);
      }

      CounetrByte=0;
      
      while (CounetrByte<64){
        Serial2.write(Packet[CounetrByte+7]);
        CounetrByte++;
      }
      /* Read ack state */
      while (!Serial.available());  
      Serial.read();
  }
}

void Bootloader_Test(){
  int receivedValue = 0 ;
      if (receivedValue == ACK) {
        /* Read number of sending bytes */
        while (!Serial.available());                
        receivedValue = Serial.read();
        Serial.printf("\nbootloader Acknowledge with (%d) bytes",receivedValue);

        /* Read status of payload */
        while (!Serial.available());
        receivedValue = Serial.read();
        
        if (receivedValue == FLASH_PAYLOAD_WRITING_PASSED){
          Serial.println("\nPayload Written Successfully");
        }
        else {
          Serial.println("\nPayload Written failed");
        }
      } 
      else {
        // Not 0xCD, continue waiting
        Serial.println("\nbootloader Not-Acknowledge");
        Serial.printf("\nbootloader Not-Acknowledge with (%d) bytes",receivedValue);
      }
}
