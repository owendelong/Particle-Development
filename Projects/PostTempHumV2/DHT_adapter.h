#ifndef DHT_ADAPTER

// This #include statement was automatically added by the Spark IDE.
#include "PietteTech_DHT.h"

#define DHTPOWER 5
#define DHTPIN 6
#define DHTTYPE DHT22
#define DHT_TIMEOUT 5000    // Timeout in milliseconds

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

bool dht_collectData(float **temp, float **humidity, PietteTech_DHT Sensor, uint8_t max_devices = 1);

#include "PostTempHumV2.h"

#define DHT_ADAPTER
#endif
