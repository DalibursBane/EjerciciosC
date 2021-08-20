/*
    Autor: Jorge Isur Balderas Ramírez
    Fecha: 7 de agosto de 2021
    Descripción: Manipula los pines GPIO para encender un led, compilado en una 
    computadora con arquitectura x86_64 para un dispositivo arm64 raspberry pi 3 model B+
    GPIO4--->Pin7
*/
#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <unistd.h>

//Definición del PIN de acuerdo al broadcom
#define LED 4

int main(void)
{
    printf("Hola samsung innovation campus!\n");
    if (gpioInitialise()<0) return 1;
    gpioSetMode(LED, PI_OUTPUT);
    for (int i = 0; i < 10; i++)
    {
        gpioWrite(LED, 1); //on
        printf("Hola\n");
        sleep(1);  //seconds
        gpioWrite(LED,0);  //off
        printf("Adios\n");
        sleep(1);
    }
    gpioTerminate();
}
