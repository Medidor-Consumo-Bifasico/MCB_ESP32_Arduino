/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  Sensor PZEM004T
*/

#pragma once
#include <Preferences.h>
#include <WiFi.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>

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
    Medidor_Consumo();

    //Variables
    structPZEM004 SensorFaseA;
    structPZEM004 SensorFaseB;


    String text_Json;


    //Funciones
    void begin(String tipo, ESP32Time rtc);
    void reset(String comando);

    void address(PZEM004Tv30 sensor);
    void medicion(void);
    String generateString(void);

};
