#include <math.h>

String inputString = "";
boolean stringComplete = false;
boolean badInput = false;
const int pins[] = {A0,A1,A2,A3};
const int FREQUENCY = 50; // Hz
const double ADC_SCALE = 1024 / 5 * 0.185;

int data[512];

void setup() {
  Serial.begin(115200);
  inputString.reserve(16);
}

void loop() {
  // process command
  if (badInput) {
    Serial.println("Error 1: Invalid command: " + inputString);
    resetInput();
  } else if (stringComplete) {
    if (inputString.length() < 2) {
      Serial.println("Error 1: Invalid command: " + inputString);
      resetInput();
    } else {
      unsigned int pin = inputString.charAt(0) - '0';
      if (pin > sizeof(pins) / sizeof(int)) {
        Serial.println("Error 2: Bad pin: " + inputString);
      } else {
        unsigned int cmd = inputString.charAt(1) - '0';
        unsigned long t1, t2;
        double currentMax;
        switch(cmd) {
          case 0:
            // Report current
            t1 = micros();
            for (int i=0; i < sizeof(data) / sizeof(int); i++) {
              data[i] = analogRead(pins[pin]);
            }
            t2 = micros();
            // Overhead to measure time is only 100 us; each cycel is 112 us
            // The error is less than 0.2 percent. Can safely ignore it :)
            currentMax = calcADC( 2 * M_PI * (t2 - t1) * FREQUENCY / 1000000 / (sizeof(data) / sizeof(int)) );
            
            Serial.println( currentMax / ADC_SCALE / M_SQRT2 );
            break;
          case 1:
            // Turn on relay
            Serial.println("Error 3: Not implemented: " + inputString);
            break;
          case 2:
            // Turn off relay
            Serial.println("Error 3: Not implemented: " + inputString);
            break;
          default:
            Serial.println("Error 1: Invalid command: " + inputString);
        }
      }
    }
    resetInput();
  }
}

void serialEvent() {
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (stringComplete) {
      continue; // Ignore any input while previous pending processing
    }
    if (ch == '\n') {
      stringComplete = true;
    } else if (inputString.length() >= 16) {
      badInput = true;
    } else {
      inputString += ch;
    }
  }
}

void resetInput() {
  inputString = "";
  stringComplete = badInput = false;
}

double calcADC(double angularVelocity) {
  size_t n = sizeof(data) / sizeof(int);
  double N  = (double)n;
  double X  = 0;
  double Y  = 0;
  double Z  = 0;
  double X2 = 0;
  double Y2 = 0;
  double XY = 0;
  double XZ = 0;
  double YZ = 0;

  for (int i = 0; i < n; i++)
  {
    double sine   = sin(angularVelocity * i);
    double cosine = cos(angularVelocity * i);
    X  += sine;
    Y  += cosine;
    Z  += data[i];
    X2 += sine*sine;
    Y2 += cosine*cosine;
    XY += sine*cosine;
    XZ += sine*data[i];
    YZ += cosine*data[i];
  }

  //double a0 = det3x3(Z, X, Y,XZ, X2, XY, YZ, XY, Y2) / det3x3(N, X, Y, X, X2, XY, Y, XY, Y2);
  double a1 = det3x3(N, Z, Y, X, XZ, XY, Y, YZ, Y2) / det3x3(N, X, Y, X, X2, XY, Y, XY, Y2);
  double a2 = det3x3(N, X, Z, X, X2, XZ, Y, XY, YZ) / det3x3(N, X, Y, X, X2, XY, Y, XY, Y2);

  return sqrt(a1*a1 + a2*a2);
}

double det3x3(
  double m00, double m01, double m02,
  double m10, double m11, double m12,
  double m20, double m21, double m22
){
  return m00*m11*m22 + m01*m12*m20 + m10*m21*m02 - m02*m11*m20 - m01*m10*m22 - m00*m12*m21;
}

