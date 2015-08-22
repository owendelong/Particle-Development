#ifndef DS_ADAPTER

// This #include statement was automatically added by the Spark IDE.
#include "PietteTech_DHT.h"

#define DSPOWER 0
#define DSPIN   1

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

bool ds_collectData(float **temp, float **humidity, PietteTech_DHT Sensor, uint8_t max_devices);

RegisterHandler(DS, ds_collectData);

#include "PostTempHumV2.h"

#define DS_ADAPTER
#endif
