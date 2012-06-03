


#include "GoroLog.h"

const char svn_id[]="$Id: Logger.h 63 2011-11-30 12:03:02Z rgorosito $";

char file_name[]="GORO_00.TXT";

#define LED_ERROR         B01010101
#define LED_GRABA         B11111111
#define LED_NO_GRABA      B00000000
#define LED_GRABA_AUTO    B01111111
#define LED_NO_GRABA_AUTO B10000000

//#define DEBUG 1

class Logger {

private:
  uint8_t modo_led;
  File archivo;
  byte ledpin;
  byte swpin;
  bool armado;
  bool grabando;
  bool automatico;
  bool boton_apretado;
  bool error;
  long anterior_grabada;
  byte file_number;
  
  bool can_log(void) {
    return ( grabando );
  }

  void marca_error(void) {
    error=true;
    modo_led=LED_ERROR;
  }

  void iniciar_grabacion() {

    unsigned long arranca=millis();

#ifdef DEBUG
    Serial.print("L:START:");
    Serial.println(arranca,DEC);
    Serial.println("L:inicia grabacion");
#endif

    if ( can_log() ) {

      for ( ; file_number < 100; file_number++) {
        file_name[5] = '0' + file_number/10;
        file_name[6] = '0' + file_number%10;
        // create if does not exist, do not open existing, write, sync after write
        if (! SD.exists(file_name)) {
#ifdef DEBUG
          Serial.print("L:");
          Serial.print(file_name);
          Serial.println(" no existia");
#endif
          break;
        } 
        else {
#ifdef DEBUG
          Serial.print("L:");
          Serial.print(file_name);
          Serial.println(" ya existia");
#endif
        }
      }

      // si i=100, entonces estan todos los archivos ocupados
      if ( file_number == 100 ) {
        marca_error();
        return;
      }


#ifdef DEBUG
      Serial.println("L:creo archivo");
#endif
      // creo el archivo
      archivo=SD.open(file_name,  O_CREAT | O_TRUNC |O_WRITE );
      //                    archivo=SD.open(file_name, FILE_WRITE);
      //         
      // fallo?
      if ( ! archivo ) {
        marca_error();
        return;
      }

      // La primer linea del archivo es la revision del logger
      archivo.println(svn_id);
      // La segunda, cuando comenzamos a grabar
      archivo.print("START,");
      archivo.println(millis(),DEC);


      // lo pudo crear, estamos grabando
      grabando=true;

      anterior_grabada=millis();

#ifdef DEBUG
      Serial.print("L:END:");
      Serial.println(millis()-arranca,DEC);
#endif


    }

  }


  void detener_grabacion() {

#ifdef DEBUG
    Serial.println("L:fin grabacion");
#endif

    grabando=false;

    if ( archivo ) {
      
      archivo.print("END,");
      archivo.println(millis(),DEC);
      
      archivo.flush(); // es necesario?
      archivo.close();
    } 
    else {
      marca_error();
    }

  }


public:
  Logger() {
  }

  void init(const byte led_pin, const byte sw_pin ) {

    file_number=0;
    boton_apretado=false;

    armado=false;
    error=false;
    grabando=false;
    automatico=true;
    modo_led=LED_NO_GRABA_AUTO;

    // configuracion de pines
    ledpin=led_pin;
    swpin=sw_pin;

    pinMode(ledpin,OUTPUT);
    pinMode(swpin,INPUT);

    if (SD.begin()) {
#ifdef DEBUG
      Serial.println("L:OK SD");
#endif
    } 
    else {
#ifdef DEBUG

      Serial.println("L:FALLO SD");
#endif
      marca_error();
    }

  }

  void actualiza_led(uint8_t frame) {
    if ( modo_led & ( 1 << frame )) {
      digitalWrite(ledpin,HIGH);
    } else {
      digitalWrite(ledpin,LOW);
    }
  }


//  void change_led(byte secuencia) {
//
//    if ( error ) {
//      digitalWrite(ledpin, secuencia & 1 ? HIGH : LOW );
//    } 
//    else if ( secuencia & B011 ) {
//      // secuencia 1,2,3
//      digitalWrite(ledpin, grabando ? HIGH : LOW );
//    } 
//    else {
//      // secuencia 0
//      digitalWrite(ledpin, ( automatico != grabando ) ? HIGH : LOW );
//      // auto graba led
//      //   1    1    0
//      //   0    1    1
//      //   1    0    1
//      //   0    0    0
//    }
//
//  }

  void check_button() {

    bool boton = ( digitalRead(swpin) == HIGH ? false : true );


    // solo hago algo si cambio de estado
    if ( boton!=boton_apretado ) {

      boton_apretado = boton;


      if ( error ) {
#ifdef DEBUG
        Serial.println("L:Ignoro boton por error");
#endif
        return;
      }



      if ( boton ) {
#ifdef DEBUG
        Serial.println("L:boton apretado");
#endif
        // estaba en off?
        if ( !automatico && !grabando ) {
          // off , paso a automatico
          automatico=true;
          modo_led=LED_NO_GRABA_AUTO;
#ifdef DEBUG
          Serial.println("L:PASO A AUTOMATICO");
#endif

        } 
        else if ( !automatico && grabando ) {
          detener_grabacion();
          modo_led=LED_NO_GRABA;
#ifdef DEBUG
          Serial.println("L:DEJO DE GRABAR FORZADO");
#endif
        } 
        else if ( automatico && grabando ) {
          automatico=false;
          modo_led=LED_GRABA;
#ifdef DEBUG
          Serial.println("L:SIGO GRABANDO, AHORA FORZADO");
#endif
        } 
        else {
          // automatico && !grabando
          automatico=false;
          grabando=true;
          iniciar_grabacion();
          modo_led=LED_GRABA;
#ifdef DEBUG
          Serial.println("L:GRABO FORZADO");
#endif
        }



      } 
      else {
        // soltado
#ifdef DEBUG
        Serial.println("L:boton soltado");
#endif
      }
    }
  }

  void log_bat(const int vbat, const int ibat, const byte frame ) {
    if ( can_log() ) {
      archivo.print("BAT,");
      log_anterior_grabada();
      archivo.print(millis(),DEC);
      archivo.print(",");
      archivo.print(frame,DEC);
      archivo.print(",");
      archivo.print(vbat,DEC);
      archivo.print(",");
      archivo.println(ibat,DEC);
    }
  }

  void log_gps(long lat, long lon, long alt, long home_lat, long home_lon, long home_alt, bool ok) {
    if ( can_log() ) {
      archivo.print("GPS,");
      archivo.print(lat,DEC);
      archivo.print(",");
      archivo.print(lon,DEC);
      archivo.print(",");
      archivo.print(alt,DEC);
      archivo.print(",");
      archivo.print(home_lat,DEC);
      archivo.print(",");
      archivo.print(home_lon,DEC);
      archivo.print(",");
      archivo.print(home_alt,DEC);
      archivo.print(",");
      archivo.println(ok ? "OK" : "BAD");
    }
  }
  
  void log_uptime(unsigned long uptime) {
    if ( can_log() ) {
      archivo.print("UPTIME,");
      archivo.println(uptime,DEC);
    }
  }
  
  void log_compass(compass_data_t *compass) {
    if ( can_log() ) {
      archivo.print("COMPASS,");
      archivo.print(compass->hdgX,DEC);
      archivo.write(',');
      archivo.print(compass->hdgY,DEC);
      archivo.write(',');
      archivo.print(compass->measuredMagX,DEC);
      archivo.write(',');
      archivo.print(compass->measuredMagY,DEC);
      archivo.write(',');
      archivo.println(compass->measuredMagZ,DEC);
    }
    
  } 

  void log_gps(gps_data_t *gps) {
    if ( can_log() ) {
      archivo.print("GPS,");
      archivo.print(gps->lat,DEC);
      archivo.write(',');
      archivo.print(gps->lon,DEC);
      archivo.write(',');
      archivo.print(gps->fix_age,DEC);
      archivo.write(',');
      archivo.print(gps->chars,DEC);
      archivo.write(',');
      archivo.print(gps->good_sentences,DEC);
      archivo.write(',');
      archivo.println(gps->failed_cs,DEC);
    }
    
  } 


  void log_anterior_grabada() {

    long ahora=millis();
    long delta=ahora-anterior_grabada;
    anterior_grabada=ahora;

    archivo.print(delta,DEC);
    archivo.print(",");

  }
  
  void log_veces_esperando(const long veces) {
    if ( can_log() ) {
      archivo.print("VECES_ESPERANDO,");
      archivo.println(veces,DEC);
    }
  }

  void motores_armados() {

#ifdef DEBUG
    Serial.println("L:armado");
#endif

    if ( automatico && !grabando )
      iniciar_grabacion();
      modo_led=LED_GRABA_AUTO;

  }

  void motores_desarmados() {

#ifdef DEBUG
    Serial.println("L:desarmado");
#endif

    if ( automatico && grabando )
      detener_grabacion();
      modo_led=LED_NO_GRABA_AUTO;

  }

  const byte estado() {

    if ( error ) {
      return GOROL_ESTADO_ERROR;
    } 
    else if ( grabando ) {
      if (automatico )
        return GOROL_ESTADO_GRABANDO_AUTO;
      else
        return GOROL_ESTADO_GRABANDO_FIX;
    } 
    else {
      if (automatico )
        return GOROL_ESTADO_ESPERANDO;
      else
        return GOROL_ESTADO_SIN_GRABAR;
    }

  }

};

