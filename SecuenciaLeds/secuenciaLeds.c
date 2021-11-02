/*
 * Secuencia de 4 Leds con el ESP32CAM
 * Autor: Yaír Sánchez (Dálibur)
 * Fecha: 19 de agosto de 2021
 *
 * ----------- Descripción del programa -----------
 * El progama encendera 4 Leds en una secuencia exclusiva ascendente durante un tiempo de 5s,
 * encender en secuencia aditiva durante un tiempo de 5s,
 * enceder en secuencia descendente exclusiva durante un tiempo de  5s y 
 * encender en secuencia descendente aditiva durante 5 segundos.
 * Se repite este orden durante 30s y despues se repite en orden inverso durante 90s, asi 4 veces
 * despues los leds se apagan. Cada estado del led dura 0.2 segundos.
 * Tambien se enviara por MQTT una suma incremental mas el valor en binario de la secuencia de los leds.
 *
 * ----------- Diagrama de los pines ----------
 * Componente     PinESP32CAM     Estados lógicos
 * led 1-------GPIO 4----------On=>HIGH, Off=>LOW
 * led 2-------GPIO 2----------On=>HIGH, Off=>LOW
 * led 3-------GPIO 14----------On=>HIGH, Off=>LOW
 * led 4-------GPIO 15----------On=>HIGH, Off=>LOW
 */

//~~~~~~~~~~ Librerias ~~~~~~~~~~
#include <WiFi.h>  //Libreria para el manejo del WiFi del ESP32CAM
#include <PubSubClient.h> //Libreria para manejar MQTT

//~~~~~~~~~~ Configuracion de los Pines ~~~~~~~~~~
#define LED_STATUS 33 //led de Status GPIO33
#define LED1 4 // led 1 con el GPIO4
#define LED2 2 // led 2 con el GPIO2
#define LED3 14 // led 3 con el GPIO14
#define LED4 15 // led 4 con el GPIO15

//~~~~~~~~~~ Variables Locales ~~~~~~~~~~
int seg=0,ciclo=0, tm=0; 
double timeLast, timeNow; 
char leds; 
char valor[5]; 

//~~~~~~~~~~ Objetos ~~~~~~~~~~
WiFiClient espClient; //Maneja el WiFi
PubSubClient client(espClient); //Maneja la conexion al broker

//~~~~~~~~~~~ Datos del Wi-Fi ~~~~~~~~~~~
#define ssid "************"  // Nombre de la red 
#define password "********"  // Contraseña de la red

//~~~~~~~~~~ Datos para la comunicacion MQTT ~~~~~~~~~~
#define mqtt_servidor "**********"  //IP del broker
#define mqtt_puerto 1883 //Puerto del MQTT
#define clientId "******" //Id de cliente
#define topic "ESP32CAM/Data" //Tema al que se va suscribir

//************************
//****** FUNCIONES *******
//************************
void conectar_wifi();  //Funcion encargada de conectarse al WiFi
void enviar_datos_mqtt(); //Funcion que envia la suma y la cadena con el valor binario de la secuencia de leds
void conectar_Broker();  //Funcion que se conecta al broker
void correr_bits(int,int,bool);  //Funcion que hace en orden el corrimiento de bits
void asc_aditivo(int);
void asc_exclusivo(int);
void des_aditivo(int);
void des_exclusivo(int);
void prender_led(char); //Funcion que prende los leds

void setup() {
  pinMode (LED_STATUS, OUTPUT);// Se configura el pin 33 como salida
  pinMode (LED1, OUTPUT);// Se configura el pin 4 como salida
  pinMode (LED2, OUTPUT);// Se configura el pin 2 como salida
  pinMode (LED3, OUTPUT);// Se configura el pin 14 como salida
  pinMode (LED4, OUTPUT);// Se configura el pin 15 como salida
  //Empezar con los leds apagados
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  conectar_wifi();  //Realiza la conexion por wifi
  client.setServer(mqtt_servidor, mqtt_puerto); //Función que define el servidor MQTT
  conectar_Broker();//Conectar al Broker
  timeLast = millis (); // Inicia el control de tiempo
}

void loop() {
  if(ciclo<4){ //Comprueba que el ciclo solo se haga 4 veces
    timeNow = millis ();  // Seguimiento de tiempo
    if (timeNow - timeLast > 0200) {  //Comprueba que ya paso un segundo
      tm++;
      //Contar los seg
      if(tm%5==0){
        seg++; //Aumenta el contador de los segundos
        printf("seg: %i\n", seg);
      }
      if ((seg%120)<30) { //Comprueba que son los primeros 30 segundos del ciclo
        if(seg%120==0 && tm==0){  //Comprueba que se esta iniciando el ciclo
          ciclo++;  //Aumenta el contador de ciclos
        }
        correr_bits(seg%20,tm%5, 1); //Hace el corrimiento de bits en orden normal
      }else{
        correr_bits((seg-30)%20,tm%5, 0); //Hace el corrimiento de bits en orden inverso
      }
      prender_led(leds); //Prende los leds conrrespondientes
      enviar_datos_mqtt();  //Envia los datos al mqtt
      timeLast = millis (); // Inicia el control de tiempo
    }
  }else{
    //Si ya se ejecuto 4 veces apaga todos los leds
    leds=0b00000000;
    prender_led(leds);
  }
}

void prender_led(char bits){
  if((bits & 0b00001000)==0b00001000){
    digitalWrite(LED1,HIGH);
    valor[0]='1';
  }else{
    digitalWrite(LED1,LOW);
    valor[0]='0';
  }
  if((bits & 0b00000100)==0b00000100){
    digitalWrite(LED2,HIGH);
    valor[1]='1';
  }else{
    digitalWrite(LED2,LOW);
    valor[1]='0';
  }
  if((bits & 0b00000010)==0b00000010){
    digitalWrite(LED3,HIGH);
    valor[2]='1';
  }else{
    digitalWrite(LED3,LOW);
    valor[2]='0';
  }
  if((bits & 0b00000001)==0b00000001){
    digitalWrite(LED4,HIGH);
    valor[3]='1';
  }else{
    digitalWrite(LED4,LOW);
    valor[3]='0';
  }
}

void correr_bits(int s, int i, bool dir){
  //Hace el corrimiento de leds dependiendo de en que segundo dentro del ciclo se encuentre
  if(s<5){
    dir==true?asc_exclusivo(i):des_aditivo(i);
  }else if(s>=5 && s<10){
    dir==true?asc_aditivo(i):des_exclusivo(i);
  }else if(s>=10 && s<15){
    dir==true?des_exclusivo(i):asc_aditivo(i);
  }else if(s>=15 && s<20){
    dir==true?des_aditivo(i):asc_exclusivo(i);
  }
}

void asc_aditivo(int i) {
    if(i==0){
      leds=0b00000000;
    }else if(i==1){
      leds=0b00001000;
    }else{
      leds=((leds>>1)|leds); //Secuencia aditiva ascendente
    }
}
void asc_exclusivo(int i) {
  if(i==1){
    leds=0b00001000;
  }else{
    leds>>=1; //Secuencia exclusiva ascendente
  }
}
void des_aditivo(int i) {
    if(i==0){
      leds=0b00000000;
    }else if(i==1){
      leds=0b00000001;
    }else{
      leds=((leds<<1)|leds); //Secuencia descendente aditiva
    }
}
void des_exclusivo(int i) {
    if(i==1){
      leds=0b00000001;
    }else{
      leds<<=1; //Secuencia descendente exclusiva
    }
}

void conectar_wifi(){
  Serial.begin (115200);  //Inicialización de comunicación serial a 1152000 baudios
  //Imprimir en el monitor serial
  Serial.print("\n\nConectando a ");
  Serial.println(ssid);
  // Iniciar el WiFi
  WiFi.begin(ssid, password); //Esta es la función que realiza la conexión a WiFi
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (LED_STATUS, HIGH); //Se apaga el led de Status
    delay(100); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (LED_STATUS, LOW); //Se enciende el led de Status
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }
}

void enviar_datos_mqtt(){
  if (client.connected()){
    char payload[25];
    String str = "Suma: "+ String(tm)+ "\nValor: "+ valor;
    str.toCharArray(payload, 25);
    client.publish(topic, payload);
  }else{
    conectar_Broker(); //Se conecta a al Broker
    enviar_datos_mqtt();  //Envia los datos al mqtt
  }
}

void conectar_Broker() {
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Intentando conexión Mqtt...");
    //Intentamos conectar
    if (client.connect(clientId)) {
      Serial.println("¡Conectado!");
      // Nos suscribimos
      if(client.subscribe(topic)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    }else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");
			delay(5000);
		}
  }
}