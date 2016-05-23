#include <ArduinoJson.h>
#include <Event.h>

// Time constants
const signed long millisecond  = 10000;
const signed long second       = 100000;

// Event Scheduler
int state = 0;
event Event;

// Serial Interface
boolean         inJson;
int             inByte;
char            data[255];
int             inChar;

// Pins
typedef struct {
  int    number;
  bool   analog;
  byte   mode;
  int    current  = 0;
  int    target   = 0;
  int    velocity = 0;
} Pin;

Pin pins[24];

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // wait serial port initialization
  }
  char rwp[] = "rwPins";
  char rws[] = "rwSerial";
  Event.Add(rws, (100 * millisecond), rwSerial);
  Event.Add(rwp,  75000,  rwPins);
  Serial.println("Interface Active");
}

void loop() {
  Event.Service();
}

// Validations
int boolValueValidation(int value){
  if(value <= 0){
    value = 0;
  }else if(value >= 1){
    value = 1;
  }
}

int intValueValidation(int value, bool velocity = false) {
  int minValue = 0;
  if(velocity){
    minValue = 1;
  }
  if(value < minValue)
    value = minValue;
  else if(value > 255){
    value = 255;
  }
  return value;
}

// Write To Output Pins
void rwPins(){
  Pin pin;
  for(int i = 0; i < sizeof(pins); i++){
    pin = pins[i];
    if(pin.mode == OUTPUT){
      if(pin.analog){
        if(pin.current != pin.target){
          analogWrite(pin.number, pin.current);
          if(pin.current < pin.target){
            pin.current += pin.velocity;
            if((pin.velocity + pin.current) > pin.target){
              pin.current = pin.target;
            }
          }else if(pin.current > pin.target){
            pin.current -= pin.velocity;
            if((pin.velocity - pin.current) < pin.target){
              pin.current = pin.target;
            }
          }
        }
      }
    }else{
      if(pin.analog){
        pin.current = analogRead(pin.number);
      }else{
        pin.current = digitalRead(pin.number);
      }
    }
  }
}
// Read And Write Serial
void rwSerial(){
  serialOutput();
  serialInput();
}

// Serial Ouput
void serialOutput(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& jsonPins = root.createNestedArray("pins");
  int pinCount = sizeof(pins);
  Pin pin;
  for(int i = 0; i < pinCount; i++){
    pin = pins[i];
    StaticJsonBuffer<255> jsonPinBuffer;
    JsonObject& pinRoot = jsonPinBuffer.createObject();
    pinRoot["number"] = pin.number;
    pinRoot["analog"] = pin.analog;
    pinRoot["mode"] = pin.mode;
    if(pin.mode == OUTPUT){
      pinRoot["current"] = pin.current;
      if(!pin.analog){
        pinRoot["target"] = pin.target;
        pinRoot["velocity"] = pin.velocity;
      }  
    }
    jsonPins.add(pinRoot);
  }
  root.printTo(Serial);
}

// Serial Input Functions
void constructJson(int inChar){
  // 123 is ASCII for {, the start of a JSON statement
  if(inChar == 123 || inJson == true){
    data[sizeof(data) + 1] = (char) inChar;
    inJson = true;
  }
  // 125 is ASCII for }, the end of a JSON statement
  if(inChar == 125){
    inJson = false;
    processJson();
   data[0] = (char) 0;
  }
}

// Serial Input
void serialInput(){
  if(Serial.available() > 0){
    do{inChar = Serial.read();}while (inChar == -1);
    constructJson(inChar);
  }
}

void processJson(){
  StaticJsonBuffer<255> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data);
  if(sizeof(root["setPins"]) > 0){
    for(int i = 0; i < sizeof(root["setPins"]); i++){
      Pin newPin;
      if(root["setPins"][i]["analog"] == "true"){
        newPin.analog = true;
      }else{
        newPin.analog = false;
      }
      newPin.number = intValueValidation(root["setPins"][i]["number"]);
      if(root["setPins"][i]["mode"] == "output"){
        pinMode(newPin.number, OUTPUT);
        newPin.mode = OUTPUT;
      }else{
        pinMode(newPin.number, INPUT);
        newPin.mode = INPUT;
      }
    }
  }
  if(sizeof(root["changePins"]) > 0){
     Pin pin;
    for(int i = 0; i < sizeof(root["changePins"]); i++){
      for(int j = 0; j < sizeof(pins); j++){
        // These types of comparisons are likely to fail
        if(pins[j].number == root["setPins"][i]["number"]){
          if(pins[j].analog){
            pins[j].target = intValueValidation(root["setPins"][i]["target"]);
            pins[j].velocity = intValueValidation(root["setPins"][i]["velocity"], true);
          }else{
            // This will probably not work
            if("HIGH" == root["setPins"][i]["target"]){
              pins[j].current =  HIGH;
              digitalWrite(pins[j].number, pins[j].current);
            }else{
              pins[j].current =  LOW;
              digitalWrite(pins[j].number, pins[j].current);
            }
          }
        }
      }
    }
  }
}
