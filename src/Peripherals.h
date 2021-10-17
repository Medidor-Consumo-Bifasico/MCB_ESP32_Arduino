/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/avbazurt/CODIGO-Medidor-Consumo-Electrico-Bifasico
*/

#pragma once
#include <TaskScheduler.h>
#include <WiFi.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DEBUG(texto)            \
    Serial.print("[");          \
    Serial.print(__FILENAME__); \
    Serial.print("]:");         \
    Serial.println(texto);

//PIN
#define BOTON 35
#define LED 25

class Peripheral
{
private:
    TaskHandle_t Blink_handle = NULL;

public:
    bool ModConfig = false;
    bool LastStatus = false;
    long int contador = 0;

    Peripheral()
    {
        xTaskCreatePinnedToCore(
            task_Blink,
            "TaskBlink",
            4024, // This stack size can be checked & adjusted by reading the Stack Highwater
            (void *) &ModConfig,
            2,
            &Blink_handle,
            0);

        xTaskCreatePinnedToCore(
            task_Button,
            "TaskButton",
            4024, // This stack size can be checked & adjusted by reading the Stack Highwater
            (void *) &ModConfig,
            3,
            &Blink_handle,
            0);

        
    }

protected:
    static void task_Blink(void *pvParameters)
    {
        pinMode(LED, OUTPUT);
        for (;;)
        {
            if ((*((bool *)pvParameters))){
                digitalWrite(LED, !digitalRead(LED));
                vTaskDelay(150 / portTICK_PERIOD_MS);
            }

            else if (WiFi.status() != WL_CONNECTED or !client.connected())
            {
                digitalWrite(LED, !digitalRead(LED));
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            else{
                if (digitalRead(LED)==LOW){
                    digitalWrite(LED, HIGH);
                }
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }  
        }
    }

    static void task_Button(void *pvParameters){
        pinMode(BOTON, INPUT_PULLDOWN);
        bool last_state = LOW;
        int contador = 0;
        float tiempo_pulsacion = 0;
        int scala = 200;

        for (;;){
            if (digitalRead(BOTON) == HIGH and last_state == LOW){
                last_state = HIGH;
                contador = 0;
            }
            else if (digitalRead(BOTON) == LOW and last_state == HIGH)
            {
                tiempo_pulsacion = contador*scala;
                tiempo_pulsacion = tiempo_pulsacion/1000;

                DEBUG("Tiempo transcurrido " + String(tiempo_pulsacion));
                last_state = LOW;

                if (tiempo_pulsacion>=5.00){
                    (*((bool *)pvParameters)) = !(*((bool *)pvParameters));
                }
            }

            else if(digitalRead(BOTON) == HIGH and last_state == HIGH){
                contador++;
            }

            vTaskDelay(scala / portTICK_PERIOD_MS);
        }
    }



};