/****************************************************************************
 ******************************  Including  **********************************
 *****************************************************************************/
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

#include <PubSubClient.h>
#include <Firebase_ESP_Client.h>
#include <LittleFS.h>
#include <addons/TokenHelper.h>
#include <addons/SDHelper.h>









/****************************************************************************
 *****************************  Definitions  *********************************
 *****************************************************************************/
/*Define the API Key */
#define API_KEY "AIzaSyAoYaJ1JTY7dG4ZMG4J5PH1qKnsL_6be1Q"

/*Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "frinfkk@gmail.com"
#define USER_PASSWORD "1234567"

/*Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-fe781.appspot.com"

/*Change Read protection Config*/
#define CBL_CHANGE_ROP_Level_CMD                0x17
#define CBL_ROP_LEVEL_0                         0x00
#define CBL_ROP_LEVEL_1                         0x01
#define RP_LENGTH                               0x02

/*Read read protection*/
#define OB_RDP_LEVEL_0                       ((uint8_t)0xA5)
#define OB_RDP_LEVEL_1                       ((uint8_t)0x00)
#define READ_RP                              0x00
#define READ_WP                              0x01
#define CBL_GET_RDP_STATUS_CMD               0x13 /* Read the flash protection level command */
#define WRITE_IS_ENABLE                      0x00 //This Mask is only for low densties to check if All the 32 pages are write protected(it should changed for higher densties STM)
#define WRITE_IS_DISABLE                     0xFF //This Mask is only for low densties to check if All the 32 pages are write protected(it should changed for higher densties STM)

/*Change Write protection Config*/
#define CBL_CHANGE_WOP_Level_CMD                0x19
#define OB_WRPSTATE_DISABLE                     0x00
#define OB_WRPSTATE_ENABLE                      0x01
#define WP_LENGTH                               0x02

/*Read version Config*/
#define CBL_GET_VER_CMD                         0x10
#define VER_LENGTH                              0x02

/*MCU ID config*/
#define CBL_GET_CID_CMD                         0x12
#define MCUID_LENGTH                            0x01

/*Define the WiFi credentials */
#define WIFI_SSID         "Ali"
#define WIFI_PASSWORD     "1234567"
#define MQTT_SERVER       "test.mosquitto.org"

/*Configrations*/
#define USED                0x01
#define NOT_USED            0x00
#define SerialMonitor_RP    NOT_USED     //Change protection level of STM using Serial Monitor
#define NodeRed_RP          USED         //Change protection level of STM using Node-Red

/*Config Master and slave ECUS*/
#define MASTERID                                0x00
#define SLAVE1                                  0x01

/*Write Program definitions*/
#define TWOBYTES                          16       
#define CBL_MEM_WRITE_CMD                 0x16
#define MAX_MEM_WRITE_TOT_Length          72
#define MAX_MEM_WRITE_DATA_Length         64
#define ADDRESS_M_WRITE_INDEX             2 /* Index of address in packet from esp to stm */
#define ECU_M_WRITE_INDEX                 6 /* Index of ECU in packet from esp to stm */
#define WRITE_M_APP_INDEX                 7 /* Index of APP in packet from esp to stm */
#define WRITE_M_DATA_INDEX                8 /* Index of DATA length in packet from esp to stm */
#define FLASH_PAYLOAD_WRITING_FAILED      0x00
#define FLASH_PAYLOAD_WRITING_PASSED      0x01


/*APP definitions*/
#define FLASH_BASE_ADDRESS            0x08000000   /* Flash base address of our micro-controller */
#define APPLICATION1_OFFEST           0x5000       /* Application 1 base address */
#define APPLICATION2_OFFEST           0xA000           /* Application 1 base address */
#define APPLICATION1                  1
#define APPLICATION2                  2






//don't forget making variables for APPs
/****************************************************************************
 ***************************  Global Variables  ******************************
 *****************************************************************************/

/*Firebase Data objects*/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool taskCompleted = false;

/*GPIO variables*/
int led=2; //Led of ESP32

/*Wifi Data objects*/
WiFiClient espClient;
PubSubClient client(espClient);

/*MQTT variables*/
unsigned long LastMsg = 0;
char msg[50];
int value = 0;
char var;
String string; //String used to choose option coming from MQTT broker

/*ECUs variables*/
uint8_t ECU = MASTERID; //initialize with weong value

/*Apps variables*/
char App = APPLICATION1;//Default value App1
int  Address = FLASH_BASE_ADDRESS + APPLICATION1_OFFEST; //Default value App1









/****************************************************************************
 *************************  Functions Definitions  ***************************
 *****************************************************************************/

/*******************************************************************************
 *                             1-LittleFS functions                            *
********************************************************************************/
/*Initialize LittleFS*/
void Init_LittleFS(void)
{
   if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed. Restarting...");
        ESP.restart();
    }
    else
    {
    Serial.println("LittleFS Mounted!");
    }
}



/*Reading downloaded file in using LittleFS*/
void readfile(const char* path)
{
	char Buf[3]; //the third byte will be 0(NULL due to initialization) and it must be of size 3 to print buf throuh serial monitor without errors
	Serial.printf("Reading file: %s\n", path);

	File file = LittleFS.open(path, "r");
	if(!file)
	{
		Serial.println("Failed to open file for reading");
		return;
	}

	Serial.print("File size: ");
	Serial.println(file.size());

	Serial.print("Available size is ");
	Serial.println(file.size());

	//Serial.print("Read from file: ");
	while(file.available())
	{
		file.readBytes(Buf, 2); //Reads two bytes from the file and store them in buffer
		Serial.printf("%s", Buf);
	}

	file.close();
	Serial.println();
	Serial.println("Ending of readfile function");
}




/*******************************************************************************
 *                             2-Firebase functions                            *
********************************************************************************/

/*Init firebase(should be called from Setup)*/
void InitFirebase(void)
{
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password =    USER_PASSWORD;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(10000 /* Rx buffer size in bytes from 512 - 16384 */, 10000 /* Tx buffer size in bytes from 512 - 16384 */);

  /* Assign download buffer size in byte */
  // Data to be downloaded will read as multiple chunks with this size, to compromise between speed and memory used for buffering.
  // The memory from external SRAM/PSRAM will not use in the TCP client internal rx buffer.
  config.fcs.download_buffer_size = 2048;

  Firebase.begin(&config, &auth);

}


/*Download file from Firebase storage, then reading and printing it on serial monitor(should be called from loop) */
void DownloadAndReadFile_Firebase(void)
{
	// Firebase.ready() should be called repeatedly to handle authentication tasks.
	if (Firebase.ready() && !taskCompleted)
	{
		taskCompleted = true;

		Serial.println("\nDownload file...\n");

		// The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
		if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "ApplicationEdited.bin" /* path of remote file stored in the bucket */, "/App.bin" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, fcsDownloadCallback /* callback function */))
			Serial.println(fbdo.errorReason());
		  readfile("/App.bin");
	}

}



/*The Firebase Storage download callback function*/
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





/*******************************************************************************
 *                             3-MQTT functions                                *
********************************************************************************/
void Init_MQTT(void)
{
  client.setServer(MQTT_SERVER, 1883);
	client.setCallback(MQTT_Callback);
}



void MQTT_Callback(char* topic, byte* payload, unsigned int length)
{
	int Local_Message = 0 ;
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");

	for (int i = 0; i < length; i++) {
		string+=((char)payload[i]);
	}

	if (topic ="Ali_FromBrokerToEsp")
	{
		Serial.println(string);
		Local_Message = string.toInt();
		Serial.printf("%d", Local_Message);
		Serial.println();
		delay(15);
	}
	else
	{
		Serial.println("Wrong Topic!!!");
	}

}




void reconnect() {
	while (!client.connected()) {
		Serial.println("Attempting MQTT connection...");

		if (client.connect("ESPClient")) {
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




void Debug(char Mess)
{
	char var;
	while(Serial.available()>0)
	{
		var = Serial.read();
		if(var == Mess)
		{
			client.publish("Ali_FromEspToBroker","54321");
		}
		clearRx0Buffer(); //To Clear the buffer so it enters only once each Message
	}

}


void MQTT_Connect(void)
{
	/*In Case of diconnecting failure from MQTT server*/
	if (!client.connected())
	{
		reconnect();
	}

	/*Always connection to client*/
	client.loop();

}





/********************************************************************************
 *                              4-WiFi functions                                *
 ********************************************************************************/
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






/********************************************************************************
 *                             5-UART functions                                 *
 ********************************************************************************/
void Init_UART(void)
{
  Serial.begin(115200);
	Serial2.begin(115200);

	Serial.setRxBufferSize(1024); //To increase Buffer of Arduinio serial RX
	Serial.setTxBufferSize(300); //To increase Buffer of Arduinio serial TX

	Serial2.setRxBufferSize(1024); //To increase Buffer of Arduinio serial RX
	Serial2.setTxBufferSize(300); //To increase Buffer of Arduinio serial TX
}



void clearRx0Buffer(void) {
	while (Serial.available() > 0) {
		Serial.read(); //Discard the read byte.
	}
}


void clearRx2Buffer(void) {
	while (Serial2.available() > 0) {
		Serial2.read(); //Discard the read byte.
	}
}



/*******************************************************************************
 *                             6-App functions                                 *
********************************************************************************/


void Write_Program(void)
{
  String Compare_Write_Program = "Write_Program";

  if(Compare_Write_Program == string)
  {
    digitalWrite(led,HIGH);//For debug
    client.publish("Ali_FromEspToBroker","Sending APP to ECU..");
    char CounetrByte = 0 ;
    char ACK_Flag = FLASH_PAYLOAD_WRITING_FAILED;
    char HexData = 0 ;
    int Address_Counter = 0;
    int Remaining_Size;

    char Buf[2]={0};
    char Packet[73]={0};
    Packet[0] = MAX_MEM_WRITE_TOT_Length;
    Packet[1] = CBL_MEM_WRITE_CMD;
    Packet[ECU_M_WRITE_INDEX] = ECU;//Select which ECU to enter
    Packet[WRITE_M_APP_INDEX] = App;//Select which App to burn
    Packet[WRITE_M_DATA_INDEX] = MAX_MEM_WRITE_DATA_Length;

    File file = LittleFS.open("/App.bin", "r");
    if(!file)
    {
      client.publish("Ali_FromEspToBroker","Failed to open file for reading!!");
	    return;
    }

    while(file.available())
    {
    Remaining_Size = file.available()/2; //Available bytes(not characters so we divide by 2) in File
    *(int*)(&Packet[ADDRESS_M_WRITE_INDEX]) = Address + 64*Address_Counter;

    if(Remaining_Size >= MAX_MEM_WRITE_DATA_Length)
    {
      /*Read file and store it to Packets*/
       while ( CounetrByte < MAX_MEM_WRITE_DATA_Length ){
       file.readBytes(Buf,2);
       HexData = strtol(Buf, NULL, TWOBYTES);
       Packet[CounetrByte+9] = HexData ;
       Serial.printf("\nPacket[%i]=%x",CounetrByte+9,Packet[CounetrByte+9]);
       CounetrByte++; 
      }
      
      for (char i = 0 ; i < 73 ; i++){
        Serial2.write(Packet[i]);
      }
      delay(100);//Must Delay to sychnorize

      /* Read ack state */
      while (Serial2.available())
      {
      ACK_Flag = Serial2.read();
      }

      if(ACK_Flag != FLASH_PAYLOAD_WRITING_PASSED)
      {
        client.publish("Ali_FromEspToBroker","NACK Send from STM!!");        
      }
 
      /*To Update address*/
      Address_Counter++;
      CounetrByte = 0;
      
    }
    else //Must go to diffrent condition when size is not 73
    {
      /*Update new size to send it correct to bootloader*/
      Packet[0] = 8 + Remaining_Size;
      Packet[WRITE_M_DATA_INDEX] = Remaining_Size;

      /*Read file and store it to Packets*/
       while ( CounetrByte < Remaining_Size){
       file.readBytes(Buf,2);
       HexData = strtol(Buf, NULL, TWOBYTES);
       Packet[CounetrByte+9] = HexData ;
       Serial.printf("\nPacket[%i]=%x",CounetrByte+9,Packet[CounetrByte+9]);
       CounetrByte++; 
      }

      for (char i = 0; i < Remaining_Size + 9; i++){
        Serial2.write(Packet[i]);
      }
      delay(100);//Must Delay to sychnorize

      /* Read ack state */
      while (Serial2.available())
      {
      ACK_Flag = Serial2.read();
      }

      if(ACK_Flag != FLASH_PAYLOAD_WRITING_PASSED)
      {
        client.publish("Ali_FromEspToBroker","NACK Send from STM!!");        
      }



      /*To Update address*/
      Address_Counter++;
      CounetrByte = 0;
    }

    }


    string = "";
    file.close();
    Serial.println();
    Serial.println("Ending of Sending File Function");   
    client.publish("Ali_FromEspToBroker","Uploading Finish");      
  }
  else
  {
    /*Do Nothing*/
  }

  digitalWrite(led,LOW);//For debug
}



void Debug_Read_Protection(void)
{
  String Compare_R_WP = "Read_WP";
	String Compare_R_RP = "Read_RP";
	uint8_t Buf;

	while(Serial2.available()>0)
	{
		Buf = Serial2.read();
	}


  if(Compare_R_WP == string)
  {
    
   if(WRITE_IS_ENABLE == Buf)
   {
	  Serial.println("Write protection level is 1");
    client.publish("Ali_FromEspToBroker", "Write protection level is 1");
   }
   else if(WRITE_IS_DISABLE == Buf)
   {
      Serial.println("Write protection level is 0");
      client.publish("Ali_FromEspToBroker", "Write protection level is 0");
   }
   else
   {
    client.publish("Ali_FromEspToBroker", "Error When getting Protection Level!!, Please try again");
   }

  }
  else if(Compare_R_RP == string)
  {
    
   if(OB_RDP_LEVEL_0 == Buf)
   {
	  Serial.println("Read protection level is 0");
    client.publish("Ali_FromEspToBroker", "Read protection level is 0");
   }
   else if(OB_RDP_LEVEL_1 == Buf)
   {
      Serial.println("Read protection level is 1");
      client.publish("Ali_FromEspToBroker", "Read protection level is 1");
   }
   else
   {
    client.publish("Ali_FromEspToBroker", "Error When getting Protection Level!!, Please try again");
   }

  }
  else
  {
  client.publish("Ali_FromEspToBroker", "Error When getting Protection Level!!, Please try again");
  }

	digitalWrite(led,LOW); //for debug
}




void Read_Protection_Level(void)
{
	String Compare_R_WP = "Read_WP";
	String Compare_R_RP = "Read_RP";

	if(string == Compare_R_RP)
	{
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_RDP_STATUS_CMD);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(READ_RP);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		Debug_Read_Protection();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_R_WP)
	{
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_RDP_STATUS_CMD);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(READ_WP);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		Debug_Read_Protection();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		/*Do nothing*/
	}
}






void Debug_WP(void)
{
	/*To Take ack from read protection*/
	char i = 0;
	char Buf[10];

	while(Serial2.available()>0)
	{
		Buf[i] = Serial2.read();
		i++;
	}


	Serial.printf("Change WP to %s", Buf);
	Serial.println();

	if(Buf[6] == '0')
	{
		digitalWrite(led,LOW); //For debug
		client.publish("Ali_FromEspToBroker","Change WP to level 0");
	}
	else  if(Buf[6] == '1')
	{
		digitalWrite(led,LOW); //For debug
		client.publish("Ali_FromEspToBroker","Change WP to level 1");
	}
	else
	{
		digitalWrite(led,LOW); //For debug
		client.publish("Ali_FromEspToBroker","Change WP to level Failled!!, Please try again");
	}

}





void  Change_WP(void)
{
	String Compare_WP_D = "WP_D";
	String Compare_WP_E = "WP_E";

	if(string == Compare_WP_D)
	{
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_WOP_Level_CMD);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(OB_WRPSTATE_DISABLE);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		Debug_WP();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_WP_E)
	{
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_WOP_Level_CMD);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(OB_WRPSTATE_ENABLE);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		Debug_WP();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		/*Do nothing*/
	}
}






void Select_ECU(void)
{
	String ECU_M = "ECU_Master";
	String ECU_S = "ECU_Slave1";

	if(ECU_M == string)
	{
		ECU = MASTERID;
		client.publish("Ali_FromEspToBroker", "Select ECU Master");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else if(ECU_S == string)
	{
		ECU = SLAVE1;
		client.publish("Ali_FromEspToBroker", "Select ECU Slave1");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else
	{
		/*Do nothing*/
	}
}



void Select_App(void)
{
	String Compare_APP1 = "Select_APP1";
	String Compare_APP2 = "Select_APP2";

	if(Compare_APP1 == string)
	{
		App = APPLICATION1;
    Address = FLASH_BASE_ADDRESS + APPLICATION1_OFFEST;
		client.publish("Ali_FromEspToBroker", "Select App1");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else if(Compare_APP2 == string)
	{
		App = APPLICATION2;
    Address = FLASH_BASE_ADDRESS + APPLICATION2_OFFEST;
		client.publish("Ali_FromEspToBroker", "Select App2");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else
	{
		/*Do nothing*/
	}
}




void Debug_MCU_ID(void)
{
	/*To Take ack from read protection*/
	char i =0;
	// Create a character array to hold the formatted result
	char formattedBuf[3]; // Adjust the size as needed
	byte Buf[10];

	while(Serial2.available()>0)
	{
		Buf[i] = Serial2.read();
		i++; //To add . between versions
	}


	// Format the values from buf into the desired string format
	snprintf(formattedBuf, sizeof(formattedBuf), "%d%d", Buf[0], Buf[1], Buf[1]);

	Serial.println(formattedBuf);
	Serial.printf("Version of BL is %s", formattedBuf);
	Serial.println();


	client.publish("Ali_FromEspToBroker", formattedBuf);

	digitalWrite(led,LOW); //for debug
}



void Read_MCU_ID(void)
{
	String Compare_VER = "MCU_ID";

	if(string == Compare_VER)
	{
		Serial2.write(MCUID_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_CID_CMD);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		Debug_MCU_ID();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		/*Do nothing*/
	}

}







void Debug_VER(void)
{
	/*To Take ack from read protection*/
	char i =0;
	// Create a character array to hold the formatted result
	char formattedBuf[15]; // Adjust the size as needed
	byte Buf[4];           //must be uin8_t(byte)

	while(Serial2.available()>0)
	{
		Buf[i] = Serial2.read();
		i++; //To add . between versions
	}


	// Format the values from buf into the desired string format
	snprintf(formattedBuf, sizeof(formattedBuf), "%d.%d.%d.%d", Buf[0], Buf[1], Buf[2], Buf[3]);

	Serial.println(formattedBuf);
	Serial.printf("Version of BL is %s", formattedBuf);
	Serial.println();


	client.publish("Ali_FromEspToBroker", formattedBuf);

	digitalWrite(led,LOW); //for debug
}



void Read_BL_Version(void)
{
	String Compare_VER = "Version";

	if(string == Compare_VER)
	{
		Serial2.write(VER_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_VER_CMD);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(ECU);
		delay(3); //You must put delay to prevent Uart overrun

		digitalWrite(led,HIGH);//For debug

		Debug_VER();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		/*Do nothing*/
	}

}



void Debug_RP(void)
{
	/*To Take ack from read protection*/
	char i =0;
	char Buf[10];

	while(Serial2.available()>0)
	{
		Buf[i] = Serial2.read();
		i++;
	}

	if(i == 8)
	{
		Serial.printf("Change RP to %s", Buf);
		Serial.println();

		if(Buf[6] == '0')
		{
			digitalWrite(led,LOW); //For debug
			client.publish("Ali_FromEspToBroker","Change RP to level 0");
		}
		else  if(Buf[6] == '1')
		{
			digitalWrite(led,LOW); //For debug
			client.publish("Ali_FromEspToBroker","Change RP to level 1");
		}
		else
		{
			digitalWrite(led,LOW); //For debug
			client.publish("Ali_FromEspToBroker","Change RP to level Failed!!, Please try again");
		}

	}
}




/*Change Read Protection*/
/**
Necssary Setups:
1-Using UART0(USB) to receive
2-Using UART2      to send


Algorithm:
1-Receive 'L' -> send 3
2-Receive 'C' -> send CBL_CHANGE_ROP_Level_CMD
3.1- Receve 'D' -> send CBL_ROP_LEVEL_0
3.2 Receve 'E' -> send CBL_ROP_LEVEL_1

Example:
Send on Serail Monitor LCD -> Change RP to Level 0
Send on Serail Monitor LCE -> Change RP to Level 1

 */
void Change_RP(void)
{
#if(SerialMonitor_RP ==   USED)    //Change protection level of STM using Serial Monitor
	char var = 0;
	while(Serial.available()>0)
	{
		var = Serial.read();
		if(var == 'L')
		{
			Serial2.write(RP_LENGTH);
			digitalWrite(led,HIGH);
		}
		else if(var == 'C')
		{
			Serial2.write(CBL_CHANGE_ROP_Level_CMD);
			digitalWrite(led,HIGH);

		}
		else if(var == 'D')
		{
			Serial2.write(CBL_ROP_LEVEL_0);
			digitalWrite(led,LOW);

		}
		else if(var == 'E')
		{
			Serial2.write(CBL_ROP_LEVEL_1);
			digitalWrite(led,LOW);
		}

	}

	Debug_R();

#elif(NodeRed_RP == USED) //Change protection level of STM using Node-Red)
	String Compare_RPL0 = "RP_L0";
	String Compare_RPL1 = "RP_L1";

	if(string == Compare_RPL0)
	{
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_ROP_Level_CMD);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(CBL_ROP_LEVEL_0);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		Debug_RP();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_RPL1)
	{
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_ROP_Level_CMD);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_ROP_LEVEL_1);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		Debug_RP();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		/*Do nothing*/
	}
#endif


}










/****************************************************************************
 *****************************  Main code   *********************************
 ***************************************************************************/
void setup()
{
	/*Setup UART*/
  Init_UART();

	/*Setup GPIO*/
	pinMode(led,OUTPUT);

	/*Setup MQTT*/
  Init_MQTT();

	/*Setup WiFi*/
	SetupWiFi();

  /*Setup Firebase*/
  InitFirebase();
}






void loop() {

	/*Connect to MQTT*/
	MQTT_Connect();

	/*Allow Changing read protection level*/
	Change_RP();

	/*Allow Changing write protection level*/
	Change_WP();

	/*Read protection level*/
	Read_Protection_Level();

	/*Reading version of STM bootloader*/
	Read_BL_Version();

	/*Reading MCU ID of STM*/
	Read_MCU_ID();

	/*Select which ECU to communicate with*/
	Select_ECU();

	/*Select which ECU to communicate with*/
	Select_App();

  /*Write dedicated App to dedicated ECU*/
	Write_Program();



  /*Downloding file from Firebase Storage then reading it and print it on Serial Monitor*/
  DownloadAndReadFile_Firebase();

}
