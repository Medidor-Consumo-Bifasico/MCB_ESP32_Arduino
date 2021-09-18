/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  Script Para la conexion y configuracion de ESP32-Medidores Consumo
*/
#pragma once
#include <EEPROM.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ---------- Web Server -------------
AsyncWebServer server(80);
const char *ssid_web = "ECUAPLUS_MC";
const char *password_web = "ECUAPLUS";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";


// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ECUAPLUS</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2> ECUAPLUS - MC<h2>
  <h3> Credenciales WIFI</h3>
  <form action="/get">
    SSID    : <input type="text" name="input1"><br>
    PASSWORD: <input type="text" name="input2"><br>
    <input type="submit" value="Submit">
  </form><br>
  
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


// ----------- Credenciales MQTT -----------
#define MQTT_MAX_PACKET_SIZE 512
WiFiClient espClient;
PubSubClient client(espClient);

String clientId;
char *mqtt_server = "147.182.236.61"; ;
int mqtt_port = 1883;

char *mqtt_user = "ideTSer";
char *mqtt_pass = "secReTex4@m#";


// ----------- RTC -----------
ESP32Time RTC;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5 * 3600;
const int   daylightOffset_sec = 0;

// ----------- Credenciales WIFI -----------
#define EEPROM_SIZE 512

char flash[20];
char ssid[30];
char password[30];

// ------------------ HILOS -----------------------
Scheduler RedScheduler;

void ConectMQTT(void);
Task task_ConectMQTT( TASK_MILLISECOND * 500, TASK_FOREVER, &ConectMQTT, &RedScheduler);


void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Conectado");
  delay(500);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    RTC.setTimeStruct(timeinfo);
  }
  Serial.println("RTC Actualizado");
  delay(500);
  task_ConectMQTT.enable();
  Serial.println("Activado");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  task_ConectMQTT.disable();
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.disconnect();
  WiFi.reconnect();
  delay(5000);
}

void ConectMQTT(void) {
  if (!client.connected() and WiFi.status() == WL_CONNECTED) {
    Serial.print("Intentando conexión Mqtt...");

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!");
      task_ConectMQTT.setInterval(TASK_MILLISECOND * 100);
    }
    else {
      Serial.println("Intentando reconexion en 10 segundos");
      task_ConectMQTT.setInterval(10 * TASK_SECOND);
    }
  }
}


class EcuaRed
{
  public:
    String MAC;
    ESP32Time RTC;
    bool Config = false;

    bool begin() {
      EEPROM.begin(EEPROM_SIZE);
      EEPROM.get(20, flash);
      EEPROM.get(40, ssid);
      EEPROM.get(70, password);
      EEPROM.end();

      Serial.println("Credenciales: ");
      Serial.println(ssid);
      Serial.println(password);

      //Configurar WIFI
      WiFi.disconnect(true);
      delay(1000);

      if (String(flash) != "listo") {
        Serial.println("No hay credenciales");
        Config = true;

        return false;
      }

      WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);
      WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(ssid, password);

      delay(3000);

      MAC = WiFi.macAddress();
      clientId = MAC;

      //Configurar MQTT
      client.setServer(mqtt_server, mqtt_port);
      client.setBufferSize(MQTT_MAX_PACKET_SIZE);

      //Activamos las credenciales
      task_ConectMQTT.enable();

      return true;
    }


    String config(void) {
      Config = true;

      task_ConectMQTT.disable();
      client.disconnect();
      WiFi.disconnect(true);
      delay(2000);

      WiFi.mode(WIFI_MODE_APSTA);

      WiFi.softAP(ssid_web, password_web);
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      Serial.println(myIP.toString());

      char wifi_network_ssid[30];
      char wifi_network_password[30];

      // Send web page with input fields to client
      server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", index_html);
      });

      // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
      server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {

        char wifi_network_ssid[30];
        char wifi_network_password[30];

        request->getParam(PARAM_INPUT_1)->value().toCharArray(wifi_network_ssid, 30);
        request->getParam(PARAM_INPUT_2)->value().toCharArray(wifi_network_password, 30);

        Serial.print("SSID: ");
        Serial.println(wifi_network_ssid);
        Serial.print("PASS: ");
        Serial.println(wifi_network_password);

        WiFi.begin(wifi_network_ssid, wifi_network_password);

        int conteo = 0;

        while (WiFi.status() != WL_CONNECTED and conteo < 4) {
          delay(1000);
          Serial.println("Connecting to WiFi..");
          conteo++;
        }

        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Credenciales invalidas");
          request->send(200, "text/html", "No se pudo conectar a la red establecida intentelo de nuevo<br><a href=\"/\">Return to Home Page</a>");
          WiFi.disconnect(true);

        }
        else {
          Serial.println("Credenciales validas");

          EEPROM.begin(EEPROM_SIZE);

          char flash_STATE[20];
          EEPROM.get(20, flash_STATE);

          if (String(flash_STATE) != "listo") {
            EEPROM.put(20, "listo");
          }

          EEPROM.put(40, wifi_network_ssid);
          EEPROM.put(70, wifi_network_password);
          EEPROM.commit();
          EEPROM.end();


          request->send(200, "text/html", "Conexion exitosa, guardando credenciales <br>");

          ESP.restart();

        }



      });
      server.onNotFound(notFound);
      server.begin();


      Serial.println("Server started");

      return myIP.toString();
    }


    void loop() {
      RedScheduler.execute();
      client.loop();
    }
};
