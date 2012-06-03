
/*
 * Aca van a ir la lista de comandos que le envia el mega al 328
 *
 */

#ifndef _GOROLOG_H
#define _GOROLOG_H

#define GOROL_CLIENT Serial1
#define GOROL_SPEED 57600

#define GOROL_MAX_LONG_DATO 64

#define GOROL_INICIALIZACION 'I' //TODO: analizar si tiene sentido
#define GOROL_QUERY_ESTADO 'Q'

#define GOROL_MOTORES_ARMADOS 'A'
#define GOROL_MOTORES_DESARMADOS 'D'

#define GOROL_UPTIME   'u'
#define GOROL_COMPASS  'c'
#define GOROL_HEADING  'a'
#define GOROL_ALTITUDE 'h'
#define GOROL_GPS      'g'


#define GOROL_PRUEBA1 '1'
#define GOROL_PRUEBA2 '2'

#define GOROL_ESTADO_GRABANDO_FIX 'g'
#define GOROL_ESTADO_GRABANDO_AUTO 'G'
#define GOROL_ESTADO_ESPERANDO 'N'
#define GOROL_ESTADO_SIN_GRABAR 'n'
#define GOROL_ESTADO_ERROR 'E'

struct compass_data_t {
  float hdgX;
  float hdgY;
  float measuredMagX;
  float measuredMagY;
  float measuredMagZ;
};

struct flight_angle_data_t {
  float angleRoll;
  float anglePitch;
  float angleYaw;
};

struct altitude_data_t {
  float data;
};

struct position_data_t {
  long lat;
  long lon;
  //TODO: agregar satelites, dop?, si es FIX o no 
};

struct home_data_t {
  long lat;
  long lon;
  float headingYaw;
  float groundAltitude;
};

struct motors_data_t {
  uint8_t num_motors;
  int motorCommand[LASTMOTOR];
};

// TODO agregar la data de bateria

struct receiver_data_t {
  int transmitterCommand[LASTCHANNEL];
};

struct gps_data_t {
  long lat;
  long lon;
  unsigned long fix_age;
  unsigned long chars;
  unsigned short good_sentences;
  unsigned short failed_cs;
};



union binary_data_t {
    uint8_t dato[GOROL_MAX_LONG_DATO];
    long largo;
    unsigned long largo_sin_signo;
    float flotante;
    compass_data_t compass;
    flight_angle_data_t myFlightAngle;
    altitude_data_t myAltitude;
    position_data_t myPosition;
    home_data_t myHome;
    motors_data_t myMotors;
    receiver_data_t myReceiver;
    gps_data_t myGps;
};


#endif
