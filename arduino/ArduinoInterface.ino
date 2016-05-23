#include <ArduinoEvent.h>
#include <ArduinoJson.h>

// Time constants
const int millisecond  = 10000;
const int second       = 100000;

// Event Scheduler
int state = 0;
event Event;

// Serial Interface
boolean         inJson   = false;
int             inByte   = 0;
String          inString = "";
int             inChar   = 0;

// Pins
typedef {
  String name;
  bool   analog;
  byte   number;
  byte   mode;
  int    current  : 0;
  int    target   : 0;
  int    velocity : 0;
} Pin

Pin pins[]   = {};

void setup() {
  Serial.begin(9600);
  Serial.println("Interface Active");
  Event.Add("serialInput", (100 * millisecond), serialInput);
  Event.Add("serialOutput", (100 * millisecond), serialOutput);
  Event.Add("writeOuput",  (25 * millisecond),  writeOutput);
  Event.Add("readInput",  (75 * millisecond),  readInput);
}

void loop() {
  Event.Service();
}

// Serial Ouput
void serialOutput(){
  Serial.print("{\"pins\": [");
  for (int i = 0; i < sizeof(pins); i++)
    Serial.print("{\"name\":");
    Serial.print(pins[i].name);
    Serial.print("{,\"number\":");
    Serial.print(pins[i].number);
    if pins[i].analog == HIGH
      Serial.print("{,\"analog\": \"HIGH\"");
    else
      Serial.print("{,\"analog\": \"LOW\"");
    if pins[i].mode == OUTPUT
      Serial.print("{,\"mode\": \"OUTPUT\"");
      Serial.print("{,\"current\":");
      Serial.print(pins[i].current);
      if !pins[i].analog
        Serial.print("{,\"target\":");
        Serial.print(pins[i].target);
        Serial.print("{,\"velocity\":");
        Serial.print(pins[i].velocity);
    else
      Serial.print("{,\"mode\": \"INPUT\"");
      Serial.print("{,\"current\":");
      Serial.print(pins[i].current);
    Serial.print("}");
    if i != pinCount
      Serial.print(",");
  Serial.print("]}\n");
  Serial.flush();
}

// Write To Output Pins
void writeOutput(){
  for (int i = 0; i < pinCount; i++)
    if pins[i].mode == OUTPUT
      if pins[i].analog == true
        if pins[i].current != pins[i].target;
          analogWrite(pins[i].number, pins[i].current);
          if pins[i].current < pins[i].target
            pins[i].current += pins[i].velocity;
            if (pins[i].velocity + pins[i].current) > pins[i].target
              pins[i].current = pins[i].target;
          if pins[i].current > pins[i].target
            pins[i].current -= pins[i].velocity;
            if (pins[i].velocity - pins[i].current) < pins[i].target
              pins[i].current = pins[i].target;
}

// Serial Input
void serialInput(){
  if (Serial.available() > 0)
    do{inChar = Serial.read();}while (inChar == -1);
    constructString(inChar);
}

// Serial Input Functions
void constructString(int inChar){
  // 123 is ASCII for {, the start of a JSON statement
  if inChar == 123 || inJson == true
    inString += (char) inChar;
    inJson = true;
  // 125 is ASCII for }, the end of a JSON statement
  if inChar == 125
    inJson = false;
    processString(inString);
    inString = "";
}

void processString(String inString){
  StaticJsonBuffer<200> jsonBuffer;
  char json[128];
  inString.toCharArray(json, 128);
  JsonObject& root = jsonBuffer.parseObject(inString);
  if sizeof(root["setPins"]) > 0
    for int i = 0; i < sizeof(root["setPins"]); i++
      Pin newPin;
      newPin.name = root["setPins"][i]["name"];
      if root["setPins"][i]["analog"] == "true"
        root["setPins"][i]["analog"] = true
      else root["setPins"][i]["analog"] == "false"
        root["setPins"][i]["analog"] = false
      newPin.number = valueValidation(root["setPins"][i]["number"].toInt());
      if root["setPins"][i]["mode"] == "output"
        pinMode(newPin.number, OUTPUT);
        newPin.mode = OUTPUT;
      else
        pinMode(newPin.number, INPUT);
        newPin.mode = INPUT;
  if sizeof(root["changePins"]) > 0
    for int i = 0; i < sizeof(root["changePins"]); i++
      for int j = 0; j < sizeof(pins); j++
        if root["setPins"][i]["name"] == pins[j].name
          if pins[j].analog
            pins[j].target = valueValidation(root["setPins"][i]["target"].toInt());
            pins[j].velocity = intValueValidation(root["setPins"][i]["velocity"].toInt(), true);
          else
            if root["setPins"][i]["target"] == "HIGH"
              pins[j].current =  HIGH
              digitalWrite(pins[i].number, pins[i].current);
            else
              pins[j].current =  LOW
              digitalWrite(pins[i].number, pins[i].current);
}

int boolValueValidation(int value){
  if value =< 0
    value = 0
  else if value >= 1
    value = 1
}

int intValueValidation(int value, bool velocity = false) {
  if velocity
    minValue = 1
  else
    minValue = 0
  if value < minValue
    value = minValue;
  else if value > 255
    value = 255;
  return value
}
