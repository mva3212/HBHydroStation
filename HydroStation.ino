

#include <OneWire.h>
#include <DallasTemperature.h>
#include <JeeLib.h>
#include <avr/wdt.h>

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 11;
const int networkGroup = 88;

const int DEBUG =0;
const int UNO = 1;

#define phPin 14
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

ISR(WDT_vect) {
  Sleepy::watchdogEvent();
}

typedef struct { int ph, temp, ec; } Payload;
Payload ptx;

float ph;
float reading;

int enable_pin = 4;
int reading_pin = 7;

void setup() {
  if (DEBUG) {
    Serial.begin(57600);
    Serial.println("HydroStation initializing");
  }
  sensors.begin();
  pinMode(reading_pin, INPUT);
  pinMode(enable_pin, OUTPUT);
  rf12_initialize(nodeID, freq, networkGroup);                                       // initialize RF
  rf12_sleep(RF12_SLEEP);
  if (UNO) wdt_enable(WDTO_8S);
}

void loop()
{  
  sensors.requestTemperatures();
  digitalWrite(enable_pin, HIGH);
  if (UNO) wdt_reset();
  
  long f =  getFrequency(reading_pin);
  reading = analogRead(phPin);
  
  ph = calculatePH();
  ptx.ph = ph * 100;
  ptx.temp = DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0));
  ptx.ec = f;
  if(DEBUG)  print_to_serial();
  
  digitalWrite(4, LOW);    
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
}

float calculatePH(){
  // This is calculated taking the readings at 4.0 and 7.0, then solving for the equation of a line y=mx + b
  // It should be later adjusted for "autocalibrate" mode
  return (477.0-3.0 * reading)/ -264.0;
}

long getFrequency(int pin) {
  #define SAMPLES 4096
  
  long x = 5000000.0;
  long y = x/2;
  long f2 = 0;
  for(unsigned int j=0; j<SAMPLES; j++){ 
    long r = pulseIn(pin, HIGH,250000);
    f2+= 500000/r;
  }
  return f2 / SAMPLES;
}

