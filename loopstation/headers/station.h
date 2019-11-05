#ifndef STATION_H
#define STATION_H
#include "request.h"
#include "constants.h"
#include <WiFi.h>
#include <MFRC522.h>
#include <SPI.h>

class Station {
  public:
    Request request;
    bool parseJSON(char* get_response);
    void setupWifi();
    void makeJSON(const char* bin_id, const char* cup_id, char* json);
    void setupRfidReader();
    void readRfidTag(char* cup_id);
    void setupProximitySensor();
    bool safe();
  private:
    MFRC522 tagReader = MFRC522(SS_PIN, RST_PIN);  // Create MFRC522 instance
};
#endif
