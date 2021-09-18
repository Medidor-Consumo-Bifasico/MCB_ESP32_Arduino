/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
*/
#include <TaskScheduler.h>

#include "Configuracion.h"
#include "ECUA_CONECT.h"
#include "MC_Sensor.h"

EcuaRed EcuaRed;
Medidor_Consumo MC(Tipo, EcuaRed.RTC);

#include "ECUA_Display.h"
EcuaDisplay EcuaDisplay(OLED);

//LED
int LED_R = 25;
int LED_G = 26;
int LED_B = 27;


void TaskBlink();
void TaskMedicion();

//Control Hilos
Scheduler EcuaScheduler;

Task T_BLK(TASK_MILLISECOND * 250, TASK_FOREVER, &TaskBlink, &EcuaScheduler);
Task T_MEDICION(TASK_SECOND * 60, TASK_FOREVER, &TaskMedicion, &EcuaScheduler);

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  //setup_ECUA_CONECT();

  if (EcuaRed.begin()) {
    T_MEDICION.enable();
    Serial.println("Habilitado");
  }

  //Display
  EcuaDisplay.begin(EcuaRed.Config);

  T_BLK.enable();
}

void loop() {
  EcuaRed.loop();
  EcuaDisplay.loop();
  EcuaScheduler.execute();
}


void TaskBlink() { // This is a task.
  if (EcuaRed.Config) {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
    T_BLK.disable();
    T_MEDICION.disable();
  }
  else if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_R, !digitalRead(LED_R));
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
  }
  else if (!client.connected()) {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, !digitalRead(LED_B));
  }
  else {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
  }
}

void TaskMedicion() { // This is a task.
  char msg[500];
  MC.generateString().toCharArray(msg, 500);
  client.publish("topicPrincipal", msg);
}
