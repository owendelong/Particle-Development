#include "PostTempHumV2.ino"

// This #include statement was automatically added by the Spark IDE.
#include "application.h"

// This #include statement was automatically added by the Spark IDE.
#include "HttpClient.h"

// This #include statement was automatically added by the Spark IDE.
#include "PietteTech_DHT.h"

#define SERVER "www.delong.com"
#define PORT 80
#define CGI_PATH "/cgi-bin/post_temp.cgi"

char MY_ID[40] = "Invalid_Photon";
#define MY_VER "2.00a"

#define CYCLE_DURATION 60 // Poll at 60 second intervals
#define LOGGING

#define RETRIES 5

SYSTEM_MODE(MANUAL);

// TODO: Migrate Serial.print statements to SYSLOG functionality
HttpClient http;

http_header_t headers[] = {
    { "Accept", "*/*" },
    { NULL, NULL },
};

const char *HAS_EXT_ANT[] = {
    "23002f000847343337373738",
    "3c002e000347343337373738",
    NULL,
};

void * data_collecters[MAX_TYPE+1];

uint32_t next_time = millis() + (CYCLE_DURATION * 1000);

http_request_t request = {0};
http_response_t response;

typedef bool(*FunctionPointer)();
FunctionPointer *data_collectors; // At time of writing only need to support 2 sensor families.
uint8_t HandlerCount=0;


// Sensor independent wrapper for sensor-specific data collection functionality
bool collectData(float **temp, float **humidity, void *Object, uint8_t max_devices, uint8_t type)
{
  return(data_collectors[type](temp, humidity, Object, max_devices));
}

bool is_external(const char *MY_ID)
{
  uint8_t i;
  for(i=0; HAS_EXT_ANT[i] != NULL; i++)
  {
    if(strncmp(MY_ID, HAS_EXT_ANT[i], 40) == 0) return true;
  }
  return false;
}

void setup() {
    Serial.begin(115200);
    pinMode(7, OUTPUT);
    digitalWrite(7, LOW);
    (System.deviceID()).toCharArray(MY_ID, 40);
    if (is_external(MY_ID))
    {
      WiFi.selectAntenna(ANT_EXTERNLA);
    }

}

void loop() {
  int result;
  float tC[40];
  float hum[40];
  char tbuf[10];
  char hbuf[10];
  char Query[1024];
  char logbuf[80];
  uint32_t startup = millis();
  uint8_t i;

  Serial.print("Starting connection process");
  Spark.connect();
  Serial.println(".");
  Serial.println("Application>\tGathering Data.");
  // Gather the data

  for (i=0; i < RETRIES; i++)
  {
    if (collectData(&tC, &hum, DHT, 1, DHT_TYPE))
    {
      // Data collection succeeded
      if (Spark.connected())
      {
        digitalWrite(7, HIGH);
        Spark.process();
      }
      break;
    }
    else
    {
      // Data collection failed
      delay(2000); // Retry in 2 seconds
    }
  }

  for(i=0; i < RETRIES; i++)
  {
    if (collectData(&tc+1, &hum+1, DS, 39, DS_TYPE))
    {
      // Data collection succeeded
      if (Spark.connected())
      {
        digitalWrite(7, HIGH);
        Spark.process();
      }
      break;
    }
    else
    {
      // Data collection failed
    }
  }
