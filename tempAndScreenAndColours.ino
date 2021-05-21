// WIFI 
#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = "VM2206379";        
char pass[] = "yjz4mqkdDpnb";
int status = WL_IDLE_STATUS;

WiFiClient client;
// Locally hosted server using XAMPP
IPAddress server(192, 168, 0, 24);

unsigned long lastConnectionTime = 0;            
const unsigned long postingInterval = 5000; // One hour 


unsigned long startTime;
unsigned long currentTime;

// Data to be sent to the server, and then to be saved into the database
String postData;
String postVariable = "temp=";
String postDataTwo;
String postVariableTwo = "noise=";

const int pinSound = A0;               // pin of Sound Sensor
const int pinLed   = 7;                // pin of LED

int thresholdValue = 70;                 // the threshold to turn on or off the LED           

// LED SCREEN

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

//const int colorR = 0;
//const int colorG = 0;
//const int colorB = 255;

// TEMP SENSOR

const int pinTemp = A1;      // pin of temperature sensor

float temperature;
int B=3975;                  // B value of the thermistor
float resistance;

// BUZZER SET UP 
// This was set up but not used in the final implementation of the assignment as it was not requested

int speakerPin = 3;          // Grove Buzzer connect to D3

// SOUND
int sensorValue;

void setup() 
{
    startTime = millis();
    Serial.begin(9600);
    
    // SOUND SENSOR 
    pinMode(pinLed, OUTPUT);

    // TEMP SENSOR 
    pinMode(A1,INPUT);      //Setting the A0 pin as input pin to take data from the temperature sensor 

    // BUZZER
    pinMode(speakerPin, OUTPUT);
  
    // LED SCREEN
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    lcd.setCursor(0,0);
    lcd.print("SOUND LEVELS:");

    // WIFI
    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
    }

    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }  
}

void loop() 
{
    currentTime = millis();
    
    // SOUND SENSOR
    // Tested by tapping the sensor gently to show different reading levels 
    
    sensorValue = analogRead(pinSound);   //read the sensorValue on Analog 
    
    // TEMP SENSOR 
    // Temp sensor was tested by simply holding the temp sensor in my hand. The values that it read increased, and when the sensor was left alone they decreased,
    // indicating that the sensor was working properly
    
    int val = analogRead(pinTemp);                               // get analog value
    resistance=(float)(1023-val)*10000/val;                      // get resistance
    temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;     // calc temperature

    // LED SCREEN
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
      lcd.clear();

    // Print a message to the LCD.
    // Tested by tapping the sound sensor to replicate different sound levels. From this, it is easy to see the LCD screen change, and to check the screen colours
    // as well as the different dB levels and warning statements
    
    if(convertDB(sensorValue) > thresholdValue && convertDB(sensorValue) <=80 )
    {
      lcd.setCursor(0,0);
      lcd.print("SOUND LEVELS:");
      lcd.setRGB(255, 0, 0);
      lcd.setCursor(0,1);
      lcd.print(convertDB(sensorValue));
      lcd.print("dB ");
      lcd.print("1: Not Good");
      // tone(speakerPin, 1000);
      delay(1000); // ONE SECOND (for the screen)
    } 
    else if (convertDB(sensorValue) > 80)
    {
      lcd.setCursor(0,0);
      lcd.print("SOUND LEVELS:");
      lcd.setRGB(255, 0, 0);
      lcd.setCursor(0,1);
      lcd.print(convertDB(sensorValue));
      lcd.print("dB ");
      lcd.print("2: Harmful");
      // tone(speakerPin, 1000);
      delay(1000); // One second (for the screen)
    }
    else if (convertDB(sensorValue) < thresholdValue)
    {
      lcd.setCursor(0,0);
      lcd.print("SOUND LEVELS:");
      lcd.setRGB(0, 255, 0);
      lcd.setCursor(0,1);
      lcd.print(convertDB(sensorValue));
      lcd.print("dB ");
      // noTone(speakerPin);
      delay(1000); // // One second (for the screen)
    }
    lcd.setCursor(0, 1);
    
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  } 
}

// Tested by checking the values this function outputs against an online converter to ensure that the conversion has worked correctly 

float convertTempToF(float temp)
{
  float newTemp = 0;
  newTemp = temp * 9 / 5 + 32;
  // newTemp = (temp - 32) * 5 / 9;
  return newTemp;
}


// This function has been tested by changing the upload time to 5 seconds and tracking changes in phpmyadmin using XAMPP to act as my local server.
// The database successfully updates with the correct information - this has since been change to a one hour delay for assignment purposes

// Security wise, I'm not sure how private this connection is, so there could be a chance for hackers to get access to the localserver and parse the database
// for any information that it holds. As this isn;t a major concern for this assignment this has been left as it is, but obviously this implementation
// would not be put into practice for other users to use 

void httpRequest() {

  // Different variables to send to the database
  int temp = temperature;
  temp = convertTempToF(temp);
  postData = postVariable + temp;

  int noise = sensorValue;
  noise = convertDB(noise);
  postDataTwo = "&" + postVariableTwo + noise;

  String dataSize = postData + postDataTwo;

  // if there's a successful connection:
  if (client.connect(server, 8081)) 
  {
    Serial.println("connecting...");
    client.println("POST /uploads/test.php HTTP/1.1");
    client.println("Host: 192.168.0.24");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(dataSize.length());
    client.println();
    client.print(dataSize);
    client.println();
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
  } 
  else 
  {
    Serial.println("connection failed");
  }

  if(client.connected())
  {
    client.stop();
  }
  Serial.println(dataSize);
}

// Function to convert sound sensor readings into dB format 
int convertDB(int DB)
{
  int dB = (DB+83.2073) / 11.003; 
  return dB;
} 
