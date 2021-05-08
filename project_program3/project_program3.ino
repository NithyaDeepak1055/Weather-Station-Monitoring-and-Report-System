#include "UbidotsESPMQTT.h"
#include "DHT.h"
#define DHTPIN D5     
#define DHTTYPE DHT11 
#include<Wire.h>
#include<SFE_BMP180.h>a
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
SFE_BMP180 pressure;
#define ALTITUDE 1655.0 
#define WIFI_SSID "Your Wifi SSID"
#define WIFI_PASSWORD "Your Wifi Password"
#define FIREBASE_HOST "Your Firebase HostID"
#define FIREBASE_AUTH "Your Firebase Authorization ID"
FirebaseData my_database;     
DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature;
int smokepin=D4;


#define TOKEN "******"     // Your Ubidots TOKEN
#define WIFINAME "******"  // Your SSID
#define WIFIPASS "******"  // Your Wifi Pass

Ubidots client(TOKEN);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void setup() {

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

//dht sensor 

  Serial.println(F("DHTxx test!"));

  dht.begin();

  if (pressure.begin())
Serial.println("BMP180 init success");
else
{
Serial.println("BMP180 init fail\n\n");
while(1); // Pause forever.
}

    //rain drop sensor    
   pinMode(A0,INPUT);
 

 //smoke sensor
 pinMode(D4,INPUT);


  
  // put your setup code here, to run once:
 
  client.setDebug(true);  // Pass a true or false bool value to activate debug messages
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);


}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    client.reconnect();
  }
 //dht calculation
     delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //pressure calculation
  char status;
double T,P,p0,a,p1,b,p2;
double Tdeg, Tfar, phg, pmb,pascal;
 
status = pressure.startTemperature();
if (status != 0)
{
// Wait for the measurement to complete:
delay(status);
status = pressure.getTemperature(T);
if (status != 0)
{
// Print out the measurement:
Serial.print("temperature: ");
Serial.print(T,2);
Tdeg = T;
Serial.print(" deg C, ");
Tfar = (9.0/5.0)*T+32.0;
Serial.print((9.0/5.0)*T+32.0,2);
Serial.println(" deg F");

 
status = pressure.startPressure(3);                                  // The startPressure() method sends the command to start the measurement of pressure.
                                                                       //We provide an oversampling value as parameter, which can be between 0 to 3.
                                                                     //A value of 3 provides a high resolution, but also a longer delay between measurements.
if (status != 0)
{
// Wait for the measurement to complete:
delay(status);
status = pressure.getPressure(P,T);
if (status != 0)
{
// Print out the measurement:
Serial.print("absolute pressure: ");
Serial.print(P,2);
pmb = P;
Serial.print(" mb, ");
phg = P*0.0295333727;
Serial.print(P*0.0295333727,2);
Serial.print(" inHg, ");
pascal=pmb*100;
Serial.print(pmb*100,2);
Serial.println("Pa");

 
p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
Serial.print("relative (sea-level) pressure: ");
Serial.print(p0,2);
Serial.print(" mb, ");
 p1 =p0*0.0295333727;
Serial.print(p0*0.0295333727,2);
Serial.print(" inHg, ");
p2=p0*100;
Serial.print(p0*100);
Serial.println(" Pa");
 
a = pressure.altitude(P,p0);
Serial.print("computed altitude: ");
Serial.print(a,0);
Serial.print(" meters, ");
b=a*3.28084;
Serial.print(a*3.28084,2);
Serial.println(" feet");
}
else Serial.println("error retrieving pressure measurement\n");
}
else Serial.println("error starting pressure measurement\n");
}
else Serial.println("error retrieving temperature measurement\n");
}
else Serial.println("error starting temperature measurement\n");
 

int rain = analogRead(A0);
int dropreading=analogRead(A0);
  Serial.print("Rain Status: ");
  Serial.println(rain);
  if(rain==1024)
  {
    Serial.println("No rainfall");
  }
  else if(rain>=900 && rain<1024)
  {
    Serial.println("No rainfall");
  }
   else if(rain>=400 && rain<900)
   {
    Serial.println("Moderate rainfall");
   }
   else
   {
    Serial.println("Heavy rainfall");
   }

  //smoke sensors
  int smokereading=digitalRead(D4);
  Serial.print("Smoke Status:");
  Serial.println(smokereading);
 if(smokereading==0)
  {
    Serial.println("Unhealthy");
  }
  else
  {
    Serial.println("Healthy");
  }



  ///calling firebase

Firebase.set(my_database,"temp",t);
Firebase.set(my_database,"tempf",f);
Firebase.set(my_database,"humidity",h);
Firebase.set(my_database,"heatindex",hic);
Firebase.set(my_database,"pressure",pmb);
Firebase.set(my_database,"Pressure_in_hg",phg);
Firebase.set(my_database,"Pressure_in_pa",pascal);
Firebase.set(my_database,"sea_level_pressure_in_mb",p0);
Firebase.set(my_database,"sea_level_pressure_in_hg",p1);
Firebase.set(my_database,"sea_level_pressure_in_pa",p2);
Firebase.set(my_database,"altitude in m",a); 
Firebase.set(my_database,"altitude in ft",b); 
Firebase.set(my_database,"rainfall",rain); 
Firebase.set(my_database,"smoke",smokereading); 

//ubidots call
//client.add("temperature c",t);
//client.ubidotsPublish("test");
//client.add("temperature f",f);
//client.ubidotsPublish("test");
//client.add("humidity",h);
//client.ubidotsPublish("test");
//client.add("heatindex",hic);
//client.ubidotsPublish("test");
client.add("Absolute pressure in mb",pmb);
client.ubidotsPublish("test");
client.add("Absolute pressure in hg",phg);
client.ubidotsPublish("test");
client.add("Absolute pressure in Pa",pascal);
client.ubidotsPublish("test");
client.add("Sea level pressure in mb",p0);
client.ubidotsPublish("test");
client.add("Sea level pressure in hg",p1);
client.ubidotsPublish("test");
client.add("Sea level pressure in Pa",p2);
client.ubidotsPublish("test");
//client.add("smokedetector",smokereading);
//client.ubidotsPublish("test");
//client.add("rainsensor",rain);
//client.ubidotsPublish("test");
client.add("altitude in meters",a);
client.ubidotsPublish("test");
client.add("altitude in feet",b);
client.ubidotsPublish("test");


client.loop();
}
