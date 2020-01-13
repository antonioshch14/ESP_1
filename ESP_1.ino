// #include<EthernetClient.h> //Uncomment this library to work with ESP8266
#include<ESP8266WiFi.h> //Uncomment this library to work with ESP8266
char jsonBuffer[2000] = "["; // Initialize the jsonBuffer to hold data

char ssid[] = "Keenetic-4574"; //  Your network SSID (name)
char pass[] = "Gfmsd45kaxu69$"; // Your network password
WiFiClient client; // Initialize the WiFi client library
char server[] = "api.thingspeak.com"; // ThingSpeak Server

/* Collect data once every 15 seconds and post data to ThingSpeak channel once every 2 minutes */
// unsigned long lastConnectionTime = 0; // Track the last connection time
unsigned long lastUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 5L * 1000L; // s
// const unsigned long updateInterval = 15L * 1000L; // Update once every 15 seconds
int const maxRow=10; //for data read from serial
char getData[maxRow][9][6]; //for data read from serial
int row; //for data read from serial


void setup() {
 Serial.begin(9600);
  // Attempt to connect to WiFi network
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass); 
    Serial.print("WiFimode is "); Serial.println (WiFi.getMode());// Connect to WPA/WPA2 network. Change this line if using open or WEP network
    delay(10000);  // Wait 10 seconds to connect
  }
  Serial.println("Connected to wifi");
 printWiFiStatus(); // Print WiFi connection information
}
void loop() {
    if (millis() - lastUpdateTime >=  postingInterval) 
    { 
    readDataSerial(row);
    if (row>0) {updatesJson(jsonBuffer,row);row=0; memset(getData,0,sizeof(getData));}
    }
  }
    
void printWiFiStatus() {
  // Print the SSID of the network you're attached to:
   
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your device IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
 long rssi = WiFi.RSSI();
 Serial.print("signal strength (RSSI):");
 Serial.print(rssi);
 Serial.println(" dBm");
  Serial.print("WiFimode is "); Serial.println (WiFi.getMode());
}

void updatesJson(char* jsonBuffer,int row){
  //   "[{\"delta_t\":0,\"field1\":-70},{\"delta_t\":3,\"field1\":-66}]"
  int i; int ii;
  char field[8][2]={"1","2","3","4","5","6","7","8"};
  for (i=0;i<row;i++)
  {strcat(jsonBuffer,"{\"delta_t\":");
   strcat(jsonBuffer,getData[i][0]);
   strcat(jsonBuffer,",");
   for (ii=1;ii<5;ii++)
      {
       strcat(jsonBuffer, "\"field");
       strcat(jsonBuffer,field[ii-1]);
       strcat(jsonBuffer,"\":");
       strcat(jsonBuffer,getData[i][ii]);
      if (ii!=4) strcat(jsonBuffer,",");
      }
       strcat(jsonBuffer,"},");
       // If posting interval time has reached 2 minutes, update the ThingSpeak channel with your data
  }
        size_t len = strlen(jsonBuffer);
        jsonBuffer[len-1] = ']';
        httpRequest(jsonBuffer);
  
  lastUpdateTime = millis(); // Update the last update time
}

// Updates the ThingSpeakchannel with data
void httpRequest(char* jsonBuffer) {
  /* JSON format for data buffer in the API
   *  This examples uses the relative timestamp as it uses the "delta_t". You can also provide the absolute timestamp using the "created_at" parameter
   *  instead of "delta_t".
   *   "{\"write_api_key\":\"YOUR-CHANNEL-WRITEAPIKEY\",\"updates\":[{\"delta_t\":0,\"field1\":-60},{\"delta_t\":15,\"field1\":200},{\"delta_t\":15,\"field1\":-66}]
   */
  // Format the data buffer as noted above
  char data[2000] = "{\"write_api_key\":\"SR7I86O66JT1ZIQQ\",\"updates\":"; // Replace YOUR-CHANNEL-WRITEAPIKEY with your ThingSpeak channel write API key
  strcat(data,jsonBuffer);
  strcat(data,"}");
  // Close any connection before sending a new request
  client.stop();
  String data_length = String(strlen(data)+1); //Compute the data buffer length
  // Serial.println(data);
  // POST data to ThingSpeak
  if (client.connect(server, 80)) {
    client.println("POST /channels/692244/bulk_update.json HTTP/1.1"); // Replace YOUR-CHANNEL-ID with your ThingSpeak channel ID
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: mw.doc.bulk-update (Arduino ESP8266)");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("Content-Length: "+data_length);
    client.println();
    client.println(data);
  }
  else {
    Serial.println("Failure: Failed to connect to ThingSpeak");
  }
  delay(250); //Wait to receive the response
  client.parseFloat();
  String resp = String(client.parseInt());
 Serial.print("R:"+resp); // Print the response code. 202 indicates that the server has accepted the response
  // Serial.println(data);
  jsonBuffer[0] = '['; //Reinitialize the jsonBuffer for next batch of data
  jsonBuffer[1] = '\0';
  //lastConnectionTime = millis(); //Update the last conenction time
}
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
