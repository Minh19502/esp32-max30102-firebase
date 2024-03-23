#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "TP-Link_0D5E"
#define WIFI_PASSWORD "93853767"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA60XyWnId7purPGY6fNAdA5z1ijoJ9yrk"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://esp32-5be65-default-rtdb.firebaseio.com/"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
//#define USER_EMAIL "USER_EMAIL"
//#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
bool signupOK = false;
void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  //  auth.user.email = USER_EMAIL;
  //  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("signUp ok");
    signupOK = true;
  }
  else
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  Firebase.begin(&config, &auth);
  config.timeout.serverResponse = 10 * 1000;
}

void loop()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    FirebaseJson json;
    json.set("lat", 123);
    json.set("lng", 12223);
    if (count == 0) { // remove
      Serial.printf("Remove node 'location'... %s\n", Firebase.RTDB.deleteNode(&fbdo, "/location") ? "ok" : fbdo.errorReason().c_str());
    }
    Serial.printf("Set node 'location'... %s\n", Firebase.RTDB.set(&fbdo, "/location/" + String(count), &json) ? "ok" : fbdo.errorReason().c_str());
    count++;
  }
}