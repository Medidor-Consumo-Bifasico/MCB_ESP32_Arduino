/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/avbazurt/CODIGO-Medidor-Consumo-Electrico-Bifasico
  Sensor PZEM004T
*/

#pragma once
#include <Preferences.h>
#include <WiFi.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DEBUG(texto)          \
  Serial.print("[");          \
  Serial.print(__FILENAME__); \
  Serial.print("]:");         \
  Serial.println(texto);


#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

struct structPZEM004 {
  float voltaje;
  float corriente;
  float potencia;
  float frecuencia;
  float energia;
  float FP;
};


class Medidor_Consumo
{
  private:
    //Variables
    PZEM004Tv30 _faseA;
    PZEM004Tv30 _faseB;

    //Json
    StaticJsonDocument<1600> doc;
    JsonArray sensores;

    JsonObject sensores_0;
    JsonObject sensores_1;
    JsonObject sensores_2;
    JsonObject sensores_3;
    JsonObject sensores_4;
    JsonObject sensores_5;
    JsonObject sensores_6;
    JsonObject sensores_7;
    JsonObject sensores_8;
    JsonObject sensores_9;
    JsonObject sensores_10;
    JsonObject sensores_11;
    JsonObject temperatura;

    String FECHA_HORA;
    String MAC;
    String type;

    ESP32Time RTC;
    void updateJson(void);
    

  public:
    //Constructor
    Medidor_Consumo(String tipo);

    //Variables
    structPZEM004 SensorFaseA;
    structPZEM004 SensorFaseB;


    String text_Json;


    //Funciones
    bool ValidateSensor();

    bool GetAddres();
    void reset(String comando);

    void medicion(void);
    String generateString(void);

};

