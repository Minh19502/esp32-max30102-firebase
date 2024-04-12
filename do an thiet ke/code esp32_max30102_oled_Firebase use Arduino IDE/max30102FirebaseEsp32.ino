#include <Arduino.h>
#include <Adafruit_GFX.h>    //OLED libraries
#include <Adafruit_SSD1306.h> //OLED libraries
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm
#include "ESP32Servo.h"

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "P502"
#define WIFI_PASSWORD "20032002@"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBME-Ez7_5SugKa1xcdo6VHPdKi_VNVrzc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp32-firebase-demo-8be1d-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// Timer variables (send new readings every three minutes)
unsigned long timerDelay = 180000;

double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;

double SpO2 = 0;
double ESpO2 = 90.0;
double FSpO2 = 0.7; //filter factor for estimated SpO2
double frate = 0.95; //low pass filter for IR/red LED value to eliminate AC component
int i = 0;
int Num = 30;
#define FINGER_ON 7000    
#define MINIMUM_SPO2 90.0 

//OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)


static const unsigned char PROGMEM logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,        
  0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
  0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
  0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,
};

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
  0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
  0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
  0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
  0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
  0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
  0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
  0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00
};

static const unsigned char PROGMEM O2_bmp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x3f, 0xc3, 0xf8, 0x00, 0xff, 0xf3, 0xfc,
  0x03, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0x7e,
  0x1f, 0x80, 0xff, 0xfc, 0x1f, 0x00, 0x7f, 0xb8, 0x3e, 0x3e, 0x3f, 0xb0, 0x3e, 0x3f, 0x3f, 0xc0,
  0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3e, 0x2f, 0xc0,
  0x3e, 0x3f, 0x0f, 0x80, 0x1f, 0x1c, 0x2f, 0x80, 0x1f, 0x80, 0xcf, 0x80, 0x1f, 0xe3, 0x9f, 0x00,
  0x0f, 0xff, 0x3f, 0x00, 0x07, 0xfe, 0xfe, 0x00, 0x0b, 0xfe, 0x0c, 0x00, 0x1d, 0xff, 0xf8, 0x00,
  0x1e, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00,
  0x0f, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int Tonepin = 4; 

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi... ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Start");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();
  delay(3000);
 
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102");
    while (1);
  }
  
  byte ledBrightness = 0x7F; 
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; 
  int sampleRate = 800; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 215; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384
  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  particleSensor.enableDIETEMPRDY();

  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  initWiFi();
  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
}

void loop() {
  long irValue = particleSensor.getIR();    //Reading the IR value it will permit us to know if there's a finger on the sensor or not

  if (irValue > FINGER_ON ) {
      if (checkForBeat(irValue) == true) {
        display.clearDisplay();
        display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(42, 10);
        //display.print(beatAvg); display.println(" BPM");
        display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE);
        display.setCursor(42, 40);
        
        if (beatAvg > 30) display.print(String(ESpO2) + "%");
        else display.print("---- %" );
        display.display();
        tone(Tonepin, 1000);
        delay(10);
        noTone(Tonepin);
        //Serial.print("Bpm="); Serial.println(beatAvg);
        long delta = millis() - lastBeat;
        lastBeat = millis();
        beatsPerMinute = 60 / (delta / 1000.0);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {       
          rates[rateSpot++] = (byte)beatsPerMinute; 
          rateSpot %= RATE_SIZE;
          beatAvg = 0;
          for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }
      }

      uint32_t ir, red ;
      double fred, fir;
      particleSensor.check(); //Check the sensor, read up to 3 samples
      if (particleSensor.available()) {
        i++;
        ir = particleSensor.getFIFOIR(); 
        red = particleSensor.getFIFORed();
        //Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
        fir = (double)ir;
        fred = (double)red;
        aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
        avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
        sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level
        sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level
        if ((i % Num) == 0) {
          double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
          SpO2 = -23.3 * (R - 0.4) + 100;
          ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;//low pass filter
          if (ESpO2 <= MINIMUM_SPO2) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
          if (ESpO2 > 100) ESpO2 = 99.9;
          //Serial.print(",SPO2="); Serial.println(ESpO2);
          sumredrms = 0.0; sumirrms = 0.0; SpO2 = 0;
          i = 0;
        }
        particleSensor.nextSample(); //We're finished with this sample so move to next sample
      }
      
      Serial.print("Bpm:" + String(beatAvg));
      
      if (beatAvg > 30)  Serial.println(",SPO2:" + String(ESpO2));
      else Serial.println(",SPO2:" + String(ESpO2));

      display.clearDisplay();
      display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(42, 10);
      display.print(beatAvg); display.println(" BPM");
      display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE);
      display.setCursor(42, 40);
      
      if (beatAvg > 30) display.print(String(ESpO2) + "%");
      else display.print("90.00%" );
      display.display();
      if (Firebase.ready() && (millis() - sendDataPrevMillis > 7000 || sendDataPrevMillis == 0)){
        sendDataPrevMillis = millis();
        if (Firebase.RTDB.setFloat(&fbdo, "/BPM", beatAvg)) {
          Serial.println("Sent BPM data to Firebase");
        } else {
          Serial.println("Failed to send BPM data to Firebase");
        }

        // Gửi dữ liệu SpO2 lên Firebase
        if (Firebase.RTDB.setFloat(&fbdo, "/SpO2", ESpO2)) {
          Serial.println("Sent SpO2 data to Firebase");
        } else {
          Serial.println("Failed to send SpO2 data to Firebase");
        }

      }
   }
  //"Finger Please"
  else {
    for (byte rx = 0 ; rx < RATE_SIZE ; rx++) rates[rx] = 0;
    beatAvg = 0; rateSpot = 0; lastBeat = 0;
    avered = 0; aveir = 0; sumirrms = 0; sumredrms = 0;
    SpO2 = 0; ESpO2 = 0;
    //Finger Please
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(30, 5);
    display.println("Finger");
    display.setCursor(30, 35);
    display.println("Please");
    display.display();
    noTone(Tonepin);
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
        sendDataPrevMillis = millis();
        if (Firebase.RTDB.setFloat(&fbdo, "/BPM", beatAvg)) {
          Serial.println("Sent BPM data to Firebase");
        } else {
          Serial.println("Failed to send BPM data to Firebase");
        }
        // Gửi dữ liệu SpO2 lên Firebase
        if (Firebase.RTDB.setFloat(&fbdo, "/SpO2", ESpO2)) {
          Serial.println("Sent SpO2 data to Firebase");
        } else {
          Serial.println("Failed to send SpO2 data to Firebase");
        }
      }
  }
}



