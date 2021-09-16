#include "MC_Sensor.h"

#if defined(ESP32)
#define RX2 16
#define TX2 17
#elif defined(ESP8266)
#define RX2 15
#define TX2 14
#else
#define RX2 16
#define TX2 17
#endif

Medidor_Consumo::Medidor_Consumo(String tipo, ESP32Time rtc):
  _faseA(Serial2, RX2, TX2, 0x01),
  _faseB(Serial2, RX2, TX2, 0x02)
{

  type = tipo;
  MAC = WiFi.macAddress();
  RTC = rtc;

  SensorFaseA = {0, 0, 0, 0, 0, 0};
  SensorFaseB = {0, 0, 0, 0, 0, 0};




  doc["modelo"] = "MC_" + type;
  sensores = doc.createNestedArray("sensores");

  if (type == "MONOFASICO") {
    sensores_0 = sensores.createNestedObject();
    sensores_1 = sensores.createNestedObject();
    sensores_2 = sensores.createNestedObject();
    sensores_3 = sensores.createNestedObject();
    sensores_4 = sensores.createNestedObject();
    sensores_5 = sensores.createNestedObject();

    //----------VOLTAJE-------------
    sensores_0["nombre"] = "VA";
    sensores_0["valor"] = "0";
    sensores_0["unidadMedicion"] = "V";

    //----------CORRIENTE-------------
    sensores_1["nombre"] = "IA";
    sensores_1["valor"] = "0";
    sensores_1["unidadMedicion"] = "A";

    //----------POTENCIA-------------
    sensores_2["nombre"] = "PA";
    sensores_2["valor"] = "0";
    sensores_2["unidadMedicion"] = "W";

    //----------FRECUENCIA-------------
    sensores_3["nombre"] = "FA";
    sensores_3["valor"] = "0";
    sensores_3["unidadMedicion"] = "Hz";

    //----------ENERGIA-------------
    sensores_4["nombre"] = "EA";
    sensores_4["valor"] = "0";
    sensores_4["unidadMedicion"] = "KwH";

    //----------FP-------------
    sensores_5["nombre"] = "FPA";
    sensores_5["valor"] = "0";
    sensores_5["unidadMedicion"] = "";
  }

  else if (type == "BIFASICO") {
    sensores_0 = sensores.createNestedObject();
    sensores_1 = sensores.createNestedObject();
    sensores_2 = sensores.createNestedObject();
    sensores_3 = sensores.createNestedObject();
    sensores_4 = sensores.createNestedObject();
    sensores_5 = sensores.createNestedObject();
    sensores_6 = sensores.createNestedObject();
    sensores_7 = sensores.createNestedObject();
    sensores_8 = sensores.createNestedObject();
    sensores_9 = sensores.createNestedObject();

    //----------VOLTAJE-------------
    sensores_0["nombre"] = "VA";
    sensores_0["valor"] = "0";
    sensores_0["unidadMedicion"] = "V";

    sensores_1["nombre"] = "VB";
    sensores_1["valor"] = "0";
    sensores_1["unidadMedicion"] = "V";

    //----------CORRIENTE-------------
    sensores_2["nombre"] = "IA";
    sensores_2["valor"] = "0";
    sensores_2["unidadMedicion"] = "A";

    sensores_3["nombre"] = "IB";
    sensores_3["valor"] = "0";
    sensores_3["unidadMedicion"] = "A";

    //----------POTENCIA-------------
    sensores_4["nombre"] = "PA";
    sensores_4["valor"] = "0";
    sensores_4["unidadMedicion"] = "W";

    sensores_5["nombre"] = "PB";
    sensores_5["valor"] = "0";
    sensores_5["unidadMedicion"] = "W";

    //----------FRECUENCIA-------------
    sensores_6["nombre"] = "FA";
    sensores_6["valor"] = "0";
    sensores_6["unidadMedicion"] = "Hz";

    sensores_7["nombre"] = "FB";
    sensores_7["valor"] = "0";
    sensores_7["unidadMedicion"] = "Hz";

    //----------ENERGIA-------------
    sensores_8["nombre"] = "EA";
    sensores_8["valor"] = "0";
    sensores_8["unidadMedicion"] = "KwH";

    sensores_9["nombre"] = "EB";
    sensores_9["valor"] = "0";
    sensores_9["unidadMedicion"] = "KwH";

    //----------FP-------------
    sensores_10["nombre"] = "FPA";
    sensores_10["valor"] = "0";
    sensores_10["unidadMedicion"] = "";

    sensores_11["nombre"] = "FPB";
    sensores_11["valor"] = "0";
    sensores_11["unidadMedicion"] = "";
  }
}


void Medidor_Consumo::medicion(void) {
  SensorFaseA.voltaje = _faseA.voltage();
  SensorFaseA.corriente = _faseA.current();
  SensorFaseA.potencia = _faseA.power();
  SensorFaseA.frecuencia = _faseA.frequency();
  SensorFaseA.energia = _faseA.energy();
  SensorFaseA.FP = _faseA.pf();

  if (isnan(SensorFaseA.voltaje)) {
    SensorFaseA.voltaje = -1;
  }
  if (isnan(SensorFaseA.corriente)) {
    SensorFaseA.corriente = -1;
  }
  if (isnan(SensorFaseA.potencia)) {
    SensorFaseA.potencia = -1;
  }
  if (isnan(SensorFaseA.frecuencia)) {
    SensorFaseA.frecuencia = -1;
  }
  if (isnan(SensorFaseA.energia)) {
    SensorFaseA.energia = -1;
  }
  if (isnan(SensorFaseA.FP)) {
    SensorFaseA.FP = -1;
  }

  if (type == "BIFASICO") {
    SensorFaseB.voltaje = _faseB.voltage();
    SensorFaseB.corriente = _faseB.current();
    SensorFaseB.potencia = _faseB.power();
    SensorFaseB.frecuencia = _faseB.frequency();
    SensorFaseB.energia = _faseB.energy();
    SensorFaseB.FP = _faseB.pf();

    if (isnan(SensorFaseB.voltaje)) {
      SensorFaseB.voltaje = -1;
    }
    if (isnan(SensorFaseB.corriente)) {
      SensorFaseB.corriente = -1;
    }
    if (isnan(SensorFaseB.potencia)) {
      SensorFaseB.potencia = -1;
    }
    if (isnan(SensorFaseB.frecuencia)) {
      SensorFaseB.frecuencia = -1;
    }
    if (isnan(SensorFaseB.energia)) {
      SensorFaseB.energia = -1;
    }
    if (isnan(SensorFaseB.FP)) {
      SensorFaseB.FP = -1;
    }
  }
}

String Medidor_Consumo::generateString(void) {
  //Actualizo datos del json

  FECHA_HORA = RTC.getTime("#%Y-%m-%d#%H:%M:%S#");

  medicion();

  Texto = "";

  if (type == "MONOFASICO") {
    sensores_0["valor"] = String(SensorFaseA.voltaje);
    sensores_1["valor"] = String(SensorFaseA.corriente);
    sensores_2["valor"] = String(SensorFaseA.potencia);
    sensores_3["valor"] = String(SensorFaseA.frecuencia);
    sensores_4["valor"] = String(SensorFaseA.energia);
    sensores_5["valor"] = String(SensorFaseA.FP);
  }

  else if (type == "BIFASICO") {
    sensores_0["valor"] = String(SensorFaseA.voltaje);
    sensores_1["valor"] = String(SensorFaseB.voltaje);

    sensores_2["valor"] = String(SensorFaseA.corriente);
    sensores_3["valor"] = String(SensorFaseB.corriente);

    sensores_4["valor"] = String(SensorFaseA.potencia);
    sensores_5["valor"] = String(SensorFaseB.potencia);

    sensores_6["valor"] = String(SensorFaseA.frecuencia);
    sensores_7["valor"] = String(SensorFaseB.frecuencia);

    sensores_8["valor"] = String(SensorFaseA.energia);
    sensores_9["valor"] = String(SensorFaseB.energia);

    sensores_10["valor"] = String(SensorFaseA.FP);
    sensores_11["valor"] = String(SensorFaseB.FP);
  }

  serializeJson(doc, Texto);

  return MAC + FECHA_HORA + Texto;
}
