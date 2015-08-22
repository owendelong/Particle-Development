// This #include statement was automatically added by the Spark IDE.
#include "application.h"

// This #include statement was automatically added by the Spark IDE.
#include "HttpClient.h"

// This #include statement was automatically added by the Spark IDE.
#include "PietteTech_DHT.h"
// Library required ISR wrapper declaration
void dht_wrapper(); // must be declared before the lib initialization


#define DHTPOWER 5
#define DHTPIN 6
#define DHTTYPE DHT22

#define SERVER "www.delong.com"
#define PORT 80
#define CGI_PATH "/cgi-bin/post_temp.cgi"

char MY_ID[40] = "Invalid_Photon";


#define MY_VER "0.02by"                      //Version Number String

#define CYCLE_DURATION 60 // Poll at 60 second intervals
#define LOGGING

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

uint32_t next_time = millis() + (CYCLE_DURATION * 1000) - 1000;

http_request_t request = {0};
http_response_t response;

void dht_wrapper() {
    DHT.isrCallback();
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
    Serial.println("Launching");
    pinMode(7, OUTPUT);
    digitalWrite(7, LOW);
    (System.deviceID()).toCharArray(MY_ID, 40);
    Serial.print("My ID is ");
    Serial.println(MY_ID);
    if (is_external(MY_ID))
    {
      WiFi.selectAntenna(ANT_EXTERNAL);
    }

}

void loop() {
  int result;
  float tC;
  float hum;
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
  pinMode(DHTPOWER, OUTPUT);
  digitalWrite(DHTPOWER, HIGH);
  if (Spark.connected())
  {
    digitalWrite(7, HIGH);
    Spark.process();
  }
  delay(1000); // Let probe stabilize after powerup.
  for(i=0; i<5; i++)
  {
    if (i > 0)
    {
      Serial.println("Retrying");
    }
    // Retry up to 5 times
    result = DHT.acquireAndWait(5000);
    if (result == DHTLIB_ERROR_ACQUIRING)
    {
      System.sleep(SLEEP_MODE_DEEP, 2); // Reset system and try again in 2 seconds.
      delay(2000);
    }
    startup = millis();
    if (result == DHTLIB_OK) break; // Got a valid sample.
    Serial.print("Failed result: ");
    Serial.println(result);
  }
  Serial.println("\nApplication>\tData acquisition complete.");
  if (Spark.connected())
  {
    digitalWrite(7, HIGH);
    Spark.process();
  }
  Serial.print("Application>\tChecking result (");
  Serial.print(result);
  Serial.println(").");
  switch(result) {
      case DHTLIB_OK:
          hum = DHT.getHumidity();
          tC = DHT.getCelsius();
          Serial.print("Application>\t\tHumidity: ");
          Serial.println(hum);
          Serial.print("Application>\t\tTemperature: ");
          Serial.println(tC);
          sprintf(logbuf, "DHT_OK");
          break;

      case DHTLIB_ERROR_CHECKSUM:
          Serial.println("Error\tChecksum error");
          sprintf(logbuf, "Error\tChecksum error");
          break;

      case DHTLIB_ERROR_ISR_TIMEOUT:
          Serial.println("Error\n\r\tISR time out error");
          sprintf(logbuf, "Error\tISR time out error");
          break;

      case DHTLIB_ERROR_RESPONSE_TIMEOUT:
          Serial.println("Error\n\r\tResponse time out error");
          sprintf(logbuf, "Error\tResponse time out error");
          break;

      case DHTLIB_ERROR_DATA_TIMEOUT:
          Serial.println("Error\n\r\tData time out error");
          sprintf(logbuf, "Error\tData time out error");
          break;

      case DHTLIB_ERROR_ACQUIRING:
          Serial.println("Error\n\r\tAcquiring");
          sprintf(logbuf, "Error\tAcquiring");
          break;

      case DHTLIB_ERROR_DELTA:
          Serial.println("Error\n\r\tDelta time to small");
          sprintf(logbuf, "Error\tDelta time to small");
          break;

      case DHTLIB_ERROR_NOTSTARTED:
          Serial.println("Error\n\r\tNot started");
          sprintf(logbuf, "Error\tNot started");
          break;

      default:
          Serial.println("Unknown error");
          sprintf(logbuf, "Unknown error");
          break;
  }
  // Report the data
  Serial.println("Application>\tPerparing Query");
  request.hostname=SERVER;
  request.port=PORT;
  String(tC).toCharArray(tbuf, 6);
  String(hum).toCharArray(hbuf, 6);
  if (result == DHTLIB_OK)
  {
    sprintf(Query, "%s?my_id=%s&ver=%s&ant=%s&sensor_unit_1=%s&temp_1=%s&hum_1=%s&logmsg=%s&next_time=%ld&current_time=%ld",
        CGI_PATH, MY_ID, MY_VER, is_external(MY_ID) ? "ext" : "int", MY_ID, tbuf, hbuf, logbuf, next_time, millis());
  }
  else
  {
    sprintf(Query, "%s?my_id=%s&ver=%s&logmsg=%s&next_time=%ld&current_time=%ld",
        CGI_PATH, MY_ID, MY_VER, logbuf, next_time/4, millis());
  }
  request.path = Query;
  // Make sure WIFI is up.
  for(result=0; millis() < startup+30000 && !Spark.connected(); result++)
  {
    Serial.print(".");
    delay(1000);
  }
  digitalWrite(7, Spark.connected() ? HIGH : LOW);
//    delay(5000);
  if (!Spark.connected())
  {
    RGB.control(true);
    RGB.color(255,0,0);
    RGB.brightness(255);
    Serial.println("Not connected");
    delay(250);
    next_time -= millis();
    next_time /= 1000;
    if(next_time < 5 || next_time > CYCLE_DURATION) next_time = 5;
    System.sleep(SLEEP_MODE_DEEP, next_time); //  Connect failed. Sleep 1/4 of normal time and retry.
    delay(1000);
  }
  // Send the request
  Serial.println("Application>\tSending Query");
  Serial.print("\t\t\t\t");
  Serial.println(request.hostname);
  Serial.print("\t\t\t\t");
  Serial.println(request.port);
  Serial.print("\t\t\t\t");
  Serial.println(request.path);
  http.get(request, response, headers);
  Serial.print("Application>\tQuery Returned ");
  Serial.println(response.status);
  if (response.status < 0)
  {
    RGB.control(true);
    RGB.color(255,0,0);
    RGB.brightness(255);
    Spark.disconnect(); // Clearly Spark.connect() didn't succeed, even if it thinks it did.
    delay(2000);
    WiFi.connect();
    delay(2000);
    if (WiFi.ready())
    {
      RGB.color(255,255,0); // Use Yellow to indicate not connected to cloud but WiFi OK
    }
    // Retry up to 5 times.
    for(result = 0; result < 5; result++)
    {
      startup = millis();
      while(millis() < startup+10000 && !WiFi.ready())
      {
        delay(250);
        Spark.process();
        digitalWrite(7, (millis() & 0x100) ? HIGH : LOW);
      }
      Serial.println("Application>\tSending Query");
      request.hostname="";
      request.ip={192,159,10,7};
      http.get(request, response, headers);
      if (response.status < 0)
      {
        digitalWrite(7, LOW);
        if (result % 2)
        {
          RGB.control(false);
          Spark.disconnect();
          delay(2000);
          Spark.connect();
          delay(2000);
        }
        else
        {
          RGB.control(true);
          RGB.color(255,255,255);
          RGB.brightness(255);
          WiFi.disconnect();
          delay(2000);
          RGB.color(255,0,0);
          WiFi.connect();
          delay(2000);
        }
        if (Spark.connected())
        {
          digitalWrite(7, HIGH);
        }
        else if (WiFi.ready())
        {
          RGB.color(255,255,0); // Use Yellow to indicate not connected to cloud
          Spark.connect();
          delay(2000);
          if (Spark.connected())
          {
            RGB.control(false);
            digitalWrite(7, HIGH);
          }
        }
        else
        {
          RGB.color(255,0,0); // WiFi connection failed too.
        }
        delay(result * 100);
        Serial.print(".");
        Serial.println("Retrying...");
        continue;
      }
      Serial.println("Finished...");
      if (response.status < 0)
      {
        Serial.println("Timed out without connecting successfully.");
      }
      else if (response.status == 200)
      {
        Serial.println("Connection succeeded.");
      }
      else
      {
        Serial.print("Unknown result: ");
        Serial.println(response.status);
      }
      break; // Connected, don't retry again, even if result is not 200.
    }
  }
  if (response.status != 200)
  {
    analogWrite(7, 255);
    delay(100);
    analogWrite(7, 0);
    delay(100);
    analogWrite(7, 255);
    delay(100);
    analogWrite(7, 128);
    if (response.status < 0) // TCP Connect failed -- RED
    {
      Serial.println("RED");
      RGB.color(255,0,0);
    }
    else // Some other HTTP Response code -- BLUE
    {
      Serial.println("BLUE");
      RGB.color(0,0,255);
    }
    result = millis();
    while (result + 60000 < millis()) // Show error for 60 seconds
    {
      RGB.brightness(millis() % 256);
      Spark.process();
      delay(5);
    }
  }
  RGB.control(false);
  Spark.process();
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);
  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
  startup=millis();
  Serial.print("30 second processing loop: ");
  while (millis() < startup+30000 && millis() > startup) // Terminate on millis() counter wrap
  {
    // Wait 15 seconds for queued update
    Spark.process();
    delay(100);

  }
  Serial.println("Completed.");
  // Sleep until next report period
  Serial.println("Sleeping.");
  delay(250);
  next_time -= millis();
  next_time /= 1000;
  if (next_time < 5 || next_time > CYCLE_DURATION) next_time = 5;
  System.sleep(SLEEP_MODE_DEEP, next_time);
  delay(2000);

}
