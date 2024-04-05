
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
#define API_KEY "AIzaSyCnu0Gv4eHBS3YngCYb6wSKASRG-p9XUPY"

/*Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "shehabmohammed2002@gmail.com" 
#define USER_PASSWORD "123456" 

/*Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "fota-70b84.appspot.com"

/*Change Read protection Config*/
#define CBL_ROP_LEVEL_0                      0x00
#define CBL_ROP_LEVEL_1                      0x01
#define RP_LENGTH                            0x03

#define CBL_GET_VER_CMD                      0x10
#define CBL_GET_RDP_STATUS_CMD               0x11 /* Read the flash protection level command */
#define JUMP_TO_APPLICATION_CMD              0x12
#define CBL_FLASH_ERASE_CMD                  0x13 
#define CBL_MEM_WRITE_CMD                    0x14
#define CBL_CHANGE_ROP_Level_CMD             0x15
#define CBL_CHANGE_WOP_Level_CMD             0x16


/*Read read protection*/
#define OB_RDP_LEVEL_0                       ((uint8_t)0xA5)
#define OB_RDP_LEVEL_1                       ((uint8_t)0x00)
#define READ_RP                              0x00
#define READ_WP                              0x01
#define WRITE_IS_ENABLE                      0x00 //This Mask is only for low densties to check if All the 32 pages are write protected(it should changed for higher densties STM)
#define WRITE_IS_DISABLE                     0xFF //This Mask is only for low densties to check if All the 32 pages are write protected(it should changed for higher densties STM)

/*Change Write protection Config*/
#define OB_WRPSTATE_DISABLE                     0x00
#define OB_WRPSTATE_ENABLE                      0x01
#define WP_LENGTH                               0x03

/*Read version Config*/
#define VER_LENGTH                              0x02

/*MCU ID config*/
#define MCUID_LENGTH                            0x02

/*Define the WiFi credentials */
#define WIFI_SSID         "NewSignal"                      /* WiFi Connection Network*/
#define WIFI_PASSWORD     "Encrpted#753951456"         /* WiFi Password */
#define MQTT_SERVER       "test.mosquitto.org"

/*Configrations*/
#define USED                0x01
#define NOT_USED            0x00

/*Config Master and slave ECUS*/
#define MASTERID                                0x00
#define SLAVE1                                  0x01

/*Write Program definitions*/
#define TWOBYTES                          16       
#define MAX_MEM_WRITE_TOT_Length          72
#define MAX_MEM_WRITE_DATA_Length         64
#define ADDRESS_M_WRITE_INDEX             2 /* Index of address in packet from esp to stm */
#define ECU_M_WRITE_INDEX                 6 /* Index of ECU in packet from esp to stm */
#define WRITE_M_APP_INDEX                 7 /* Index of APP in packet from esp to stm */
#define WRITE_M_DATA_INDEX                8 /* Index of DATA length in packet from esp to stm */
#define FLASH_PAYLOAD_WRITING_FAILED      0x00
#define FLASH_PAYLOAD_WRITING_PASSED      0x01

/*Erase Memory definitions*/
#define FLASH_ERASE_LENGTH                      7
#define FLASH_ERASE_PACKET_LENGTH               8
#define FLASH_ERASE_ADDRESS_INDEX               2
#define MQTT_ERASE_ID                           230
#define PAGE_ID_LOWER_LIMIT                     0
#define PAGE_AMOUNT_LOWER_LIMIT                 1
#define PAGE_UPPER_LIMIT                        127
#define SUCESSFUL_ERASE                         0x03
#define UNSUCESSFUL_ERASE                       0x02

/*APP definitions*/
#define FLASH_BASE_ADDRESS            0x08000000   /* Flash base address of our micro-controller */
#define APPLICATION1_OFFEST           0x5000       /* Application 1 base address */
#define APPLICATION2_OFFEST           0xA000           /* Application 2 base address */
#define APPLICATION1                  1
#define APPLICATION2                  2
#define MQTT_MESSAGE_RESET_VALUE      0xFFFFFFFF
#define JUMP_TO_APP_LENGTH            6
#define JUMP_TO_APP_PACKET_LENGTH     0x07
#define BL_BEGIN_MATCH                0xAA
#define BL_BEGIN_NOT_MATCH            0xEE
#define RESET_ECU                     0xCC



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
int MQTT_Message = 0 ; //Using it to select which funcion to execute from the command coming from MQTT callback(Node-Red)


/*ECUs variables*/
uint8_t ECU = MASTERID; //initialize with wrong value

/*Apps variables*/
char App = APPLICATION1;//Default value App1
int  Address = FLASH_BASE_ADDRESS + APPLICATION1_OFFEST; //Default value App1
uint8_t Global_Erase_Counter = 0; //Counter that dedicate which stage to execute in Erase function(Choosing Erase, Choosing Page_ID, Choosing Page_Amount) 
uint8_t Page_ID;
uint8_t Page_Amount;
String Compare_RPL0 = "RP_L0";
String Compare_RPL1 = "RP_L1";
String Compare_VER = "Version";
String Compare_MCUID = "MCU_ID";
String Compare_APP1 = "Select_APP1";
String Compare_APP2 = "Select_APP2";
String ECU_M = "ECU_Master";
String ECU_S = "ECU_Slave1";
String Compare_WP_D = "WP_D";
String Compare_WP_E = "WP_E";
String Compare_R_WP = "Read_WP";
String Compare_R_RP = "Read_RP";
String Compare_Write_Program = "Write_Program";
String Compare_Jump_To_App = "Jump_App";
String Compare_Jump_To_BL = "Jump_BL";
String Compare_Reset_ECU = "Reset_ECU";

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
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");

	for (int i = 0; i < length; i++) {
		string+=((char)payload[i]);
	}

	if (topic ="FromBrokerToESP")
	{
		Serial.println(string);
		MQTT_Message = string.toInt();
		Serial.printf("%d", MQTT_Message);
		Serial.println();
		delay(15);

		/*To enter Amount of pages without bugs*/
		if(Global_Erase_Counter == 2)
		{
			Global_Erase_Counter = 3; //There must be dependence on MQTT callback function(otherwise it will take same value of PageID)
		}

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
			client.subscribe("FromBrokerToESP");
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

void Get_Erase_Config(void)
{
	if((MQTT_Message == MQTT_ERASE_ID) && (Global_Erase_Counter == 0))
	{ 
		client.publish("FromEspToBroker","Please Enter Page ID to Erase");
		Global_Erase_Counter = 1;//Global_Erase_Counter must = 1 after entering, to avoid entering condition infinite times
		string = ""; //To clear message for the other configrations(If not bugs appear)
	} 
	else if((MQTT_Message >= PAGE_ID_LOWER_LIMIT) && (MQTT_Message <= PAGE_UPPER_LIMIT) && (Global_Erase_Counter == 1))
	{
		digitalWrite(led,HIGH);//For debug
		delay(100);

		client.publish("FromEspToBroker","Please Enter Page Amount to Erase");		
		Page_ID = MQTT_Message;
		Global_Erase_Counter = 2;

		digitalWrite(led,LOW);//For debug
		string = ""; //To clear message for the other configrations(If not bugs appear)
	}
	else if((MQTT_Message >= PAGE_AMOUNT_LOWER_LIMIT) && (MQTT_Message <= PAGE_UPPER_LIMIT) && (Global_Erase_Counter == 3))
	{
		digitalWrite(led,HIGH);//For debug
		delay(100);

		Page_Amount = MQTT_Message;
		Global_Erase_Counter = 4;

		digitalWrite(led,LOW);//For debug
		string = ""; //To clear message for the other configrations(If not bugs appear)
	}

}






void Ack_Erase(void)
{
	/*Creating Ack variable*/
	uint8_t Ack;

	/*Reading Ack value from STM ECU*/
	while(Serial2.available()>0)
	{
		Ack = Serial2.read();
	}

	/*Print Ack value on serial monitor to ease debugging*/
	Serial.printf("Ack Erase value is %x", Ack);
	Serial.println();

	/*Check if erase operation is succefuly completed or not*/
	if(SUCESSFUL_ERASE  == Ack)
	{
		/*Serial monitor Ack*/
		Serial.printf("Erase operation successfuly completed!!");
		Serial.println();

		/*Node-Red Ack*/
		client.publish("FromEspToBroker","Erase operation successfuly completed!!");
	}
	else
	{
		/*Serial monitor Nack*/
		Serial.printf("Erase operation Failed!!, Please try again");
		Serial.println();

		/*Node-Red Nack*/
		client.publish("FromEspToBroker","Erase operation Failed!!, Please try again");
	}

}






void Erase_Memory(void)
{
	/*To get Erase configtations(Which page to start to erase and the amount of pages to erase from that page)*/
	Get_Erase_Config();

	if(Global_Erase_Counter == 4) //When Global_Erase_Counter = 3, This means all configrations are made.
	{
		/*Notify user beginning of Erase process */
		digitalWrite(led,HIGH);//For debug
		client.publish("FromEspToBroker","Erasing From ECU..");

		/*Erase Packet formation */
		uint8_t Packet[FLASH_ERASE_PACKET_LENGTH] = {0};
		Packet[0] = FLASH_ERASE_LENGTH;
		Packet[1] = CBL_FLASH_ERASE_CMD;
		*(int*)(&Packet[FLASH_ERASE_ADDRESS_INDEX]) = FLASH_BASE_ADDRESS+1024*Page_ID ;
		Packet[6] = ECU;
		Packet[7] = Page_Amount;

		/* Packet transmitting */
		Serial2.write(Packet[0]);//Send first the length of Packet
		char i;
		delay(1);//Must Delay to sychnorize
		for (i = 1; i < FLASH_ERASE_PACKET_LENGTH; i++)
		{
			Serial2.write(Packet[i]);
		} 
		delay(1000);//Must Delay to sychnorize

		/* Taking Ack from STM ECU */
		Ack_Erase();

		/* Re-initialize global variables in Erase function after finishing Erasing  */	
		Global_Erase_Counter = 0;
		MQTT_Message = MQTT_MESSAGE_RESET_VALUE; //To Reset MQTT_Message variable.
		digitalWrite(led,LOW);//For debug	

	}
}


void Write_Program(void)
{
	digitalWrite(led,HIGH);//For debug
	client.publish("FromEspToBroker","Sending APP to ECU..");
	char CounetrByte = 0 ;
	char ACK_Flag = FLASH_PAYLOAD_WRITING_FAILED;
	char HexData = 0 ;
	int Address_Counter = 0;
	int Remaining_Size;

	/*Write Program Packet formation */
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
		client.publish("FromEspToBroker","Failed to open file for reading!!");
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
				client.publish("FromEspToBroker","NACK Send from STM!!");        
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
			while ( CounetrByte < Remaining_Size)
			{
				file.readBytes(Buf,2);
				HexData = strtol(Buf, NULL, TWOBYTES);
				Packet[CounetrByte+9] = HexData ;
				Serial.printf("\nPacket[%i]=%x",CounetrByte+9,Packet[CounetrByte+9]);
				CounetrByte++; 
			}

			for (char i = 0; i < Remaining_Size + 9; i++)
			{
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
				client.publish("FromEspToBroker","NACK Send from STM!!");        
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
	client.publish("FromEspToBroker","Uploading Finish");      

	digitalWrite(led,LOW);//For debug
}



void Ack_Read_Protection(void)
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
			client.publish("FromEspToBroker", "Write protection level is 1");
		}
		else if(WRITE_IS_DISABLE == Buf)
		{
			Serial.println("Write protection level is 0");
			client.publish("FromEspToBroker", "Write protection level is 0");
		}
		else
		{
			client.publish("FromEspToBroker", "Error When getting Protection Level!!, Please try again");
		}

	}
	else if(Compare_R_RP == string)
	{

		if(OB_RDP_LEVEL_0 == Buf)
		{
			Serial.println("Read protection level is 0");
			client.publish("FromEspToBroker", "Read protection level is 0");
		}
		else if(OB_RDP_LEVEL_1 == Buf)
		{
			Serial.println("Read protection level is 1");
			client.publish("FromEspToBroker", "Read protection level is 1");
		}
		else
		{
			client.publish("FromEspToBroker", "Error When getting Protection Level!!, Please try again");
		}

	}
	else
	{
		client.publish("FromEspToBroker", "Error When getting Protection Level!!, Please try again");
	}

	digitalWrite(led,LOW); //for debug
}




void Read_Protection_Level(void)
{
	if(string == Compare_R_RP)
	{
		/*Packet transmiting */
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_RDP_STATUS_CMD);
		delay(3);  //You must put delay to prevent Uart overrun
    Serial2.write(ECU);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(READ_RP);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		/*Read protection Ack */
		Ack_Read_Protection();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_R_WP)
	{
		/*Packet transmiting */
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_GET_RDP_STATUS_CMD);
    delay(3);  //You must put delay to prevent Uart overrun
    Serial2.write(ECU);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(READ_WP);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		/*Read protection Ack */
		Ack_Read_Protection();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		client.publish("FromEspToBroker", "Error When getting Protection Level!!, Please try again");
	}
}






void Ack_WP(void)
{
	/*To Take ack from read protection*/
	char Buf;

	while(Serial2.available()>0)
	{
		Buf= Serial2.read();
	}

	if(Buf == 0x00)
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","WP haven't been changed");
	}
	else if (Buf == 0x01)
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","WP have been changed");
	}
	else
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","Change WP to level Failled!!, Please try again");
	}

}





void  Change_WP(void)
{
	if(string == Compare_WP_D)
	{
		/*Packet transmiting */
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_WOP_Level_CMD);
    delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(ECU);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(OB_WRPSTATE_DISABLE);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		/*Write protection Ack */
		Ack_WP();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_WP_E)
	{
		/*Packet transmiting */
		Serial2.write(WP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_WOP_Level_CMD);
    delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(ECU);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(OB_WRPSTATE_ENABLE);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		/*Write protection Ack */
		Ack_WP();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		client.publish("FromEspToBroker","Change WP to level Failled!!, Please try again");
	}
}






void Select_ECU(void)
{
	if(ECU_M == string)
	{
		ECU = MASTERID;
		client.publish("FromEspToBroker", "Select ECU Master");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else if(ECU_S == string)
	{
		ECU = SLAVE1;
		client.publish("FromEspToBroker", "Select ECU Slave1");
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
	if(Compare_APP1 == string)
	{
		App = APPLICATION1;
		Address = FLASH_BASE_ADDRESS + APPLICATION1_OFFEST;
		client.publish("FromEspToBroker", "Select App1");
		digitalWrite(led,HIGH); //for debug
		delay(50);
		digitalWrite(led,LOW); //for debug
		string = "";
	}
	else if(Compare_APP2 == string)
	{
		App = APPLICATION2;
		Address = FLASH_BASE_ADDRESS + APPLICATION2_OFFEST;
		client.publish("FromEspToBroker", "Select App2");
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


void Ack_VER(void)
{
	/*To Take ack from read protection*/
	char i =0;
	// Create a character array to hold the formatted result
	char formattedBuf[15]; // Adjust the size as needed
	byte Buf[4];           //must be uint8_t(byte)

	while(Serial2.available()>0)
	{
		Buf[i] = Serial2.read();
		i++; //To add . between versions
	}

  if ( Buf[0] == 0xAA || Buf[1] == 0xAA ){
    client.publish("FromEspToBroker", "Error in get version function");
    return ;
  }

	// Format the values from buf into the desired string format
	snprintf(formattedBuf, sizeof(formattedBuf), "%d.%d.%d.%d", Buf[0], Buf[1], Buf[2], Buf[3]);

	Serial.println(formattedBuf);
	Serial.printf("Version of BL is %s", formattedBuf);
	Serial.println();


	client.publish("FromEspToBroker", formattedBuf);

	digitalWrite(led,LOW); //for debug
}



void Read_BL_Version(void)
{
	Serial2.write(VER_LENGTH);
	delay(3); //You must put delay to prevent Uart overrun
	Serial2.write(CBL_GET_VER_CMD);
	delay(3); //You must put delay to prevent Uart overrun
	Serial2.write(ECU);
	delay(3); //You must put delay to prevent Uart overrun

	digitalWrite(led,HIGH);//For debug

	Ack_VER();

	string = ""; //Clear string to avoid multiple entering
}







void Ack_RP(void)
{
	/*To Take ack from read protection*/
	char Buf;

	while(Serial2.available()>0)
	{
		Buf= Serial2.read();
	}

	if(Buf == 0x00)
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","RP haven't been changed");
	}
	else if ( Buf == 0x01)
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","RP have been changed");
	}
	else
	{
		digitalWrite(led,LOW); //For debug
		client.publish("FromEspToBroker","Change RP to level Failled!!, Please try again");
	}
}


void Change_RP(void)
{
	if(string == Compare_RPL0)
	{
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_ROP_Level_CMD);
		delay(3);  //You must put delay to prevent Uart overrun
    Serial2.write(ECU);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(CBL_ROP_LEVEL_0);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH);//For debug

		Ack_RP();

		string = ""; //Clear string to avoid multiple entering
	}
	else if(string == Compare_RPL1)
	{
		Serial2.write(RP_LENGTH);
		delay(3); //You must put delay to prevent Uart overrun
		Serial2.write(CBL_CHANGE_ROP_Level_CMD);
		delay(3); //You must put delay to prevent Uart overrun
    Serial2.write(ECU);
		delay(3);  //You must put delay to prevent Uart overrun
		Serial2.write(CBL_ROP_LEVEL_1);
		delay(3); //You must put delay to prevent Uart overrun
		digitalWrite(led,HIGH); //For debug

		Ack_RP();

		string = ""; //Clear string to avoid multiple entering
	}
	else
	{
		client.publish("FromEspToBroker","Change RP to level Failed!!, Please try again");
	}
}


void Jump_To_App(void)
{
	uint8_t Packet[JUMP_TO_APP_PACKET_LENGTH];
	char i;

	/*Notify user beginning of Jumping to App process */
	digitalWrite(led,HIGH);//For debug
	client.publish("FromEspToBroker","Jumping to App..");

	/*Jumping to App Packet formation */
	Packet[0] = JUMP_TO_APP_LENGTH;
	Packet[1] = JUMP_TO_APPLICATION_CMD;
	*(int*)(&Packet[2]) = Address;
	Packet[6] = ECU;

	/*Packet transmitting */
	Serial2.write(Packet[0]);//Send first the length of Packet
	delay(1);//Must Delay to sychnorize

	for (i = 1; i < JUMP_TO_APP_PACKET_LENGTH; i++)
	{
    Serial.printf("\nPacket[%i]=%x",i,Packet[i]);
		Serial2.write(Packet[i]);
	} 

	/*Reinitializing to prevent continous logging*/
	string = "";
}


void Jump_To_BL(void)
{
  /*Receive Ack*/
  uint8_t Ack = 0;

  /*Notify user beginning of Jumping to App process */
	digitalWrite(led,HIGH);//For debug
	client.publish("FromEspToBroker","Jumping to Bootloader..");

	/*Packet transmitting to notify App to jump to bootloader*/
	Serial2.write(BL_BEGIN_MATCH);
  delay(10);//To avoid bugs if Uart of STM send data after ESP read it
	
  while(Serial2.available()>0)
	{
		Ack = Serial2.read();
	}

  /*Notify Node-red about the status of transmiting*/
  if(Ack == BL_BEGIN_MATCH)
  {
    client.publish("FromEspToBroker", "Receive Ack from BL jump function in STM");
  }
  else if(Ack == BL_BEGIN_NOT_MATCH)
  {
    client.publish("FromEspToBroker", "Receive Nack from BL jump function in STM!!");
  }

  /*Reinitializing to prevent continous logging*/
	string = "";
	digitalWrite(led,LOW);//For debug	
}

void Reset_Function (){
	digitalWrite(led,HIGH);//For debug
	client.publish("FromEspToBroker","Reset ECU");

	/*Packet transmitting to notify App to jump to bootloader*/
	Serial2.write(RESET_ECU);
  delay(10);//To avoid bugs if Uart of STM send data after ESP read it

  /*Reinitializing to prevent continous logging*/
	string = "";
	digitalWrite(led,LOW);//For debug	
}

void Select_Function(void)
{
	if((string == Compare_RPL0) || (string == Compare_RPL1))
	{
		/*Allow Changing read protection level*/
		Change_RP();
	}
  else if((string == Compare_WP_D) || (string == Compare_WP_E))
	{
		/*Allow Changing write protection level*/
		Change_WP();
	}
	else if(string == Compare_VER)
	{
		/*Reading version of STM bootloader*/
		Read_BL_Version();
	}
  else if((string == Compare_R_WP) || (string == Compare_R_RP))
	{
		/*Read protection level*/
		Read_Protection_Level();
	}
  else if(string == Compare_Write_Program)
	{
		/*Write dedicated App to dedicated ECU*/
		Write_Program();
	}
	else if((string == Compare_APP1) || (string == Compare_APP2))
	{
		/*Select which ECU to communicate with*/
		Select_App();
	}
	else if((string == ECU_M) || (string == ECU_S))
	{
		/*Select which ECU to communicate with*/
		Select_ECU();
	}
	else if(string == Compare_Jump_To_App)
	{
		Jump_To_App();
	}
  else if(string == Compare_Jump_To_BL)
  {
    Jump_To_BL();
  }
  else if (string == Compare_Reset_ECU){
    Reset_Function();
  }
	else
	{
		/*Erase dedicated Amount of pages beginning from selected Page_ID*/ 
		Erase_Memory();
	}
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

	/*Downloding file from Firebase Storage then reading it and print it on Serial Monitor*/
	DownloadAndReadFile_Firebase();

}



void loop() 
{
	/*Connect to MQTT*/
	MQTT_Connect();

	/*Select bootloader function to execute*/
	Select_Function();
}
