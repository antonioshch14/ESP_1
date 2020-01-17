//ESP connection to send data to local ESP server

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "DataTransfer";
const char* password = "BelovSer";//"DataTransfer", "Belov"
const String  Devicename = "4";//device name for ESP at bathroom
char jsonBuffer[2000] = "["; // Initialize the jsonBuffer to hold data

  // WIFI Module Role & Port
IPAddress     TCP_Server(192, 168, 4, 1);
IPAddress     TCP_Gateway(192, 168, 4, 1);
IPAddress     TCP_Subnet(255, 255, 255, 0);
IPAddress Own(192, 168, 4, 102);
unsigned int  TCPPort = 2390;

WiFiClient    TCP_Client;


/* Collect data once every 15 seconds and post data to ThingSpeak channel once every 2 minutes */
// unsigned long lastConnectionTime = 0; // Track the last connection time
unsigned long lastUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 5L * 1000L; // s
// const unsigned long updateInterval = 15L * 1000L; // Update once every 15 seconds
int const maxRow=10; //for data read from serial
char getData[maxRow][9][6]; //for data read from serial
int row; //for data read from serial


void setup() {
	Serial.begin(115200);
	WiFi.hostname("ESP_Bathroom");      // DHCP Hostname (useful for finding device for static lease)
	WiFi.config(Own, TCP_Gateway, TCP_Subnet);
	Check_WiFi_and_Connect_or_Reconnect();          // Checking For Connection
	//WiFiClient::setLocalPortStart(2391);
}
void loop() {
    if (millis() - lastUpdateTime >=  postingInterval) 
    { 
    readDataSerial(row);
    if (row>0) { Send_Request_To_Server();row=0; memset(getData,0,sizeof(getData));}
    }
  }
    
void Send_Request_To_Server() {
	unsigned long tNow;
	delay(1000);
	tNow = millis();
	if (TCP_Client.connected()) {
		TCP_Client.setNoDelay(1);
		String dataToSend = "Device:" + Devicename + ";" + "time:" + String(tNow) + ";" +
			"signal:" + String(WiFi.RSSI()) + "temp:" + getData[0][0] + ";"
			+ "humid" + getData[0][1] + ";" + "humidAv:" + getData[0][2] + ";" +
			"status:" + getData[0][3] + ";" + ";";
		TCP_Client.println(dataToSend);
		Serial.print("- data stream: ");	Serial.println(dataToSend);//Send sensor data


	}
	
}

//====================================================================================

void Check_WiFi_and_Connect_or_Reconnect() {
	if (WiFi.status() != WL_CONNECTED) {

		TCP_Client.stop();                                  //Make Sure Everything Is Reset
		WiFi.disconnect();
		Serial.println("Not Connected...trying to connect...");
		delay(50);
		WiFi.mode(WIFI_STA);                                // station (Client) Only - to avoid broadcasting an SSID ??
		//WiFi.begin(ssid, password,0);                    // the SSID that we want to connect to
		WiFi.begin(ssid, password);
		Serial.println("ssid: " + String(ssid) + "  password:" + String(password));
		for (int i = 0; i < 10; ++i) {
			if (WiFi.status() != WL_CONNECTED) {
				delay(500);
				Serial.print(".");
			}
			else {

				// Printing IP Address --------------------------------------------------
				Serial.println("Connected To      : " + String(WiFi.SSID()));
				Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
				Serial.print("Server IP Address : ");
				Serial.println(TCP_Server);
				Serial.print("Device IP Address : ");
				Serial.println(WiFi.localIP());

				// conecting as a client -------------------------------------
				Tell_Server_we_are_there();
				break;
			}

		}
	}
}

//====================================================================================

void Tell_Server_we_are_there() {
	// first make sure you got disconnected
	//TCP_Client.stop();

	// if sucessfully connected send connection message
	if (TCP_Client.connect(TCP_Server, TCPPort)) {
		Serial.println("<" + Devicename + "-CONNECTED>");
		TCP_Client.println("<" + Devicename + "-CONNECTED>");
	}
	TCP_Client.setNoDelay(1);                                     // allow fast communication?
}

//====================================================================================

void loop() {
	//WiFiClient::setLocalPortStart(2391);
	Check_WiFi_and_Connect_or_Reconnect();          // Checking For Connection
	Send_Request_To_Server();


	yield();
	//while (tNow2 > (millis() - 50)) {
	if (TCP_Client.connected()) {

		if (TCP_Client.available() > 0) {                     // Check For Reply
			String line = TCP_Client.readStringUntil('\r');     // if '\r' is found
			Serial.print("received: ");                         // print the content
			Serial.println(line);
			
		}
	}
	else Tell_Server_we_are_there();
	//	}
}

  
  //jsonBuffer[0] = '['; //Reinitialize the jsonBuffer for next batch of data
  //jsonBuffer[1] = '\0';
 

void readDataSerial(int& rowRead)
{ if (Serial.available()){
  int column; 
  int entry;
  bool start=false;
  char ch;
  rowRead=0;
  int i=0;
  while (Serial.available()){
    ch = Serial.read();
	yield();
   // delay (2);
    if(ch=='{') {start=true;column=0;entry=0;} 
    else if (start==true)
    {
      if (ch >= '0' && ch <= '9' ) 
        {
         getData[rowRead][column][entry]=ch;  entry++;
        }
      else if (ch == '-' ) 
        {
        getData[rowRead][column][entry]=ch;  entry++;
         }
      else if ( ch == '.' ) 
        {
       getData[rowRead][column][entry]=ch;  entry++;
       }
      else if ( ch==',') {column++;entry=0;}
      else if (ch=='}') {delay(2);start=false; rowRead++;}
        }
    
  }
}
}
