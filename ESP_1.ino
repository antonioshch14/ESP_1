//ESP connection to send data to local ESP server

#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h> 
#include <FS.h>
//#define LOG(x) Serial.println(x);
#define LOG(x);
#define TCPPort_Web_Client 80
#define TCPPort_Websocket 81
#define  TCPPort 2390
const char* ssid = "DataTransfer";
const char* password = "BelovSer";//"DataTransfer", "Belov"
const String  Devicename = "4";//device name for ESP at bathroom
const char* web_ssid = "Bathroom";
const char* web_password = "Belov_628";
IPAddress WebInt(192, 168, 5, 1);
IPAddress WebGetaway(192, 168, 5, 1);
IPAddress WebSubnet(255, 255, 255, 0);
IPAddress     TCP_Server(192, 168, 4, 1);
IPAddress     TCP_Gateway(192, 168, 4, 1);
IPAddress     TCP_Subnet(255, 255, 255, 0);
IPAddress Own(192, 168, 4, 104);

WiFiClient    TCP_Client;
WebSocketsServer webSocket = WebSocketsServer(TCPPort_Websocket);
WiFiServer  TCP_SERVER(TCPPort);
ESP8266WebServer server(TCPPort_Web_Client);
struct datatFromUno {
	float temp;
	float humid;
	int humidAwer;
	byte state;

	void parseInput(String data) {
		LOG(data)
		webSocket.broadcastTXT(data);
		int beginning = data.indexOf('{') + 1;
		int fullEnd = data.indexOf('}');
		if (beginning != 0 && fullEnd != 0) {
			int currentEnd = data.indexOf(',', beginning);
			String field = data.substring(beginning, currentEnd);
			temp = field.toFloat();
			beginning = currentEnd + 1;
			currentEnd = data.indexOf(',', beginning);
			field = data.substring(beginning, currentEnd);
			humid = field.toFloat();
			beginning = currentEnd + 1;
			currentEnd = data.indexOf(',', beginning);
			field = data.substring(beginning, currentEnd);
			humidAwer = field.toInt();
			beginning = currentEnd + 1;
			field = data.substring(beginning, fullEnd);
			state = getByte(field);
			/*Serial.println(dataUNO.state, BIN);
			if (bitRead(dataUNO.state, 0))LOG("deodorantActivated=true")
			else LOG("deodorantActivated=false")
			if (bitRead(dataUNO.state, 1))LOG("humanBodyDeteckted=true")
			else LOG("humanBodyDeteckted=false")
			if (bitRead(dataUNO.state, 2))LOG("buttonStart=true")
			else LOG("buttonStart=false")
			if (bitRead(dataUNO.state, 3))LOG("fanWork=true")
			else LOG("fanWork=false")
			if (bitRead(dataUNO.state, 4))LOG("webStart=true")
			else LOG("webStart=false")
			if (bitRead(dataUNO.state, 5))LOG("humidStart=true")
			else LOG("humidStart=false")
			if (bitRead(dataUNO.state, 6))LOG("coutnFull=true")
			else LOG("coutnFull=false")
			if (bitRead(dataUNO.state, 7))LOG("light=true")
			else LOG("light=false")*/
		}
	}
	private:
	byte nibble(char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';

		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;

		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 0;  // Not a valid hexadecimal character
	}
	byte getByte(String data) {
		byte state = B00000000;
		if (data.length() > 1) {
			state = nibble(data[0]) << 4;
			state |= nibble(data[1]);
		}
		else state |= nibble(data[0]);
		return state;
	}
};
datatFromUno dataUNO;
class task {
public:
	unsigned long period;
	bool ignor = false;
	void reLoop() {
		taskLoop = millis();
	};
	bool check() {
		if (!ignor) {
			if (millis() - taskLoop > period) {
				taskLoop = millis();
				return true;
			}
		}
		return false;
	}
	void StartLoop(unsigned long shift) {
		taskLoop = millis() + shift;
	}
	task(unsigned long t) {
		period = t;
	}
private:
	unsigned long taskLoop;

};
task sendRequestToServer(20000);
task task_checkConnectionToServer(20000);
task sendLogToSrver(10000);
task task_checkClient(5000);
task task_askHumidFromServer(10000);
String fieldsInLogMes = "Device:4;get:3;Time general,Time device,Signal,Temp,Humid, HumidAver,Status;";

bool connectionTried = false;
bool  connectionEstablished = false;
unsigned long timeConnectioTried, timeAttemptToConnect, checkconnection;

/* Collect data once every 15 seconds and post data to ThingSpeak channel once every 2 minutes */
// unsigned long lastConnectionTime = 0; // Track the last connection time
unsigned long lastUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 5L * 1000L; // s
// const unsigned long updateInterval = 15L * 1000L; // Update once every 15 seconds
#include "web&file_setup.h"

void setup() {
	Serial.begin(9600);
	Serial.println("Begin");
	//WiFi.hostname("ESP_Bathroom");      // DHCP Hostname (useful for finding device for static lease)
	//WiFi.config(Own, TCP_Gateway, TCP_Subnet);
	WiFi.mode(WIFI_AP_STA);//WIFI_STA WIFI_AP_STA 
	setupWeb();
	setupClient();
	//server.on("/", HTTP_GET, handleFileRead);
	server.onNotFound([]() {                              // If the client requests any URI
		if (!handleFileRead(server.uri()))                  // send it if it exists
			server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
	});
	server.begin();                           // Actually start the server
	webSocket.begin();                          // start the websocket server
	startSPIFFS();
	webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
}
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length)
{
	if (length) {
		//char message[16];
		//message[0] = (char)payload[0];
		String messageSt;
		for (int i = 0; i < length; i++) messageSt += (char)payload[i];
		LOG("websocket got:" + messageSt);
		Serial.println(messageSt);
		//delete[] messChar;
		/*if (type == WStype_TEXT)
		{
			if (payload[0] == '2') webSocket.broadcastTXT("F1_OFF");
			else if (payload[0] == '3') webSocket.broadcastTXT("F1_ON");
			else if (payload[0] == '4') webSocket.broadcastTXT("F2_OFF");
			else if (payload[0] == '5') webSocket.broadcastTXT("F2_ON");
			else LOG("Socket message isn't recognised");

		}

		else
		{
			Serial.print("WStype = ");   Serial.println(type);
			Serial.print("WS payload = ");
			for (int i = 0; i < length; i++) { Serial.print((char)payload[i]); }
			Serial.println();

		}*/
	}
}
    
void Send_Request_To_Server() {
	unsigned long tNow;
	tNow = millis();
	if (TCP_Client.connected()) {
		TCP_Client.setNoDelay(1);
		String dataToSend = "Device:" + Devicename + ";" + "time:" + String(tNow) + ";" +
			"signal:" + String(WiFi.RSSI()) + ";" + "temp:" + dataUNO.temp + ";"
			+ "humid:" + dataUNO.humid + ";" + "humidAv:" + dataUNO.humidAwer + ";" +
			"status:" + String(dataUNO.state,HEX) + ";" ;
		TCP_Client.println(dataToSend);
		LOG(dataToSend)
	}	
}
void Send_Log_To_Server() {
	if (TCP_Client.connected()) {
		TCP_Client.setNoDelay(1);
		String strToSend = "Device:" + Devicename + ";get:2;" + String(millis()) + "," +
			String(WiFi.RSSI()) + "," + dataUNO.temp + ","
			+ dataUNO.humid + "," + dataUNO.humidAwer + "," + String(dataUNO.state, HEX) + ";";
		TCP_Client.println(strToSend);
		webSocket.broadcastTXT(strToSend);
		LOG(strToSend)

	}
}


void loop() {

	
	if (task_checkClient.check()) {
		if (TCP_SERVER.hasClient()) {
			WiFiClient Temp = TCP_SERVER.available();
			IPAddress IP = Temp.remoteIP();
			LOG("Conneted client ");
			LOG(IP);
			task_checkClient.ignor = true;
		}
	}
	//while (tNow2 > (millis() - 50)) {
	if (TCP_Client.connected()) {

		if (TCP_Client.available() > 0) {                     // Check For Reply
			String line = TCP_Client.readStringUntil('\r');     // if '\r' is found
			//Serial.print("received: ");                         // print the content
			if (line.indexOf("humid:") != -1) task_askHumidFromServer.period = 60000;
			Serial.println(line);

		}
	}
	else if (task_checkConnectionToServer.check()) setupClient();
	if (Serial.available()) dataUNO.parseInput(Serial.readStringUntil('\r'));
	if (sendRequestToServer.check()) Send_Request_To_Server();
	if (sendLogToSrver.check()) Send_Log_To_Server();
	if (task_askHumidFromServer.check()) {
		if(!bitRead(dataUNO.state,7)) 
			if (TCP_Client.connected()) TCP_Client.println("Device:" + Devicename + ";get:5;");
	}
	server.handleClient();                    // Listen for HTTP requests from clients
	webSocket.loop();
	yield();
}
/*bool get_field_value(String Message, String field, unsigned long* value, int* index) {
	int fieldBegin = Message.indexOf(field) + field.length();
	int check_field = Message.indexOf(field);
	int ii = 0;
	*value = 0;
	*index = 0;
	bool indFloat = false;
	if (check_field != -1) {
		int filedEnd = Message.indexOf(';', fieldBegin);
		if (filedEnd == -1) { return false; }
		int i = 1;
		char ch = Message[filedEnd - i];
		while (ch != ' ' && ch != ':') {
			if (isDigit(ch)) {
				int val = ch - 48;
				if (!indFloat)ii = i - 1;
				else ii = i - 2;
				*value = *value + ((val * pow(10, ii)));
			}
			else if (ch == '.') { *index = i - 1; indFloat = true; }
			i++;
			if (i > (filedEnd - fieldBegin + 1) || i > 10)break;
			ch = Message[filedEnd - i];
		}

	}
	else return false;
	return true;
}*/
/*bool get_field_valueString(String Message, String field, String *value) {
	int check_field = Message.indexOf(field);
	if (check_field != -1) {
		int fieldBegin = Message.indexOf(field) + field.length();
		int filedEnd = Message.indexOf(';', fieldBegin);
		*value = Message.substring(fieldBegin, filedEnd);
	}
	else return false;
	return true;
}*/
/*
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
	delay (2);
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

*/
