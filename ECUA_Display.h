/*
  Autor: Vidal Bazurto (avbazurt@espol.edu.ec)
  GitHub: https://github.com/TheLast20/CODIGO-Medidor-Consumo-Electrico-Bifasico
  Sistemas de display
*/

#pragma once
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ECUA_Display_Imagenes.h"

//********** DISPLAY **********
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//********** PIN **********
#define BOTON 35

bool Menu_Config = false;

int Num_Menu = 1;
bool SubMenu = false;


//Osciloscopio
int anteriorMillis = 0;
int tiempo = 0;

float potencia = 0;
float graficaPotencia = 0;

int x[128]; //buffer de la grafica
int y[128]; //buffer secundario de la grafica


Scheduler DisplayScheduler;

void TaskBoton(void);
void TaskMenu(void);

void TaskMediciones(void);
void TaskGrafica(void);
void TaskConfiguracion(void);

Task task_Boton(TASK_MILLISECOND * 50, TASK_FOREVER, &TaskBoton, &DisplayScheduler);
Task task_Menu(TASK_MILLISECOND * 500, TASK_FOREVER, &TaskMenu, &DisplayScheduler);

Task task_Mediciones(TASK_MILLISECOND * 500, TASK_FOREVER, &TaskMediciones, &DisplayScheduler);
Task task_Grafica(TASK_MILLISECOND * 500, TASK_FOREVER, &TaskGrafica, &DisplayScheduler);


String IP;


void TaskBoton(void) {
  if (digitalRead(BOTON) == HIGH) {
    int t1 = millis();
    while (digitalRead(BOTON) == HIGH) {
    }
    int t = millis() - t1;
    if (t > 50) {
      if (t > 1200) {
        Serial.println("Pulso Largo");
        if (OLED == false) {
          if (Menu_Config){
            ESP.restart();
          }
          else{
            TaskConfiguracion();
            OLED = true;
          }
          
        }

        else if (not(SubMenu)) {
          if (Num_Menu == 1) {
            task_Menu.disable();
            task_Mediciones.enable();
            SubMenu = true;
          }
          else if (Num_Menu == 2) {
            task_Menu.disable();

            display.setTextSize(1);  // ajusta el tamaño de texto en el minimo valor


            //llenamos las matrices con un valor fuera del rango de medicion >1023
            for (int i = 127; i >= 0; i--) {
              x[i] = 9999;
            }
            for (int i = 127; i >= 0; i--) {
              y[i] = 9999;
            }

            task_Grafica.enable();
            SubMenu = true;
          }

          else if (Num_Menu == 3) {
            task_Menu.disable();
            TaskConfiguracion();
          }
        }

        else {
          task_Mediciones.disable();
          task_Grafica.disable();
          task_Menu.enable();
          SubMenu = false;
        }
      }
      else {
        Serial.println("Pulso Corto");
        Num_Menu += 1;
        if (Num_Menu >= 4) {
          Num_Menu = 1;
        }
      }
    }
  }
}

void TaskMenu(void) {
  if (Num_Menu == 1) {
    display.clearDisplay();
    display.drawBitmap(0, 0, epd_bitmap_multimeter, 128, 64, WHITE);
    display.display();
  }

  else if (Num_Menu == 2) {
    display.clearDisplay();
    display.drawBitmap(0, 0, epd_bitmap_osciloscopio, 128, 64, WHITE);
    display.display();
  }

  else if (Num_Menu == 3) {
    display.clearDisplay();
    display.drawBitmap(0, 0, epd_bitmap_CONFIGURATION, 128, 64, WHITE);
    display.display();
  }
}


void TaskMediciones(void) {
  display.clearDisplay();
  MC.medicion();
  display.setCursor(0, 0);
  display.setTextSize(1.9);      // Escala 2:1 texto
  display.println("Mediciones:");
  display.println("");

  display.setTextSize(1.15);      // Escala 2:1 texto

  display.print("Voltaje: ");
  display.print(MC.SensorFaseA.voltaje);
  display.println("[V]");

  display.print("Corriente: ");
  display.print(MC.SensorFaseA.corriente);
  display.println("[A]");

  display.print("Potencia: ");
  display.print(MC.SensorFaseA.potencia);
  display.println(" [W]");

  display.print("Frecuencia: ");
  display.print(MC.SensorFaseA.frecuencia);
  display.println("[Hz]");


  display.print("Energia: ");
  display.print(MC.SensorFaseA.energia);
  display.println("[Kw/H]");

  display.print("FP: ");
  display.println(MC.SensorFaseA.FP);
  display.display();
}


void TaskGrafica(void) {
  display.clearDisplay(); //limpia el buffer del display

  //dibuja escala
  display.setCursor(0, 0);
  display.print(F("500W"));
  display.setCursor(0, 11);
  display.print(F("400W"));
  display.setCursor(0, 22);
  display.print(F("300W"));
  display.setCursor(0, 32);
  display.print(F("200W"));
  display.setCursor(0, 43);
  display.print(F("100W"));

  display.drawLine(25, 0, 35, 0, WHITE);
  display.drawLine(25, 11, 35, 11, WHITE);
  display.drawLine(25, 22, 35, 22, WHITE);
  display.drawLine(25, 32, 35, 32, WHITE);
  display.drawLine(25, 43, 35, 43, WHITE);
  //dibuja eje X y Y
  display.drawLine(0, 53, 127, 53, WHITE);
  display.drawLine(25, 53, 25, 0, WHITE);



  MC.medicion();
  potencia = MC.SensorFaseA.potencia; //calcula el valor en voltaje
  graficaPotencia = map(potencia, 0, 500, 53, 0); //escala el valor analogico a un pixel imprimible en pantalla

  x[127] = graficaPotencia; //asigna el valor escalado a el ultimo dato de la matriz

  for (int i = 127; i >= 25; i--) {
    display.drawPixel(i, x[i], WHITE); //dibuja punto a punto el contenido de x
    y[i - 1] = x[i]; //guarda la informacion desplazada una posicion temporalmente en y
  }
  //imprime el voltaje en texto
  display.setCursor(25, 57);
  display.print(potencia);
  display.print(F("W "));

  //imprime el tiempo que se tarda en tomar y mostrar cada dato
  tiempo = millis() - anteriorMillis;
  display.print(MC.SensorFaseA.energia);
  anteriorMillis = millis();
  display.print(F("Kw/H"));

  display.display(); //despliega la informacion del buffer en la pantalla

  //delay(60); //pausa opcional

  for (int i = 127; i >= 0; i--) {
    x[i] = y[i]; //envia los datos desplazados de vuelta a la variable x
  }

}


void TaskConfiguracion(void) {
  Serial.println("configurate");
  String dato = EcuaRed.config();
  Serial.println(dato);
  if (OLED) {
    display.clearDisplay();  // limpia el buffer del display
    display.setTextSize(2);  // ajusta el tamaño de texto en el minimo valor
    display.setCursor(0, 0);
    display.println("Ip: ");
    display.println("");
    display.setTextSize(1);  // ajusta el tamaño de texto en el minimo valor
    display.println(dato);
    display.display();

  }



}

class EcuaDisplay
{
  public:
    bool OLED;

    EcuaDisplay(bool oled) {
      OLED = oled;
    }

    void begin(bool configurate) {
      Serial.println(configurate);
      pinMode(BOTON, INPUT_PULLDOWN);
      DisplayScheduler.addTask(task_Boton);
      task_Boton.enable();

      if (OLED) {
        Wire.begin();
        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
          Serial.println(F("SSD1306 allocation failed"));
          OLED = false;
          return;
        }
        display.setTextColor(SSD1306_WHITE); // Dibujamos en color Blanco
        display.cp437(true);
        display.setRotation(0);  // se escoje la orientacion del display puede ser 0 o 2
        display.dim(true); //dejamos el brillo en maximo

        delay(500);
        display.clearDisplay();


        //animacion inicial
        for (int i = 0; i < 1; i++) {
          display.clearDisplay();  // limpia el buffer del display
          display.drawBitmap(0, 0, epd_bitmap_ECUA, 128, 64, 1); // despliega imagen
          display.display(); // muestra la informacion del buffer en la pantalla
          delay(3000); //pausa
          display.fillRect(0, 0, 128, 64, 1);
          display.drawBitmap(0, 0, epd_bitmap_ECUA, 128, 64, 0); // despliega imagen en negativo
          display.display(); // muestra la informacion del buffer en la pantalla
          delay(1000); //pausa
          display.clearDisplay(); //limpia buffer
          display.display(); //muestra el buffer limpio
        }
        display.display();

        if (configurate) {
          TaskConfiguracion();
        }
        else {
          task_Menu.enable();
        }
      }

      else {
        if (configurate) {
          TaskConfiguracion();
        }

      }
    }

    void loop(void) {
      DisplayScheduler.execute();
    }
};
