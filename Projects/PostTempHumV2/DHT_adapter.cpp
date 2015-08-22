#include "DHT_adapter.h"

RegisterHandler(DHT_TYPE, dht_collectData);

// ISR Callback wrapper required by library
void dht_wrapper() {
    DHT.isrCallback();
}


// Collect data from sensor. Return true for success, false otherwise
// Results are placed in *temp and *humidity, NaN for any missing value.
bool dht_collectData(float **temp, float **humidity, PietteTech_DHT Sensor, uint8_t max_devices = 1)
{
  uint8_t i;

  pinMode(DHTPOWER, OUTPUT);
  digitalWrite(DHTPOWER, HIGH);
  delay(1000);

  result = Sensor.acquireAndWait(DHT_TIMEOUT); // Allow up to 5 seconds for acquisition
  digitalWrite(DHTPOWER, LOW); // Power off sensor

  switch(result) {
    case DHTLIB_OK:
    hum = DHT.getHumidity();
    tC = DHT.getCelsius();
    Serial.print("Application>\t\tHumidity: ");
    Serial.println(hum);
    Serial.print("Application>\t\tTemperature: ");
    Serial.println(tC);
    sprintf(logbuf, "DHT_OK");
    *temp = tC;
    *humidity = hum;
    return(true);

    case DHTLIB_ERROR_CHECKSUM:
    Serial.println("Error\tChecksum error");
    sprintf(logbuf, "Error\tChecksum error");
    return(false);

    case DHTLIB_ERROR_ISR_TIMEOUT:
    Serial.println("Error\n\r\tISR time out error");
    sprintf(logbuf, "Error\tISR time out error");
    return(false);

    case DHTLIB_ERROR_RESPONSE_TIMEOUT:
    Serial.println("Error\n\r\tResponse time out error");
    sprintf(logbuf, "Error\tResponse time out error");
    return(false);

    case DHTLIB_ERROR_DATA_TIMEOUT:
    Serial.println("Error\n\r\tData time out error");
    sprintf(logbuf, "Error\tData time out error");
    return(false);

    case DHTLIB_ERROR_ACQUIRING:
    Serial.println("Error\n\r\tAcquiring");
    sprintf(logbuf, "Error\tAcquiring");
    return(false);

    case DHTLIB_ERROR_DELTA:
    Serial.println("Error\n\r\tDelta time to small");
    sprintf(logbuf, "Error\tDelta time to small");
    return(false);

    case DHTLIB_ERROR_NOTSTARTED:
    Serial.println("Error\n\r\tNot started");
    sprintf(logbuf, "Error\tNot started");
    return(false);

    default:
    Serial.println("Unknown error");
    sprintf(logbuf, "Unknown error");
    return(false);
  }
}
