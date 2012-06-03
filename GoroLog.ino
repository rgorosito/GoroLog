/*
 
 This example code is in the public domain.
 */
 
// estos vienen definidos en el master
#define LASTCHANNEL 8
#define LASTMOTOR 4

#include <SD.h>
#include "GoroLog.h"
#include "Logger.h"

#define PINLED 8    // PB0(ICP) - pata 14 del 328p
#define PINBOTON 7  // PD7(AIN1) - pata 13 del 328p
//#define BAUDIOS 115200  <--- a esta velocidad patina de vez en cuando
#define BAUDIOS 57600

// TODO : puede ser alverre estos dos:
#define ANALOG_BAT A1  // PC1(ADC1) - pata 24 del 328p
#define ANALOG_CUR A2  // PC2(ADC2) - pata 25 del 328p

// CONFIG

#define LOG_BATA 1
//#define SEND_STATUS 1

//#define DEBUG 1

// sensores
int estadoBoton;
int bat;
int cur;
// para el loop
unsigned long previous_time;
unsigned long current_time;
unsigned long delta_time;
byte frame_counter=0;
// objetos
Logger logger;
// para el dato del comando
binary_data_t bd;
// para contar lo ocupado
long veces_esperando;

void setup() {                
  
  //
  veces_esperando=0;

  // inicializo el serie
  Serial.begin(BAUDIOS);

#ifdef DEBUG
  Serial.println("M:Iniciando");
#endif

  // initialize the digital pin as an output.
  //  pinMode(PINLED, OUTPUT);
  //  pinMode(PINBOTON, INPUT);

  // inicializacion de sd y demás yerbas
  logger.init(PINLED,PINBOTON);

  // leo los primeros valores analógicos (TODO: para que?)
  bat=analogRead(ANALOG_BAT);
  cur=analogRead(ANALOG_CUR);

  // lo último a setupear es previous_time porque lo siguiente es el loop
  previous_time=millis();

}

void loop() {
  
  // leemos el serie
  int c = lee_byte_serie();
    
    #ifdef DEBUG
      Serial.print("COMANDO:");
      Serial.write(c);
      Serial.println();
    #endif
    
    // interpretacion del comando
    
    switch ( c ) {
      case GOROL_COMPASS: // datos del magnetometro
        lee_bytes(sizeof(compass_data_t));
        logger.log_compass(&(bd.compass)); 
        break;
      case GOROL_MOTORES_ARMADOS:
        //motores armados
        logger.motores_armados();
        break;
      case GOROL_MOTORES_DESARMADOS:
        //motores desarmados
        logger.motores_desarmados();
        break;
      case GOROL_QUERY_ESTADO:
        Serial.write(logger.estado());
        break;
      case GOROL_UPTIME:
        lee_bytes(4);
        logger.log_uptime(bd.largo_sin_signo);
        break;
      case GOROL_GPS:
        lee_bytes(sizeof(gps_data_t));
        logger.log_gps(&(bd.myGps));
        break;
      default: //comando no interpretado
        inner_loop();
        
   }


}

void lee_bytes(const uint8_t cantidad) { //FIXME: no limito la cantidad

   uint8_t offset=0;
   
  #ifdef DEBUG
    Serial.print("POR LEER ");
    Serial.print(cantidad,DEC);
    Serial.println(" BYTES");
  #endif


  // mientas tenga datos por leer, leo
  while ( cantidad > offset ) {
    
    bd.dato[offset]=lee_byte_serie();
    offset++;
    
  }
  
  #ifdef DEBUG
    Serial.println("LEIDOS");
  #endif
  
}

int lee_byte_serie() {
  
  int c = Serial.read();
 
  veces_esperando--;
  
  // para asegurarme que aunque saturado, genera log igual
  inner_loop();
     
  while ( c==-1 ) {
    
     veces_esperando++;
     inner_loop();
     c = Serial.read();

  }
 
 return c; 
  
}


void inner_loop() {
  
    // loops
  current_time = millis();
  delta_time = current_time - previous_time;

  // main loop in 8Hz
  if ( delta_time > 125 ) {

    //previous_time=previous_time+125;
    previous_time = current_time;

    // ciclo de 8Hz --------------------------
    {
      
      logger.log_veces_esperando(veces_esperando);
      veces_esperando=0;
      
      logger.check_button();
      logger.actualiza_led(frame_counter);

    #ifdef LOG_BATA
      // levanto el estado de bat
      bat=analogRead(ANALOG_BAT);
      cur=analogRead(ANALOG_CUR);
      // y logeo
      logger.log_bat(bat,cur, frame_counter); //ToDo: pasar punteros?
    #endif

    }

    // ciclo de 4Hz --------------------------
    if ( frame_counter % 2 == 0 ) {
//      logger.change_led(frame_counter>>1);
    } 

    // ciclo de 2Hz --------------------------
    if ( frame_counter % 4 == 0 ) {
      //
    } 

    // ciclo de 1Hz --------------------------
    if ( frame_counter % 8 == 0 ) {
     #ifdef SEND_STATUS
      // le envio al shield el estado
      Serial.write(logger.estado());
     #endif
    }

    // actualiza el contador de frames    
    if ( frame_counter == 7 ) {
      frame_counter=0;
    } 
    else {
      frame_counter++;
    }

  }
  
}


