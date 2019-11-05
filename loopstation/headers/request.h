#ifndef REQUEST_H
#define REQUEST_H
#include <WiFi.h>

class Request {
  public:
    void GET(const char* host, const char* cup_id, char* get_response);
    void POST(const char* host, const char* json);
  private:
    WiFiClient client;

    bool skipResponseHeaders();
};
#endif
