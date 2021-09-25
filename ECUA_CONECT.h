/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  Script Para la conexion y configuracion de ESP32-Medidores Consumo
*/
#pragma once
#include <nvs_flash.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>


#include "ECUA_CONECT_HTML_ERROR.h"
#include "ECUA_CONECT_HTML_MAIN.h"
#include "ECUA_CONECT_HTML_OK.h"

// ---------- Web Server -------------
AsyncWebServer server(80);
const char *ssid_web = "ECUAPLUS_MC";
const char *password_web = "ECUAPLUS";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// ----------- Credenciales MQTT -----------
#define MQTT_MAX_PACKET_SIZE 812
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
      client.subscribe("CONFIG");
      task_ConectMQTT.setInterval(TASK_MILLISECOND * 100);
    }
    else {
      Serial.println("Intentando reconexion en 10 segundos");
      task_ConectMQTT.setInterval(10 * TASK_SECOND);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "CONFIG") {
    int indice = messageTemp.indexOf("/");
    int indice2 = messageTemp.indexOf("/", indice + 1);

    Serial.println(indice);
    Serial.println(indice2);

    if (indice != -1) {
      String MAC_SYSTEM = messageTemp.substring(0, indice);
      String OPCION = messageTemp.substring(indice + 1, indice2);
      String DATO = messageTemp.substring(indice2 + 1, messageTemp.length());

      Serial.print("MAC: ");
      Serial.println(MAC_SYSTEM);

      Serial.print("OPCION: ");
      Serial.println(OPCION);

      Serial.print("DATO: ");
      Serial.println(DATO);

      if (MAC_SYSTEM == WiFi.macAddress()) {
        if (OPCION == "RESET") {
          MC.reset(DATO);
          Serial.println("Resetear");

        }
        else if (OPCION == "FLASH") {
          Serial.println("Flasheado");
          nvs_flash_erase(); // erase the NVS partition and...
          nvs_flash_init(); // initialize the NVS partition.
          ESP.restart();
        }

        else if (OPCION == "TIME") {
          Serial.print("Tiempo puesto: ");
          Serial.println(DATO.toInt());
          
          flash.begin("tiempo", false);
          flash.putULong("periodo", DATO.toInt());
          flash.end();
          
          ESP.restart();
        }





      }
    }
  }
}



class EcuaRed
{
  public:
    //Datos
    String MAC;
    ESP32Time RTC;
    bool Config = false;

    //Memoria
    bool begin(Preferences flash) {
      flash.begin("credenciales", false);
      String wifi_ssid = flash.getString("SSID", "none");
      String wifi_pass = flash.getString("PASS", "none");
      flash.end();

      //Configurar WIFI
      WiFi.disconnect(true);
      delay(1000);

      if (wifi_ssid == "none" or wifi_pass == "none") {
        Serial.println("No hay credenciales");
        Config = true;
        return false;
      }

      wifi_ssid.toCharArray(ssid, 30);
      wifi_pass.toCharArray(password, 30);

      Serial.println("Credenciales: ");
      Serial.println(ssid);
      Serial.println(password);


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
      client.setCallback(callback);

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
        delay(500);

        int conteo = 0;
        Serial.print("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED and conteo < 10) {
          delay(250);
          Serial.print(".");
          conteo++;
        }

        if (WiFi.status() != WL_CONNECTED) {
          Serial.print("Credenciales invalidas");
          request->send(200, "text/html", index_html_error);
          WiFi.disconnect(true);

        }
        else {
          Serial.print("Credenciales validas");

          Preferences flash_data;
          flash_data.begin("credenciales", false);
          flash_data.putString("SSID", String(wifi_network_ssid));
          flash_data.putString("PASS", String(wifi_network_password));
          flash_data.end();

          request->send(200, "text/html", index_html_ok);
          WiFi.disconnect(true);

          delay(3500);
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
