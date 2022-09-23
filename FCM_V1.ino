//Servo Library
#include <Servo.h>
#include <Wire.h>

//display Stuff
#include <Arduino.h>
#include <OLED_SSD1306_Chart.h>
#include <Adafruit_I2CDevice.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define BAUDRATE 9600
#define SDA_PIN 2
#define SCL_PIN 3
OLED_SSD1306_Chart display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
char actualThickness;


// RC Channels
int throttle = A0;
int safety = A2;
int ch;
int minValue;
int maxValue;
int defaultValue;

// Fuel Management
Servo fuelValve;
int fuelServo = 10;
int fuelVal;
int throttleVal;
int fuelFlow;

// Ignition
int arcPin = 8;
int flameSensor = A1;
int flameVal = 0;
int flameOut = 4;
int safetyVal = 0;
int armed = 0;
int idle = 80;
int purgeRequired = 1;
int ignitionRequired = 1;
int timeOut = 0;
int prevState = 0;

//Channel Reading
int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue) {
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

void setup() {

  Serial.begin(BAUDRATE);
  Wire.begin();
  initialiseDisplay();
  pinMode(throttle, INPUT);
  pinMode(safety, INPUT);
  pinMode(flameSensor, INPUT);;
  fuelValve.attach(fuelServo);
}

void loop() {

  throttleVal = readChannel(throttle, 0, 1000, 0);
  Serial.print("Throttle: ");
  Serial.println(throttleVal);
  safetyVal = readChannel(safety, 0, 1000, 0);
  Serial.print("Safety: ");
  Serial.println(safetyVal);
  fuelVal = map(throttleVal, 0, 1000, 80, 50);
  fuelFlow = map(fuelVal, 80, 50, 0, 100);
  flameVal = analogRead(flameSensor);
  if (flameVal <= 1) {
    ignitionRequired = 1;
  }

  preFlight();

  if (ignitionRequired == 1 && armed == 1) {
    ignite();
    Serial.println("Igniting");
  }
  if (throttleVal <= 100 && armed == 1) {
    fuelValve.write(idle);
    Serial.println("Idling Servo Value");
  }
  if (throttleVal > 100 && armed == 1) {
    fuelValve.write(fuelVal);
    Serial.println("Servo under manual control");
  }
  updateDisplay(fuelFlow);
}

void preFlight() {
  if (safetyVal >= 100) {
    armed = 1;
  }

  else if (safetyVal <= 100) {
    armed = 0;
  }
  Serial.print("Armed: ");
  Serial.println(armed);
  if (armed == 0) {
    Serial.println("Disarmed");
    fuelValve.write(90);
    digitalWrite(arcPin, LOW);
    prevState = 0;
    
        if (timeOut == 180) {
          purgeRequired = 1;
          timeOut = 0;
          }
        else timeOut++;
    
            if (flameVal >= 1) {
               Serial.println("WARNING! UNCONTROLLED FLAME IN TUBE.");
               display.clearDisplay();
               display.setTextSize(2); // Draw 2X-scale text
               display.setTextColor(SSD1306_WHITE);
               display.setCursor(1, 0);
               display.println(F("WARNING!"));
               display.print(F("Flame!|"));
               display.println(flameVal);
               display.display(); 
    }
  }
  if (armed == 1) {
    Serial.println("armed");
    
   if (prevState == 0) {
    display.clearDisplay();
    initialiseDisplay();
    prevState = 1;   
   }
         
    if (purgeRequired == 1) {
        display.clearDisplay();
        display.setTextSize(2); // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(1, 0);
        display.println(F("ARMED!"));
        display.println(F("Purging..."));
        display.display(); 
      purge();
      purgeRequired = 0;
      display.clearDisplay();
      Serial.print("clearing display");
      initialiseDisplay();
    }
    timeOut = 0;
   
  //      display.drawChart();
  //display.display();
    updateDisplay;
  }
}

void purge() {
  fuelValve.write(65);
  delay(500);
  fuelValve.write(90);
  delay(500);
  fuelValve.write(65);
  delay(750);
  fuelValve.write(90);
  delay(1000);
}

void ignite() {
  if (ignitionRequired = 1) {
    digitalWrite(arcPin, HIGH);
    delay(100);
    digitalWrite(arcPin, LOW);
    delay(100);
    digitalWrite(arcPin, HIGH);
    delay(100);
    digitalWrite(arcPin, LOW);
    delay(100);
    digitalWrite(arcPin, HIGH);
    delay(100);
    digitalWrite(arcPin, LOW);
    delay(100);
    digitalWrite(arcPin, HIGH);
    delay(100);
    digitalWrite(arcPin, LOW);
  }

}

void initialiseDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  display.setChartCoordinates(0, 28);      //Chart lower left coordinates (X, Y)
  display.setChartWidthAndHeight(123, 27); //Chart width = 123 and height = 60
  display.setXIncrement(1);                //Distance between Y points will be 5px
  display.setYLimits(0, 100);             //Ymin = 0 and Ymax = 100
  display.setYLimitLabels("0", "100");    //Setting Y axis labels
  display.setYLabelsVisible(true);
  display.setAxisDivisionsInc(8, 5);    //Each 12 px a division will be painted in X axis and each 6px in Y axis
  display.setPlotMode(SINGLE_PLOT_MODE); //Set single plot mode
  // display.setPointGeometry(POINT_GEOMETRY_CIRCLE);
  actualThickness = NORMAL_LINE;
  display.setLineThickness(actualThickness);
  display.drawChart(); //Update the buffer to draw the cartesian chart
  display.display();

}

void updateDisplay(int fuelFlow) {
  if (!display.updateChart(fuelFlow)) //Value between Ymin and Ymax will be added to chart
  {
    display.clearDisplay(); //If chart is full, it is drawn again
    Serial.print("clearing display2");
    if (actualThickness == NORMAL_LINE)
    {
      actualThickness = LIGHT_LINE;
    }
    else if (actualThickness == LIGHT_LINE)
    {
      actualThickness = NORMAL_LINE;
    }
    display.setLineThickness(actualThickness);
    display.drawChart();
  }
  delay(100);
}
