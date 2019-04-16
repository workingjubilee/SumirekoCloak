/*
 *  Sumireko Usami Cloak Firmware
 *  by Alice D
 * 
 *  Using: 
 *  * Adafruit Feather M0 Express
 *  * Adafruit Feather NeoPXL87
 *  * Adafruit Feather Hallowing
 *  * Adafruit INA219 Breakout Board
 *  * Adafruit I2C OLED Display
 *  * DFRobot DC-DC Power Module 25W (x2)
 *  * KitterLabs RobotEars v1.0 (self-made part)
 *  * 16 meters WS2811 UV LEDs
 * 
 */

#include <Wire.h>
#include <Adafruit_NeoPXL8.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define NUM_LED 64  // Multiply by 8, as per NeoPXL8 documentation

// SDA pin (I2C) remapped to MOSI
int8_t pins[8] = { A3, MOSI, A4, 5, MISO, PIN_SERIAL1_TX, 13, PIN_SERIAL1_RX};

Adafruit_NeoPXL8 leds(NUM_LED, pins, NEO_GRB);

Adafruit_INA219 ina219;

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

#define OLED_RESET     4
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

uint32_t total_sec = 0;
float total_mA = 0.0;

// mapping for the cape LEDs
// there are 14 strips on the cloak, at an irregular strip on the collar
// LEDs starting at 448 are on the collar in one continuous strip

#define NUM_LED_MATRIX_HEIGHT 16
#define NUM_LED_MATRIX_WIDTH 32
#define NUM_LED_TOTAL 512 // total LEDs

uint16_t cape_matrix[32][16] = {
0, 63, 64, 127, 128, 191, 192, 255, 256, 319, 320, 383, 384, 447, 448, 511,
1, 62, 65, 126, 129, 190, 193, 254, 257, 318, 321, 382, 385, 446, 449, 510,
2, 61, 66, 125, 130, 189, 194, 253, 258, 317, 322, 381, 386, 445, 450, 509,
3, 60, 67, 124, 131, 188, 195, 252, 259, 316, 323, 380, 387, 444, 451, 508,
4, 59, 68, 123, 132, 187, 196, 251, 260, 315, 324, 379, 388, 443, 452, 507,
5, 58, 69, 122, 133, 186, 197, 250, 261, 314, 325, 378, 389, 442, 453, 506,
6, 57, 70, 121, 134, 185, 198, 249, 262, 313, 326, 377, 390, 441, 454, 505,
7, 56, 71, 120, 135, 184, 199, 248, 263, 312, 327, 376, 391, 440, 455, 504,
8, 55, 72, 119, 136, 183, 200, 247, 264, 311, 328, 375, 392, 439, 456, 503,
9, 54, 73, 118, 137, 182, 201, 246, 265, 310, 329, 374, 393, 438, 457, 502,
10, 53, 74, 117, 138, 181, 202, 245, 266, 309, 330, 373, 394, 437, 458, 501,
11, 52, 75, 116, 139, 180, 203, 244, 267, 308, 331, 372, 395, 436, 459, 500,
12, 51, 76, 115, 140, 179, 204, 243, 268, 307, 332, 371, 396, 435, 460, 499,
13, 50, 77, 114, 141, 178, 205, 242, 269, 306, 333, 370, 397, 434, 461, 498,
14, 49, 78, 113, 142, 177, 206, 241, 270, 305, 334, 369, 398, 433, 462, 497,
15, 48, 79, 112, 143, 176, 207, 240, 271, 304, 335, 368, 399, 432, 463, 496,
16, 47, 80, 111, 144, 175, 208, 239, 272, 303, 336, 367, 400, 431, 464, 495,
17, 46, 81, 110, 145, 174, 209, 238, 273, 302, 337, 366, 401, 430, 465, 494,
18, 45, 82, 109, 146, 173, 210, 237, 274, 301, 338, 365, 402, 429, 466, 493,
19, 44, 83, 108, 147, 172, 211, 236, 275, 300, 339, 364, 403, 428, 467, 492,
20, 43, 84, 107, 148, 171, 212, 235, 276, 299, 340, 363, 404, 427, 468, 491,
21, 42, 85, 106, 149, 170, 213, 234, 277, 298, 341, 362, 405, 426, 469, 490,
22, 41, 86, 105, 150, 169, 214, 233, 278, 297, 342, 361, 406, 425, 470, 489,
23, 40, 87, 104, 151, 168, 215, 232, 279, 296, 343, 360, 407, 424, 471, 488,
24, 39, 88, 103, 152, 167, 216, 231, 280, 295, 344, 359, 408, 423, 472, 487,
25, 38, 89, 102, 153, 166, 217, 230, 281, 294, 345, 358, 409, 422, 473, 486,
26, 37, 90, 101, 154, 165, 218, 229, 282, 293, 346, 357, 410, 421, 474, 485,
27, 36, 91, 100, 155, 164, 219, 228, 283, 292, 347, 356, 411, 420, 475, 484,
28, 35, 92, 99, 156, 163, 220, 227, 284, 291, 348, 355, 412, 419, 476, 483,
29, 34, 93, 98, 157, 162, 221, 226, 285, 290, 349, 354, 413, 418, 477, 482,
30, 33, 94, 97, 158, 161, 222, 225, 286, 289, 350, 353, 414, 417, 478, 481,
31, 32, 95, 96, 159, 160, 223, 224, 287, 288, 351, 352, 415, 416, 479, 480,
};

uint8_t brightness = 0;
uint8_t frame = 0;

void setup() {
  // A5 = Potentiometer
  // 11 = Button (internal pulldown! set this!)
  pinMode(11, INPUT_PULLUP); // physical button on side of MCU case
  pinMode(A5, INPUT); // knob on side of MCU case
  attachInterrupt(A5, input_checks, CHANGE);

  // LEDs
  leds.begin();
  brightness = (analogRead(A5) / 4);
  leds.setBrightness(brightness);

  // I2C section
  Wire.begin(); // begin I2C bus
  
  ina219.begin(); // begin power-monitoring breakout board
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // begin display
  display.clearDisplay();
  display.display();
  display.setRotation(0);
  display.setTextSize(2);
  display.setTextColor(WHITE);

  lis.begin(0x18);
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setClick(2, 500);

  //Serial.begin(9600);
};

void loop() {
  //rain();
  int buttonState = digitalRead(11);
  //Serial.println(buttonState); 
  //input_checks(); // slow it down a bit
  //delay(1);
};

void input_checks() {
  brightness = (analogRead(A5) / 4);
  leds.setBrightness(brightness);
  //delay(1); // placeholder
};



// Given row number (0-7) and pixel number along row (0 - (NUM_LED-1)),
// first calculate brightness (b) of pixel, then multiply row color by
// this
void rain() {
  for(uint8_t r=0; r<NUM_LED_MATRIX_WIDTH; r++) { // For each row...
    for(uint8_t p=0; p<NUM_LED_MATRIX_HEIGHT; p++) { // For each pixel of row...

      uint16_t b = 256 - ((frame - r  + p * 256 / NUM_LED) & 0xFF);
      leds.setPixelColor(
              cape_matrix[r][p],
             ((uint32_t)(255 * b) >> 8 << 16) |
             ((uint32_t)(255 * b) >> 8 <<  8) |
             (255 * b) >> 8
             );

    }
  }
  leds.show(); // display LEDs
  frame++;
}


// INA219 / SSD1306 section
// mostly taken from their example code 

void power_check() {
  // Read voltage and current from INA219.
  float shuntvoltage = ina219.getShuntVoltage_mV();
  float busvoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  
  // Compute load voltage, power, and milliamp-hours.
  float loadvoltage = busvoltage + (shuntvoltage / 1000);  
  float power_mW = loadvoltage * current_mA;
  total_mA += current_mA;
  total_sec += 1;
  float total_mAH = total_mA / 3600.0;  
  
  // Update display.
  display.clearDisplay();
  display.setCursor(0,0);
  int mode = (total_sec / 5) % 2;
  if (mode == 0) {
    // Mode 0, display volts and amps.
    printSIValue(loadvoltage, "V:", 2, 10);
    display.setCursor(0, 16);
    printSIValue(current_mA/1000.0, "A:", 5, 10);
  }
  else {
    // Mode 1, display watts and milliamp-hours.
    printSIValue(power_mW/1000.0, "W:", 5, 10);
    display.setCursor(0, 16);
    printSIValue(total_mAH/1000.0, "Ah:", 5, 10);
  }
  display.display();

  
}

void printSIValue(float value, char* units, int precision, int maxWidth) {
  // Print a value in SI units with the units left justified and value right justified.
  // Will switch to milli prefix if value is below 1.
  
  // Add milli prefix if low value.
  if (fabs(value) < 1.0) {
    display.print('m');
    maxWidth -= 1;
    value *= 1000.0;
    precision = max(0, precision-3);
  }
  
  // Print units.
  display.print(units);
  maxWidth -= strlen(units);
  
  // Leave room for negative sign if value is negative.
  if (value < 0.0) {
    maxWidth -= 1;
  }
  
  // Find how many digits are in value.
  int digits = ceil(log10(fabs(value)));
  if (fabs(value) < 1.0) {
    digits = 1; // Leave room for 0 when value is below 0.
  }
  
  // Handle if not enough width to display value, just print dashes.
  if (digits > maxWidth) {
    // Fill width with dashes (and add extra dash for negative values);
    for (int i=0; i < maxWidth; ++i) {
      display.print('-');
    }
    if (value < 0.0) {
      display.print('-');
    }
    return;
  }
  
  // Compute actual precision for printed value based on space left after
  // printing digits and decimal point.  Clamp within 0 to desired precision.
  int actualPrecision = constrain(maxWidth-digits-1, 0, precision);
  
  // Compute how much padding to add to right justify.
  int padding = maxWidth-digits-1-actualPrecision;
  for (int i=0; i < padding; ++i) {
    display.print(' ');
  }
  
  // Finally, print the value!
  display.print(value, actualPrecision);
}
