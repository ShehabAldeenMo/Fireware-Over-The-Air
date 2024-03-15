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









/****************************************************************************
 *****************************  Definitions  *********************************
 *****************************************************************************/
/*Change Read protection Config*/
#define CBL_CHANGE_ROP_Level_CMD                0x17
#define CBL_ROP_LEVEL_0                         0x00
#define CBL_ROP_LEVEL_1                         0x01
#define RP_LENGTH                               0x02
//p[0] ---> length
//p[1] ---> CBL_CHANGE_ROP_Level_CMD
//p[2] ---> CBL_ROP_LEVEL_0 or CBL_ROP_LEVEL_1

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
#define WIFI_SSID         "Ali_Mamdouh"
#define WIFI_PASSWORD     "1234567"
#define MQTT_SERVER       "test.mosquitto.org"

/*Configrations*/
#define USED                0x01
#define NOT_USED            0x00
#define SerialMonitor_RP    NOT_USED     //Change protection level of STM using Serial Monitor
#define NodeRed_RP          USED //Change protection level of STM using Node-Red

/*Config Master and slave ECUS*/
#define MASTERID                                0x00
#define SLAVE1                                  0x01




//Don't orcget to add variable for ECUS and uart transmiot for , and icrease lenhrth

/****************************************************************************
 ***************************  Global Variables  ******************************
 *****************************************************************************/
int led=2; //Led of ESP32
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long LastMsg = 0;
char msg[50];
int value = 0;
char var;
String string; //String used to choose option coming from MQTT broker

uint8_t ECU = 0xFF; //initialize with weong value







/****************************************************************************
 *************************  Functions Definitions  ***************************
 *****************************************************************************/


/*******************************************************************************
 *                              1-MQTT functions                               *
********************************************************************************/
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
		//MessageInfo(Local_Message);
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





/*******************************************************************************
 *                              2-WiFi functions                                *
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





/*******************************************************************************
 *                             3-UART functions                                 *
 ********************************************************************************/
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
 *                             4-App functions                                 *
********************************************************************************/
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
	snprintf(formattedBuf, sizeof(formattedBuf), "%d%d", Buf[0], Buf[1]);

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
	Serial.begin(115200);
	Serial2.begin(115200);

	Serial.setRxBufferSize(1024); //To increase Buffer of Arduinio serial RX
	Serial.setTxBufferSize(300); //To increase Buffer of Arduinio serial TX

	Serial2.setRxBufferSize(1024); //To increase Buffer of Arduinio serial RX
	Serial2.setTxBufferSize(300); //To increase Buffer of Arduinio serial TX

	/*Setup GPIO*/
	pinMode(led,OUTPUT);

	/*Setup MQTT*/
	client.setServer(MQTT_SERVER, 1883);
	client.setCallback(MQTT_Callback);

	/*Setup WiFi*/
	SetupWiFi();
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

}
