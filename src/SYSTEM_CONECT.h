/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/avbazurt/CODIGO-Medidor-Consumo-Electrico-Bifasico
  Conexiones ESP32 - Web Server
*/

#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <TaskScheduler.h>
#include <nvs_flash.h>
#include <ESP32Time.h>
#include <Preferences.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DEBUG(texto)          \
  Serial.print("[");          \
  Serial.print(__FILENAME__); \
  Serial.print("]:");         \
  Serial.println(texto);

#define DEBUG_PRINTF()        \
  Serial.print("[");          \
  Serial.print(__FILENAME__); \
  Serial.print("]:");

// ----------- Credenciales MQTT -----------
WiFiClient espClient;
PubSubClient client(espClient);

#define MQTT_MAX_PACKET_SIZE 812
#define mqtt_server "147.182.236.61"
#define mqtt_port 1883
#define mqtt_user "ideTSer"
#define mqtt_pass "secReTex4@m#"

// ----------- WEB SERVER -----------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WebSocketsServer websockets(81);

class WebSystem
{
private:
  TaskHandle_t WIFI_handle = NULL;

  bool WiFI_Begin = false;

  void setup_web_server()
  {
    if (!webServer)
    {
      DEBUG("INICIANDO WEB SERVER CONFIGURACION");
      if (WiFI_Begin)
      {
        vTaskDelete(WIFI_handle);
      }

      task_ConectMQTT.disable();
      client.disconnect();

      webServer = true;

      WiFi.disconnect(true);
      WiFi.mode(WIFI_MODE_APSTA);
      delay(500);

      MAC = WiFi.macAddress();

      String NAME = "MCB-100A-" + MAC.substring(15, 17) + "B";
      while (!(WiFi.softAP(NAME.c_str())))
      {
      };

      DEBUG(WiFi.softAPIP());

      // Route for root / web page
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html"); });

      // Route to load style.css file
      server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });

      // Route js

      server.on("/fondo.jpg", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/fondo.jpg", "image/jpg"); });

      server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/favicon.ico"); });

      server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/main.js"); });

      server.on("/index_error.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index_error.html"); });

      server.on("/index_ok.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index_ok.html"); });

      // Start server
      server.begin(); // it will start webserver
      websockets.begin();
      websockets.onEvent(webSocketEvent);
    }
  }

public:
  WebSystem() : task_ConectMQTT(TASK_MILLISECOND * 500, TASK_FOREVER, &ConectMQTT, &RedScheduler)
  {
  }

  Preferences flash;

  //Constantes Web Server
  bool config_native = false;
  bool webServer = false;

  //Hilos
  Scheduler RedScheduler;
  Task task_ConectMQTT;

  //Constante Sistema
  String MAC;

  //Funciones
  bool setup()
  {
    // Initialize SPIFFS
    if (!SPIFFS.begin())
    {
      DEBUG("An Error has occurred while mounting SPIFFS");
      while (1)
      {
      }
    }

    char ssid[40];
    char password[40];

    //------------- CREDENCIALES ----------
    flash.begin("credenciales", false);
    flash.getBytes("SSID", ssid, 40);
    flash.getBytes("PASS", password, 40);
    flash.end();

    if ((strlen(ssid) == 0) or (strlen(password) == 0))
    {
      DEBUG("NO EXISTEN CREDENCIALES");
      config_native = true;
      return false;
    }

    //WIFI
    xTaskCreatePinnedToCore(
        TaskWIFI, "TaskWIFI" // A name just for humans
        ,
        4024 // This stack size can be checked & adjusted by reading the Stack Highwater
        ,
        (void *)&task_ConectMQTT, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,
        &WIFI_handle, 0);

    WiFI_Begin = true;
    delay(500);
    MAC = WiFi.macAddress();

    //Configurar MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setBufferSize(MQTT_MAX_PACKET_SIZE);
    client.setCallback(callback);

    return true;
  }

  void send_MQTT(String text)
  {
    client.publish("topicPrincipal", text.c_str());
  }

  void loop(bool mod_configurate)
  {
    RedScheduler.execute();
    client.loop();
    if (mod_configurate)
    {
      setup_web_server();
      websockets.loop();
    }

    else if (!mod_configurate and webServer and !config_native)
    {
      DEBUG("DESACTIVANDO WEB SERVER CONFIGURACION");
      websockets.disconnect(true);
      delay(1000);
      WiFi.softAPdisconnect(true);
      delay(1000);
      ESP.restart();
    }
  }

  static void ConectMQTT(void)
  {
    if (!client.connected() and WiFi.status() == WL_CONNECTED)
    {
      DEBUG("Intentando conexión Mqtt...");

      if (client.connect(WiFi.macAddress().c_str(), mqtt_user, mqtt_pass))
      {
        DEBUG("MQTT Conectado!");
        client.subscribe("CONFIG");
        //task_ConectMQTT.setInterval(TASK_MILLISECOND * 100);
      }
      else
      {
        DEBUG("Intentando reconexion MQTT en 10 segundos");
        //task_ConectMQTT.setInterval(10 * TASK_SECOND);
      }
    }
  }

  static void callback(char *topic, byte *message, unsigned int length)
  {
    /*
        Serial.print("Message arrived on topic: ");
        Serial.print(topic);
        Serial.print(". Message: ");
        */
    String messageTemp;
    for (int i = 0; i < length; i++)
    {
      messageTemp += (char)message[i];
    }

    DEBUG("MENSAJE RECIBIDO");
    DEBUG("TOPICO " + String(topic) + " MENSAJE " + String(messageTemp));

    if (String(topic) == "CONFIG")
    {
      int indice = messageTemp.indexOf("/");
      int indice2 = messageTemp.indexOf("/", indice + 1);

      if (indice != -1)
      {
        String MAC_SYSTEM = messageTemp.substring(0, indice);
        String OPCION = messageTemp.substring(indice + 1, indice2);
        String DATO = messageTemp.substring(indice2 + 1, messageTemp.length());

        DEBUG("MAC " + MAC_SYSTEM);
        DEBUG("OPCION " + OPCION);
        DEBUG("DATO " + DATO);

        if (MAC_SYSTEM == WiFi.macAddress())
        {
          if (OPCION == "RESET")
          {
            MC.reset(DATO);
          }
          else if (OPCION == "FLASH")
          {
            DEBUG("Flasheado");
            nvs_flash_erase(); // erase the NVS partition and...
            nvs_flash_init();  // initialize the NVS partition.
            ESP.restart();
          }

          else if (OPCION == "TIME")
          {
            DEBUG_PRINTF();
            Serial.print("Tiempo puesto: ");
            Serial.println(DATO.toInt());

            Preferences flash;
            flash.begin("tiempo", false);
            flash.putULong("periodo", DATO.toInt());
            flash.end();

            ESP.restart();
          }
        }
      }
    }
  }

  static void TaskWIFI(void *pvParameters)
  {
    Preferences flash_wifi;
    char ssid[40];
    char password[40];

    flash_wifi.begin("credenciales", false);
    flash_wifi.getBytes("SSID", ssid, 40);
    flash_wifi.getBytes("PASS", password, 40);
    flash_wifi.end();

    bool last_state = true; //Referencia si anteriormente estaba desconectado
    ESP32Time RTC;

    // ----------- RTC -----------
    const char *ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = -5 * 3600;
    const int daylightOffset_sec = 0;

    // ---------- PUNTERO -------
    WebSystem *p = static_cast<WebSystem *>(pvParameters);

    for (;;) // A Task shall never return or exit.
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (last_state)
        {
          DEBUG("WIFI CONECTADO");
          configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
          struct tm timeinfo;
          if (getLocalTime(&timeinfo))
          {
            RTC.setTimeStruct(timeinfo);
          }
          DEBUG("RTC ACTUALIZADO");
          //p->StartTask();
          last_state = false;
          (*((Task *)pvParameters)).enable();
        }
        continue;
      }

      if (last_state)
      {
        last_state = true;
        (*((Task *)pvParameters)).disable();
      }

      DEBUG("INTENTANDO CONEXION A WIFI");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);

      int contador = 0;
      while (WiFi.status() != WL_CONNECTED && contador < 20)
      {
        contador++;
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      if (WiFi.status() != WL_CONNECTED)
      {
        DEBUG("FALLO CONEXION, REITENTO EN 20 SEGUNDOS");
        vTaskDelay(20000 / portTICK_PERIOD_MS);
        continue;
      }
    }
  }

  static void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
  {
    switch (type)
    {
    case WStype_DISCONNECTED:
      DEBUG("Cliente Desconectado!");
      break;
    case WStype_CONNECTED:
    {
      IPAddress ip = websockets.remoteIP(num);
      DEBUG_PRINTF();
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    }
    break;
    case WStype_TEXT:
      DEBUG_PRINTF();
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char *)(payload));
      DEBUG(message);

      // ----------- DATA WEB SERVER --------
      StaticJsonDocument<520> json;
      String JSON;

      DynamicJsonDocument doc(200);
      DeserializationError error = deserializeJson(doc, message);

      if (error)
      {
        DEBUG("DeserializeJson() failed: " + String(error.c_str()));
        return;
      }

      if (doc["OPCION"] == "update")
      {
        int n = WiFi.scanNetworks();
        if (n > 0)
        {
          DEBUG(String(n) + " networks found");

          json.clear();
          JSON = "";
          json["OPCION"] = "CLAVES";
          JsonArray SSID = json.createNestedArray("SSID");

          for (int i = 0; i < n; ++i)
          {
            String texto = String(WiFi.SSID(i));
            SSID.add(texto);
            delay(10);
          }
          serializeJson(json, JSON);
          websockets.broadcastTXT(JSON);
        }
      }

      else if (doc["OPCION"] == "ssid")
      {
        String ssid = doc["SSID"];
        String pass = doc["PASS"];
        DEBUG("SSID_WIFI: " + ssid);
        DEBUG("PASS_WIFI: " + pass);

        WiFi.disconnect(true);
        delay(500);
        WiFi.begin(ssid.c_str(), pass.c_str());
        delay(500);

        int conteo = 0;

        DEBUG_PRINTF();
        Serial.print("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED and conteo < 30)
        {
          delay(500);
          Serial.print(".");
          conteo++;
        }
        Serial.println();

        json.clear();
        JSON = "";
        json["OPCION"] = "PAGINA";

        if (WiFi.status() != WL_CONNECTED)
        {
          Serial.print("Credenciales invalidas");
          json["STATUS"] = "FALLO";
          serializeJson(json, JSON);
          websockets.broadcastTXT(JSON);
        }

        else
        {
          DEBUG("Credenciales validas");
          json["STATUS"] = "OK";
          serializeJson(json, JSON);
          websockets.broadcastTXT(JSON);
          delay(4000);
          Preferences flash;
          flash.begin("credenciales", false);
          flash.putBytes("SSID", ssid.c_str(), 40);
          flash.putBytes("PASS", pass.c_str(), 40);
          flash.end();
          delay(3000);
          ESP.restart();
        }
      }
    }
  }
};
