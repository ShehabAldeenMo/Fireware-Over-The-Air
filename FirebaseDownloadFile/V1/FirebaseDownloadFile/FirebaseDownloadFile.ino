/*======================    Includes             =====================*/
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
#include <PubSubClient.h>
#include "LittleFS.h"
#include <addons/TokenHelper.h>   // Provide the token generation process info.
#include <addons/SDHelper.h>      // Provide the SD card interfaces setting and mounting



/*======================    Definations             =====================*/
/* 1. Define the WiFi credentials */
#define WIFI_SSID                     "Ali_Mamdouh"
#define WIFI_PASSWORD                 "1234567"

#define mqtt_server                   "test.mosquitto.org"           /* MQTT Server */
#define TWOBYTES                      16       
#define FLASH_BASE_ADDRESS            0x08000000   /* Flash base address of our micro-controller */
#define APPLICATION1_OFFEST           0x5000       /* Application 1 base address */
#define APPLICATION2_OFFEST           0xA000       /* Application 1 base address */
#define ADDRESS_INDEX                 2            /* Index of address in packet from esp to stm */
#define PACKET_LENGTH_FILE_TRANS      64           /* The max number of payload data length of transferring file */


/* 2. Define the API Key */
#define API_KEY "AIzaSyAoYaJ1JTY7dG4ZMG4J5PH1qKnsL_6be1Q"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "frinfkk@gmail.com"
#define USER_PASSWORD "1234567"

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-fe781.appspot.com"

/*======================    Global Variables             =====================*/
int  Global_Address_Counter  = 0 ;                    /* The number of packets used to send file 64byte by 64byte */ 

char Global_SendingFile_Flag = 0 ;                    /* To ensure that is related to writing payload function   */
char Global_SendingFile_Counter = 0 ;                 /* Counter to function of Sending file o determine the time of calling */ 
char Global_SendingFile_ECU  = 0 ;                    /* The number of ECU hat you want to write into */
char Global_SendingFile_APP  = 0 ;                    /* The number of application */

char Global_Erase_Counter    = 0 ;                    /* To count the input parameters in erase function */
char Global_Erase_Flag       = 0 ;                    /* To ensure that is related to erase function */

FirebaseData fbdo;                                    /* Define Firebase Data object */
FirebaseAuth auth;
FirebaseConfig config;
bool taskCompleted = false;
WiFiClient espclient;
PubSubClient client(espclient);

/*======================        Functions                =====================*/
void SetupWiFi();                                /* To set configurations of WiFi */
void MQTTcallback(char* topic, byte* payload, unsigned int length);
                                                  /* Call back for reciving and transmit to MQTT server */
void reconnect();                                 /* Reconnect on MQTT Server */
void MessageInfo(int Message);                    /* To jump on specific function that meets target */
void Erase_Flash(int Message);                    /* To erase specific pages */
void SendFileToBootloader(int Message);           /* To track file on firebase and transmit it */
void Change_Read_Protection_Level();              /* Change protection level for specific pages */

/*=========================  System Setup   ====================================*/
void setup()
{
    /* Initialize uart 0 and 2 */
    Serial.begin(115200);
    Serial2.begin(115200);

    /* To connect on MQTT server */
    Client.setServer(mqtt_server, 1883);
    Client.setCallback(MQTTcallback);
    
    /* To set configurations of WiFi */
    SetupWiFi();
    /* Assign the api key (required) */
    config.api_key = API_KEY;    

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
  
    Serial.printf("Firebase client v%s\n\n", FIREBASE_Client_VERSION);

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
}


/*==================== System loop =================================*/
void loop()
{
    if (!client.connected()) {
      reconnect();
    }
    
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nDownload file...\n");

        // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
        if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "ApplicationEdited.bin" /* path of remote file stored in the bucket */, "/update.bin" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, fcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
            //SendFileToBootloader();
    }
    
    client.loop();
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

void  SetupWiFi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin (WIFI_SSID,WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED ){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  String string;
  int Local_Message = 0 ; 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
     string+=((char)payload[i]);   
  }
  
  if (topic ="Ali_FromBrokerToEsp"){  
    Serial.println(string); 
    Local_Message = string.toInt();
    MessageInfo(Local_Message);
    delay(15);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection..."); 
    
    if (client.connect("ESPclient")) {
      Serial.println("connected");
      client.subscribe("Ali_FromBrokerToEsp");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds"); 
      delay(5000);
    }
   }
}

void MessageInfo(int Message){
   if (Message == 1 || Global_SendingFile_Flag == 1){
    Global_SendingFile_Counter++;
    Global_SendingFile_Flag = 1 ;
    SendFileToBootloader(Message);
   }
   else if (Message == 2 || Global_Erase_Flag != 0 ){
    Global_Erase_Counter++;
    Global_Erase_Flag = 1 ;
    Erase_Flash(Message);
   }
   else if (Message == 3){
    Change_Read_Protection_Level();
   }
}

void SendFileToBootloader(int Message){
  char Buf[2]={0};
  char Packet[73]={0};
  char CounetrByte = 0 ;
  char HexData = 0 ;
  static int  Address = FLASH_BASE_ADDRESS ;

  if (Global_SendingFile_Counter == 1){
    //client.publish("FOTA/Commends","Sending file is running ");
    //client.publish("FOTA/Commends","Enter the ECU number ");
  }
  if (Global_SendingFile_Counter == 2){
    //client.publish("FOTA/Commends","Enter the application number");
    Global_SendingFile_ECU = Message ;
  }
  else if (Global_SendingFile_Counter == 3){
    Global_SendingFile_APP = Message ; 

    /* To check on Application number  */
    if (Global_SendingFile_APP == 1){
       Address += APPLICATION1_OFFEST ;
    }
    else if (Global_SendingFile_APP = 2){
       Address += APPLICATION2_OFFEST ;
    }
    else {
       client.publish("FOTA/Commends","Not Valid Application Number");
       return ;
    }
    
    File file = LittleFS.open("/update.bin", "r");
  
    while(file.available()){
      CounetrByte = 0 ;
      Packet[0] = 72;
      Packet[1] = 0x16;
      *(int*)(&Packet[ADDRESS_INDEX]) = Address+64*Global_Address_Counter ;
      Packet[6] = Global_SendingFile_ECU ;
      Packet[7] = Global_SendingFile_APP ;
      Packet[8] = 64;
            
      while ( CounetrByte < 64 ){
        file.readBytes(Buf,2);
        HexData = strtol(Buf, NULL, TWOBYTES);
        Packet[CounetrByte+9] = HexData ;
        Serial.printf("\nPacket[%i]=%x",CounetrByte+7,Packet[CounetrByte+9]);
        CounetrByte++; 
      }
      
      for (char i = 0 ; i < 73 ; i++){
        Serial2.write(Packet[i]);
      }

      /* Read ack state */
      Global_Address_Counter++;
      while (!Serial.available()); 
      Serial.read();
     }//end while 

     Global_SendingFile_Counter = 0 ;
     Global_SendingFile_ECU     = 0 ;
     Global_SendingFile_Flag    = 0 ;
     Global_SendingFile_APP     = 0 ;
     Global_Address_Counter     = 0 ;
     //client.publish("FOTA/Commends","File is fully transmitted");
     file.close();
  }//end else if == 3
}

void Erase_Flash(int Message){
  char Packet[8]={0};
  int Flash_Base_Address = FLASH_BASE_ADDRESS;
  static int Page_Number = 0 ;
  static int Page_Amount = 0 ;

  /* 1st iteration to ensure that the Global_Erase_Flag is risied to enter erase function in next iteration from MessageInfo function*/
  /* To enter number of page */
  if (Global_Erase_Counter == 1){
    //client.publish("FOTA/Commends","Erase flash is runing");
    //client.publish("FOTA/Commends","Enter page number");
  }
  else if (Global_Erase_Counter == 2){
    //client.publish("FOTA/Commends","Enter amount of erasing pages");
    Page_Number = Message ; 
  }
  /* To determine amount of pages that you want to erase it */
  else if (Global_Erase_Counter == 3){
    //client.publish("FOTA/Commends","Enter ECU number");
    Page_Amount = Message;
  }
  else if (Global_Erase_Counter == 4){
    /* Packet formation */
    Packet[0] = 7;
    Packet[1] = 0x15;
    *(int*)(&Packet[ADDRESS_INDEX]) = Flash_Base_Address+1024*Page_Number ;
    Packet[6] = Message;
    Packet[7] = Page_Amount;

    /* Packet transmitting */
    for (char i = 0 ; i < 8 ; i++){
      Serial2.write(Packet[i]);
    } 

    /* To initialize global variables in Erase function after transmit data  */
    Global_Erase_Counter = 0 ;
    Global_Erase_Flag  = 0 ;
    Page_Number = 0 ;
    Page_Amount = 0 ;
  }//end else if
}

void Change_Read_Protection_Level(){
  
}
