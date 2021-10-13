#include <Arduino.h>
#include "SYSTEM_CONECT.h"
#include "MC_Sensor.h"

Medidor_Consumo MC;
WebSystem Web;


void setup() {
  Serial.begin(115200);
  Web.setup();
}

void loop() {
  Web.loop();
}