#pragma once
#include <Preferences.h>
#include <WiFi.h>
#include <PZEM004Tv30.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>


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
    StaticJsonDocument<768> doc;
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

    String FECHA_HORA;
    String MAC;
    String type;

    ESP32Time RTC;

  public:
    //Constructor
    Medidor_Consumo(String tipo, ESP32Time rtc);

    //Variables
    structPZEM004 SensorFaseA;
    structPZEM004 SensorFaseB;


    String Texto;


    //Funciones
    void medicion(void);
    String generateString(void);

};
