/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/TheLast20/CODIGO-Medidor-Consumo-Electrico-Bifasico
*/
#include <TaskScheduler.h>
#include "Configuracion.h"
#include <Preferences.h>

Preferences flash;

#include "MC_Sensor.h"
Medidor_Consumo MC;

#include "ECUA_CONECT.h"
EcuaRed EcuaRed;

#include "ECUA_Display.h"
EcuaDisplay EcuaDisplay(OLED);

//LED RGB
#define LED_R 25
#define LED_G 26
#define LED_B 27

//LED MONOCOLOR
#define LED 25


void TaskBlinkRGB();
void TaskBlinkLED();
void TaskMedicion();

//Control Hilos
Scheduler EcuaScheduler;

Task Task_BlinkRGB(TASK_MILLISECOND * 250, TASK_FOREVER, &TaskBlinkRGB);
Task Task_BlinkLED(TASK_MILLISECOND * 150, TASK_FOREVER, &TaskBlinkLED);

Task T_MEDICION(TASK_SECOND * 60, TASK_FOREVER, &TaskMedicion, &EcuaScheduler);

void setup() {
  Serial.begin(115200);
  MC.begin(Tipo, EcuaRed.RTC);
  
  if (EcuaRed.begin(flash)) {
    T_MEDICION.enable();
    Serial.println("Habilitado");
  }

  //Display
  EcuaDisplay.begin(EcuaRed.Config);

  //LED
  if (INDICADOR == "RGB") {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);

    EcuaScheduler.addTask(Task_BlinkRGB);
    Task_BlinkRGB.enable();
  }

  else if (INDICADOR == "LED") {
    pinMode(LED, OUTPUT);
    EcuaScheduler.addTask(Task_BlinkLED);
    Task_BlinkLED.enable();
  }


  flash.begin("tiempo", false);
  unsigned long tiempo = flash.getULong("periodo",0);
  flash.end();

  if (tiempo > 0){
    T_MEDICION.setInterval(TASK_MILLISECOND * tiempo); 
  }

  T_MEDICION.enable();

}

void loop() {
  EcuaRed.loop();
  EcuaDisplay.loop();
  EcuaScheduler.execute();
}


void TaskBlinkLED(){
  if (EcuaRed.Config) {
    digitalWrite(LED, !digitalRead(LED));
    T_MEDICION.disable();
  }
  
  else if (WiFi.status() != WL_CONNECTED or !client.connected()) {
    digitalWrite(LED, !digitalRead(LED));
    Task_BlinkLED.setInterval(TASK_MILLISECOND * 500); 
  }
  else {
    digitalWrite(LED, HIGH);
  }
}


void TaskBlinkRGB() { // This is a task.
  if (EcuaRed.Config) {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
    Task_BlinkRGB.disable();
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
  char msg[800];
  MC.generateString().toCharArray(msg, 800);
  Serial.println(msg);
  client.publish("topicPrincipal", msg);
}
