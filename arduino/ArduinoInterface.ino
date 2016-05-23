#include <ArduinoJson.h>
#include <AikoEvents.h>


// Time constants
const signed long millisecond  = 10000;
const signed long second       = 100000;

// Event Scheduler
using namespace Aiko;

// Serial Interface
boolean         inJson;
int             inByte;
String          inString;
int             inChar;
int             openBrackets = 0;

// Pins
typedef struct {
  bool   active = false;
  int    number;
  bool   analog;
  bool   output;
  int    current;
  int    target;
  int    velocity;
} Pin;

Pin pins[32];

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // wait serial port initialization
  }
  Events.addHandler(writeSerial, 1500);
  //Events.addHandler(rwPins, 1500);
  //Events.addHandler(readSerial, 1500);

  Serial.println("Interface Active");
}

void loop() {
  Events.loop();
}

// Validation
int intValueValidation(int value, bool velocity = false) {
  int minValue = 0;
  if(velocity){
    minValue = 1;
  }
  if(value <= minValue)
    value = minValue;
  else if(value > 255){
    value = 255;
  }
  return value;
}

// Write To Output Pins
void rwPins(){
  for(int i = 0; i < 32; i++){
    if(pins[i].active || pins[i].number != 0){
      if(!pins[i].output){
        if(pins[i].analog){
          pins[i].current = analogRead(i);
        }else{
          pins[i].current = digitalRead(i);
        }
       }else{
        if(pins[i].analog){
          if(pins[i].current != pins[i].target){
            analogWrite(i, pins[i].current);
            if(pins[i].current < pins[i].target){
              pins[i].current += pins[i].velocity;
              Serial.println("Increasing");
              if((pins[i].velocity + pins[i].current) >= pins[i].target){
                pins[i].current = pins[i].target;
                pins[i].velocity = 0;
              }
            }else if(pins[i].current > pins[i].target){
              pins[i].current -= pins[i].velocity;
              if((pins[i].velocity - pins[i].current) <= pins[i].target){
                pins[i].current = pins[i].target;
                pins[i].velocity = 0;
              }
            }
          }
        }
      }
    }
  }
}

void zzwriteSerialbe(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& jsonPins = root.createNestedArray("pins");
  StaticJsonBuffer<255> jsonPinBuffer;
  
  for(int i = 0; i < 32; i++){
    JsonObject& pinRoot = jsonPinBuffer.createObject();
    int validatedNumber = intValueValidation(pins[i].number);
    if(pins[validatedNumber].active == true && validatedNumber != 0){
      pinRoot["number"] = pins[validatedNumber].number;
      pinRoot["analog"] = pins[validatedNumber].analog;
      pinRoot["output"] = pins[validatedNumber].output;
      if(pins[validatedNumber].output){
        pinRoot["current"] = pins[validatedNumber].current;
        if(pins[validatedNumber].analog){
          pinRoot["target"] = pins[validatedNumber].target;
          pinRoot["velocity"] = pins[validatedNumber].velocity;
          //Serial.println("current " + String(pinRoot["current"]));
          
          //Serial.println("velocity " + String(pinRoot["velocity"]));

        }
      }
      jsonPins.add(pinRoot);
    }
  }
}

void writeSerial(){
  Serial.println("dinge and dinge etwas etwas etwas");
  //String printBuffer;
  //root.printTo(printBuffer);
  //Serial.println(printBuffer);
  //root.printTo(Serial);
  Serial.flush();
}

// Serial Input Functions
void constructJson(int inChar){
  inString += (char) inChar;
  if(inChar == 123){
    openBrackets += 1;
  }else if(inChar == 125){
     if(openBrackets > 0){
       openBrackets -= 1;
       if(openBrackets <= 0){
         processJson(inString);
         inString = "";
       }
     }
  }
  if(inString[0] != 123 || (inString[0] != 123 && inString[1] != 34) || sizeof(inString) > 500){
    inString = "";
  }
}

void processJson(String jsonData){
  // {"setPins":[{"number": 2, "output": true, "analog": true}]}
  StaticJsonBuffer<255> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonData);
  int validatedNumber;
  if(sizeof(root["setPins"]) > 0){
    for(int i = 0; i < 32; i++){
      validatedNumber = intValueValidation(root["setPins"][i]["number"]);
      if(!pins[validatedNumber].active && pins[validatedNumber].number == 0){
        pins[validatedNumber].number = validatedNumber;
        pins[validatedNumber].active = true;
        pins[validatedNumber].analog = root["setPins"][i]["analog"];
        pins[validatedNumber].current = 0;
        if(root["setPins"][i]["output"]){
          pins[validatedNumber].target = 0;
          pins[validatedNumber].velocity = 0;
          pinMode(validatedNumber, OUTPUT);
          pins[validatedNumber].output = true;
        }else{
          pinMode(validatedNumber, INPUT);
          pins[validatedNumber].output = false;
        }
      }
    }
  }
  // {"changePins":[{"number": 2, "target": 125, "velocity": 5}]}
  if(sizeof(root["changePins"]) > 0){
    for(int i = 0; i < 32; i++){
      validatedNumber = intValueValidation(root["changePins"][i]["number"]);
      if(pins[validatedNumber].active && pins[validatedNumber].number != 0 && pins[validatedNumber].output){
        if(pins[validatedNumber].analog){
          pins[validatedNumber].target = intValueValidation(root["changePins"][i]["target"]);
          pins[validatedNumber].velocity = intValueValidation(root["changePins"][i]["velocity"], true);
        }else{
          Serial.println("digital write");
          if(root["changePins"][i]["target"] == "high"){
            pins[validatedNumber].current =  HIGH;
            digitalWrite(validatedNumber, pins[validatedNumber].current);
          }else{
            pins[validatedNumber].current =  LOW;
            digitalWrite(validatedNumber, pins[validatedNumber].current);
          }
        }
      }
    }
  }
  //writeSerial();
}

// Read And Write Serial
void readSerial(){
  if(Serial.available() > 0){
    do{inChar = Serial.read();}while (inChar == -1);
    constructJson(inChar);
  }
}

