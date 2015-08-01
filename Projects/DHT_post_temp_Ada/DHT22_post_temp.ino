#include "Adafruit_DHT.h"

#include "application.h"
#include "HttpClient.h"

#define SERVER "www.delong.com"
#define PORT 80
#define CGI_PATH "/cgi-bin/post_temp.cgi"
#define MY_ID "Photon_Test1"

#define DHTPOWER 5
#define DHTPIN 6
#define DHTTYPE DHT22

// #define SLEEP_DURATION 285 // 15 seconds to do our thing and 285 seconds sleep = 5 minute reporting interval
#define SLEEP_DURATION 5 // Sleep for 5 seconds between polling intervals For debugging

http_header_t headers[] = {
    { "Accept", "*/*" },
    { NULL, NULL },
};

DHT probe(DHTPIN, DHTTYPE);

HttpClient http;
uint32_t next_time;

http_request_t request;
http_response_t response;


void setup() {
    Serial.begin(115200);
    float tC;
    float hum;
    char Query[1024];
    char tbuf[10];
    char hbuf[10];
//    char **tbuf;
//    char **hbuf;
//    int *decpt;
//    int *sign;

    Serial.println();
    Serial.println("Application>\tGathering Data");
    // Gather the data
    pinMode(DHTPOWER, OUTPUT);
    digitalWrite(DHTPOWER, HIGH);
    probe.begin();
    delay(1000); // Let probe stabilize after powerup.
    hum = probe.getHumidity();
    tC = probe.getTempCelcius();
    if (tC > 150) tC = NAN; // Sample is ridiculous.
    Serial.print("Application>\t\tHumidity: ");
    Serial.println(hum);
    Serial.print("Application>\t\tTemperature: ");
    Serial.println(tC);


    // Report the data
    Serial.println("Application>\tPerparing Query");
    request.hostname=SERVER;
    request.port=PORT;
//    dtoa( tC, 0, 1, decpt, sign, tbuf);
//    dtoa(hum, 0, 1, decpt, sign, hbuf);
    String(tC).toCharArray(tbuf, 6);
    String(hum).toCharArray(hbuf, 6);
    sprintf(Query, "%s?my_id=%s&sensor_unit_1=dht_2&temp_1=%s&hum_1=%s", CGI_PATH, MY_ID, tbuf, hbuf);
    Serial.print("Applicatin>\t\tQuery String: ");
    Serial.println(Query);
    request.path = Query;

    // Send the request
    Serial.println("Application>\tSending Query");
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);

    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(response.body);

    // Sleep until next report period
    Serial.println("Sleeping");
    delay(500);
    System.sleep(SLEEP_MODE_DEEP, SLEEP_DURATION);
    Serial.println("This shouldn't happen");
}

void loop() {
}
