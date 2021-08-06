#include <wiringPi.h>

int main (void)
{
    wiringPiSetup();
    pinMode(0, OUTPUT);
    int i = 0;
    for (i = 0;i<10;i++)
    {
        digitalWrite(0, HIGH);
        delay(500);
        digitalWrite(0, LOW);
        delay(500);
    }
    return 0 ;
}