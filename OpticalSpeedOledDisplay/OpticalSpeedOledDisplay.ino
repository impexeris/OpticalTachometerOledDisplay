/*********************************************************************
 Speed display with OLED digital and analog meter
 *********************************************************************/

/*********************************************************************
 USER DEFINED CONSTANTS
 You must adjust the constants below to tailor the display to your configuration
   First:  Select the type of OLED display you are using
   Second: Enter the diameter of the wheel that is being used for sensing pulses
           Either enter WHEEL_DIAMETER_IN_INCHES or WHEEL_DIAMETER_IN_CM
           The program will favor WHEEL_DIAMETER_IN_CM if both are defined
   Third:  Enter the number of spokes on the wheel that will interrupt the sensor
   Fourth: Choose if the display will be in Metric units (true) or SAE units (false)
 *********************************************************************/

//One of the next two #defines must be uncommented for the type of OLED display
     //SSD1306 is typically the 0.96" OLED
//#define OLED_TYPE_SSD1306
     //SH1106 is typically a 1.3" OLED 
#define OLED_TYPE_SH1106

//Wheel diameter in either inches or CM
#define WHEEL_DIAMETER_IN_INCHES 13.8  
//#define WHEEL_DIAMETER_IN_CM  35
#define WHEEL_REFLECTIVE_STRIP_COUNT 24
//Specify either SAE units or Metric units
#define DISPLAY_SAE_UNITS 
//#define DISPLAY_METRIC_UNITS 
//#define SPEED_PER_SECOND 

/*********************************************************************
 Libraries
 *********************************************************************/
#ifdef OLED_TYPE_SH1106 
   #include <Adafruit_SH1106.h>
#endif
#ifdef OLED_TYPE_SSD1306
  #include <Adafruit_SSD1306.h>
#endif 

#include <Math.h>

#ifdef WHEEL_DIAMETER_IN_INCHES
  #ifdef WHEEL_DIAMETER_IN_CM
     "ERROR: Only Define Metric or SAE wheel diameter, not both"
  #endif
#endif

#ifdef DISPLAY_METRIC_UNITS
  #ifdef DISPLAY_SAE_UNITS
     "ERROR: Only Define Metric or SAE, not both"
  #endif
#endif

namespace {
  #ifdef DISPLAY_SAE_UNITS
    #ifdef SPEED_PER_SECOND
      const unsigned int MAJOR_TICKS[] = { 0, 50, 100 };
      const unsigned int MID_TICKS[] = { 25, 75 };
      const unsigned int MINOR_TICKS[] = {10, 20, 30, 40, 60, 70, 80, 90};
    #else
      const unsigned int MAJOR_TICKS[] = { 0, 1000, 2000, 3000, 4000 };
      const unsigned int MID_TICKS[] = { 500, 1500, 2500, 3500 };
      const unsigned int MINOR_TICKS[] = {};
    #endif
  #endif
  
  #ifdef DISPLAY_METRIC_UNITS
    #ifdef SPEED_PER_SECOND
      const unsigned int MAJOR_TICKS[] = { 0, 10, 20 };
      const unsigned int MID_TICKS[] = { 5, 15 };
      const unsigned int MINOR_TICKS[] = {1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 16, 17, 18, 19};
     #else
      const unsigned int MAJOR_TICKS[] = { 0, 1000, 2000 };
      const unsigned int MID_TICKS[] = { 500, 1500 };
      const unsigned int MINOR_TICKS[] = {100, 200, 300, 400, 600, 700, 800, 900, 1100, 1200, 1300, 1400, 1600, 1700, 1800, 1900};
    #endif
  #endif
 
  const int OLED_RESET = 4;
  const int TEXT_SIZE_SMALL = 1;
  const int TEXT_SIZE_LARGE = 2;
  const int ONE_K = 1000;
  
  const int OLED_HEIGHT = 64;
  const int OLED_WIDTH = 128;
  const int YELLOW_SEGMENT_HEIGHT = 16;
  const int DISPLAY_FULL_BRIGHTNESS = 255;
  const int DISPLAY_DIM_BRIGHTNESS = 0;
  
  const int IR_LED_PIN_3 = 3;
  const int PHOTODIODE_PIN_2 = 2;
  const int INTERRUPT_ZERO_ON_PIN_2 = 0;
  
  const uint16_t DIAL_CENTER_X = OLED_WIDTH / 2;
  const uint16_t DIAL_RADIUS = (OLED_HEIGHT - YELLOW_SEGMENT_HEIGHT) - 1;
  const uint16_t DIAL_CENTER_Y = OLED_HEIGHT - 1;
  const uint16_t INDICATOR_LENGTH = DIAL_RADIUS - 5;
  const uint16_t INDICATOR_WIDTH = 5;
  const uint16_t LABEL_RADIUS = DIAL_RADIUS - 18;
  const int DIAL_LABEL_Y_OFFSET = 6;
  const int DIAL_LABEL_X_OFFSET = 4;
  
  const int MAJOR_TICK_COUNT = sizeof(MAJOR_TICKS) / sizeof(MAJOR_TICKS[0]);
  const int MAJOR_TICK_LENGTH = 7;
  const int MID_TICK_COUNT = sizeof(MID_TICKS) / sizeof(MID_TICKS[0]);
  const int MID_TICK_LENGTH = 6;
  const int MINOR_TICK_COUNT = sizeof(MINOR_TICKS) / sizeof(MINOR_TICKS[0]);
  const int MINOR_TICK_LENGTH = 3;
  
  const uint16_t DIAL_MAX_RPM = MAJOR_TICKS[MAJOR_TICK_COUNT-1];
  
  const float HALF_CIRCLE_DEGREES = 180.0;
  const float PI_RADIANS = PI/HALF_CIRCLE_DEGREES;
  
  const float MILLIS_PER_SECOND = 1000.0;
  const float SECONDS_PER_MINUTE = 60.0;
  const int DISPLAY_TIMEOUT_INTERVAL = 10 * MILLIS_PER_SECOND;
  const int DISPLAY_DIM_INTERVAL = DISPLAY_TIMEOUT_INTERVAL/2;
  const int DISPLAY_UPDATE_INTERVAL = 250;
  const int  DISPLAY_AVERAGE_INTERVALS = 6;
  const float CM_PER_INCH = 2.54;
  const float  CM_PER_METER = 100.0;
  const float  CM_PER_FOOT = 30.48;

  //If SPEED_DISPLAY is false it will display RPM
  const bool SPEED_DISPLAY = true;
  #ifndef WHEEL_DIAMETER_IN_CM
     #define  WHEEL_DIAMETER_IN_CM  (WHEEL_DIAMETER_IN_INCHES * CM_PER_INCH)
  #endif
  const float WHEEL_CIRCUMFERENCE_IN_CM = WHEEL_DIAMETER_IN_CM * PI;
  
  volatile unsigned long sensor_pulses;

  unsigned long pulse_count[DISPLAY_AVERAGE_INTERVALS]; 
  unsigned long interval_millis[DISPLAY_AVERAGE_INTERVALS]; 
  unsigned int interval_index = 0;
  
  unsigned long previous_millis = 0;
  unsigned long last_sensor_time = 0;
  bool is_oled_display_on = false;
  bool is_oled_display_dim = false;
}

#ifdef OLED_TYPE_SH1106
   Adafruit_SH1106 display(OLED_RESET);
#else
   Adafruit_SSD1306 display(OLED_RESET);
#endif

void setup() {
  Serial.begin(9600);
  initOledDisplayWithI2CAddress(0x3C);
  display.setTextColor(WHITE);
  initArrays();
  
  turnOnIrLED();
  attachPhotodiodeToInterrruptZero();
  last_sensor_time = millis();
  turnOnDisplay();  
}

void initArrays() {
  memset(pulse_count,0,sizeof(pulse_count));
  memset(interval_millis,0,sizeof(interval_millis));
}

void loop() {
  unsigned long current_millis = millis();
  if (current_millis - last_sensor_time >= DISPLAY_TIMEOUT_INTERVAL) {
    turnOffDisplay();
  } else if (current_millis - last_sensor_time >= DISPLAY_DIM_INTERVAL) {
    dimDisplay();
  }  
  
  if (current_millis - previous_millis >= DISPLAY_UPDATE_INTERVAL) {
    previous_millis = current_millis;
    updateDisplay();
  }
}

void initOledDisplayWithI2CAddress(uint8_t i2c_address) {
  #ifdef OLED_TYPE_SH1106
    display.begin(SH1106_SWITCHCAPVCC, i2c_address);
  #else
    display.begin(SSD1306_SWITCHCAPVCC, i2c_address);
  #endif
}

void turnOnDisplay() {
  #ifdef OLED_TYPE_SH1106
    display.SH1106_command(SH1106_DISPLAYON);
  #else
    display.ssd1306_command(SSD1306_DISPLAYON); 
  #endif
  is_oled_display_on = true;
  oledDisplayFullBrightness();
}

void turnOffDisplay() {
  #ifdef OLED_TYPE_SH1106
    display.SH1106_command(SH1106_DISPLAYOFF);
  #else
    display.ssd1306_command(SSD1306_DISPLAYOFF); 
  #endif
  is_oled_display_on = false;
  is_oled_display_dim = false;
}

void dimDisplay() {
  #ifdef OLED_TYPE_SSD1306 
    display.dim(true); 
  #endif
  is_oled_display_dim = true;
}

void oledDisplayFullBrightness() {
  #ifdef OLED_TYPE_SSD1306 
    display.dim(false); 
  #endif
  is_oled_display_dim = false;
}

void turnOnIrLED() {
  pinMode(IR_LED_PIN_3, OUTPUT);
  digitalWrite(IR_LED_PIN_3, HIGH);
}

void attachPhotodiodeToInterrruptZero() {
  pinMode(PHOTODIODE_PIN_2, INPUT_PULLUP);
  attachInterrupt(INTERRUPT_ZERO_ON_PIN_2, incrementSensorPulse, FALLING);
}

void incrementSensorPulse() {
  sensor_pulses++;
}

void updateDisplay() {
  double rpm = calculateRpm();
  if (rpm > 0) {
    last_sensor_time = millis();
    if (!is_oled_display_on || is_oled_display_dim) {
      turnOnDisplay();
    }
  }
  if (is_oled_display_on) {
    display.clearDisplay();
    drawBanner(rpm);
    drawDial(rpm);
    display.display();
  }
}

double calculateRpm() {
  unsigned long current_millis = millis();
  unsigned long current_pulses = sensor_pulses;

  queueIntervalRevolution(current_pulses, current_millis);
  
  unsigned long previous_display_millis = getIntervalMillis();
  unsigned long pulses_over_interval = getIntervalPulses();
  
  unsigned long elapsed_millis =  current_millis - previous_display_millis;
  double elapsed_seconds = ((elapsed_millis * 1.0) / MILLIS_PER_SECOND);
  double delta_interval_pulses = (current_pulses - pulses_over_interval) * 1.0;

  double rpm = (((delta_interval_pulses / elapsed_seconds) * SECONDS_PER_MINUTE) / (WHEEL_REFLECTIVE_STRIP_COUNT * 1.0));
  return rpm;
}

void queueIntervalRevolution(unsigned long revolution_value, unsigned long milliseconds) {
  interval_index++;
  int queue_index = (int)(interval_index % DISPLAY_AVERAGE_INTERVALS);
  pulse_count[queue_index] = revolution_value; 
  interval_millis[queue_index] = milliseconds;
}

unsigned long getIntervalMillis() {
  int index_front_of_queue = (int)((interval_index + 1) % DISPLAY_AVERAGE_INTERVALS);
  return interval_millis[index_front_of_queue];
}

unsigned long getIntervalPulses() {
  int index_front_of_queue = (int)((interval_index + 1) % DISPLAY_AVERAGE_INTERVALS);
  return pulse_count[index_front_of_queue];
}

void drawBanner(double rpm_value) {
  if (SPEED_DISPLAY) {
    drawSpeedBanner(getSpeed(rpm_value));
  } else {
    drawRpmBanner(rpm_value);
  }
}

double getSpeed(double rpm_value) {
  double speedInCmPerMinute = rpm_value * WHEEL_CIRCUMFERENCE_IN_CM;
  double speedValue = speedInCmPerMinute;
  if (isSpeedPerSecond()) {
      speedValue = speedInCmPerMinute / 60;
  }
  if (isMetricUnits()) {
    return speedValue / CM_PER_METER;
  } else {
    return speedValue / CM_PER_FOOT;
  }
}

void drawRpmBanner(double rpm_value) {
  display.setCursor(0, 0);

  display.setTextSize(TEXT_SIZE_LARGE);
  display.print("RPM: ");
  display.print((long)rpm_value);
}

void drawSpeedBanner(double speed_value) {
  display.setCursor(0, 0);

  display.setTextSize(TEXT_SIZE_LARGE);
  if (isMetricUnits()) {
    if (isSpeedPerSecond()) {
      display.print("m/s: ");
    } else {
      display.print("MPM: ");
    }
  } else {
    if (isSpeedPerSecond()) {
      display.print("f/s: ");
    } else {
      display.print("FPM: ");
    } 
  }
  if (isSpeedPerSecond()) {
     display.print(speed_value);
  } else {
     display.print((long)speed_value);    
  }
}

bool isMetricUnits() {
  #ifdef DISPLAY_METRIC_UNITS
     return true;
  #else
     return false;
  #endif
}

bool isSpeedPerSecond() {
  #ifdef SPEED_PER_SECOND
     return true;
  #else
     return false;
  #endif
}

void drawDial(double rpm_value) {
  display.drawCircle(DIAL_CENTER_X, DIAL_CENTER_Y, DIAL_RADIUS, WHITE);
  drawTickMarks();
  drawMajorTickLabels();
  drawIndicatorHand(rpm_value);
}

void drawTickMarks() {
  drawTicks(MAJOR_TICKS, MAJOR_TICK_COUNT, MAJOR_TICK_LENGTH);
  drawTicks(MID_TICKS, MID_TICK_COUNT, MID_TICK_LENGTH);
  drawTicks(MINOR_TICKS, MINOR_TICK_COUNT, MINOR_TICK_LENGTH);
}

void drawTicks(const unsigned int ticks[], int tick_count, int tick_length) {  
  for (int tick_index = 0; tick_index < tick_count; tick_index++) {
    long rpm_tick_value = ticks[tick_index];
    float tick_angle = (HALF_CIRCLE_DEGREES * getPercentMaxRpm(rpm_tick_value)) + HALF_CIRCLE_DEGREES;
    uint16_t dial_x = getCircleXWithLengthAndAngle(DIAL_RADIUS - 1, tick_angle);
    uint16_t dial_y = getCircleYWithLengthAndAngle(DIAL_RADIUS - 1, tick_angle);
    uint16_t tick_x = getCircleXWithLengthAndAngle(DIAL_RADIUS - tick_length, tick_angle);
    uint16_t tick_y = getCircleYWithLengthAndAngle(DIAL_RADIUS - tick_length, tick_angle);
    display.drawLine(dial_x, dial_y, tick_x, tick_y, WHITE);
  }
}

float getPercentMaxRpm(double value) {
  return value/(DIAL_MAX_RPM * 1.0);
}

float getCircleXWithLengthAndAngle(uint16_t radius, float angle) {
  return DIAL_CENTER_X + radius * cos(angle*PI_RADIANS);
};

float getCircleYWithLengthAndAngle(uint16_t radius, float angle) {
  return DIAL_CENTER_Y + radius * sin(angle*PI_RADIANS);
};

void drawMajorTickLabels() {
  display.setTextSize(TEXT_SIZE_SMALL);
  for (int label_index = 0; label_index < MAJOR_TICK_COUNT; label_index++) {
    long rpm_tick_value = MAJOR_TICKS[label_index];
    float tick_angle = (HALF_CIRCLE_DEGREES * getPercentMaxRpm(rpm_tick_value)) + HALF_CIRCLE_DEGREES;
    uint16_t dial_x = getCircleXWithLengthAndAngle(LABEL_RADIUS, tick_angle);
    uint16_t dial_y = getCircleYWithLengthAndAngle(LABEL_RADIUS, tick_angle);
    display.setCursor(dial_x - DIAL_LABEL_X_OFFSET, dial_y - DIAL_LABEL_Y_OFFSET);
    
    int label_value = rpm_tick_value;
    if (DIAL_MAX_RPM > ONE_K) {
      label_value = rpm_tick_value / ONE_K;
    }
    display.print(label_value);
  }
}

void drawIndicatorHand(double rpm_value) {
  double value = rpm_value;
  if (SPEED_DISPLAY) {
    value = getSpeed(rpm_value);
  }
  
  float indicator_angle = (HALF_CIRCLE_DEGREES * getPercentMaxRpm(value)) + HALF_CIRCLE_DEGREES;
  uint16_t indicator_top_x = getCircleXWithLengthAndAngle(INDICATOR_LENGTH, indicator_angle);
  uint16_t indicator_top_y = getCircleYWithLengthAndAngle(INDICATOR_LENGTH, indicator_angle);

  display.drawTriangle(DIAL_CENTER_X - INDICATOR_WIDTH / 2,
                       DIAL_CENTER_Y,DIAL_CENTER_X + INDICATOR_WIDTH / 2,
                       DIAL_CENTER_Y,
                       indicator_top_x, 
                       indicator_top_y, 
                       WHITE);
}
