#include <arduino.h>
#include "../headers/request.h"
#include <ArduinoJson.h>

/**
    Skips the headers from a GET response.

    @param None.
    @return a book if the end of headers marker is found.
*/
bool Request::skipResponseHeaders() {
  char endOfHeaders[] = "\r\n\r\n"; // HTTP headers end with an empty line
  bool ok = client.find(endOfHeaders);
  if (!ok){ Serial.println("No response or invalid response!"); }
  return ok;
}

/**
    Make a GET request and dump the response into a variable passed by reference.

    @params host The URL of the server.
            cup_id The UID of the RFID tag of the cup.
            get_response The place to dump the GET response.
    @return Void.
*/
void Request::GET(const char* host, const char* cup_id, char* get_response){
  Serial.print("Connecting to ");
  Serial.println(host);

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
      Serial.println("Connection failed");
      return;
  }

  // We now create a URI for the request i.e. /station/cup/d9cff5f3
  String url = "/station/cup/";
  url += cup_id;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          break;
      }
  }

  // Wait until you have skipped all the headers
  while(client.available() && !skipResponseHeaders());

  while(client.available()){
    // Store the unprocessed data
    String line = client.readStringUntil('\r');
    line.toCharArray(get_response, 20);
  }
}

/**
    Make a POST request and send the payload passed by reference.

    @params host The URL of the server.
            json The payload to be sent to the server in JSON form.
    @return Void.
*/
void Request::POST(const char* host, const char* json){
  Serial.print("Connecting to ");
  Serial.println(host);

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
      Serial.println("Connection failed");
      return;
  }
  Serial.println("Connected to server");
  // Make the HTTP request
  client.println("POST /station/cup HTTP/1.1");
  client.println("Connection: Keep-Alive");
  client.println("Keep-Alive: timeout=2, max=1");
  char message[200];
  sprintf(message, "Host: %s", host); // A workaround to format data like so.
  Serial.println(message);
  client.println(message);
  client.println("Accept: */*");
  client.println("Content-Length: " + String(String(json).length()));
  client.println("Content-Type: application/json");
  client.println();
  client.println(json);
  Serial.println("POST connection keep-alive, but sent...");
  delay(100);

  client.flush();
  client.stop();
}
