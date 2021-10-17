/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/avbazurt/CODIGO-Medidor-Consumo-Electrico-Bifasico
  Sensor PZEM004T
*/

#include <TaskScheduler.h>
#include <Arduino.h>

#include "MC_Sensor.h"
Medidor_Consumo MC("BIFASICO");

#include "SYSTEM_CONECT.h"
#include "Peripherals.h"
WebSystem Web;
Peripheral Perifericos;

void SendData();
Scheduler MainScheduler;
Task Task_Send_Data(TASK_MILLISECOND * 1000,TASK_FOREVER,&SendData,&MainScheduler,false);


void setup() {
  Serial.begin(115200);

  bool status_sensor = MC.GetAddres();
  //bool status_sensor = true;
  
  if (!status_sensor){
    while (1){}
  }
  
  if (Web.setup()){
    DEBUG("Begin Send Data");

    Web.flash.begin("tiempo", false);
    unsigned long tiempo = Web.flash.getULong("periodo", 1000);
    Web.flash.end();

    Task_Send_Data.setInterval(TASK_MILLISECOND * tiempo); 
    Task_Send_Data.enable();
  }
  else{
    Perifericos.ModConfig = true;
  }
}

void loop() {
  Web.loop(Perifericos.ModConfig);
  MainScheduler.execute();
}

void SendData(){
  if (WiFi.status() == WL_CONNECTED and client.connected()){
    Web.send_MQTT(MC.generateString());
  }
  if (Perifericos.ModConfig){
    DEBUG("End Send Data");
    Task_Send_Data.disable();
  }
}