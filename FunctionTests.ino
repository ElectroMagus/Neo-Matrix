#include "application.h"
SYSTEM_MODE(AUTOMATIC);


#include <neopixel.h>
#include <neomatrix.h>
#include <Adafruit_GFX.h>

#define PIN 2
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);

// Bitmaps, Pixmaps, Color Macros,
#include "matrixPixmaps.h"
// Temp Bitmap
#include "smileytongue24.h"

// Uptime Variables
unsigned long lastTime = 0UL;
char publishString[40];
char brightnessString[40];

// Light Settings and Internet Control Variables
//int DEF_BRIGHTNESS    = 200;
int brightness = 120;
String mode = "Temp";


void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup loop...");

    Serial.println("Setting up Cloud Variables...");
    Particle.variable("mode", &mode, STRING);
    Particle.variable("brightness", &brightness, INT);

    Serial.println("Setting up Cloud Functions...");
    Particle.function("setbright", setbrightness);
    Particle.function("setmode", setMode);

    Serial.println("Sending first status to feed...");
    publishUptime("now");

    Serial.println("Initializing Neopixel Matrix...");
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(brightness);
    Serial.print("Brightness Set To:  ");
    Serial.println(brightness);
    // Test full bright of all LEDs. If brightness is too high
    // for your current limit (i.e. USB), decrease it.
    matrix.fillScreen(LED_PURPLE_HIGH);
    matrix.show();
    Serial.println("It should be pretty purple in there right now.");
    delay(10000);
    Serial.println("Now, there.  All better?");
    matrix.clear();
}

// Cloud Functions and basic light controls
int publishUptime(String command) {
     unsigned long now = millis();
    //Every 15 seconds publish uptime
    if (now-lastTime>30000UL) {
        lastTime = now;
        // now is in milliseconds
        unsigned nowSec = now/1000UL;
        unsigned sec = nowSec%60;
        unsigned min = (nowSec%3600)/60;
        unsigned hours = (nowSec%86400)/3600;
        sprintf(publishString,"%u:%u:%u",hours,min,sec);
        Particle.publish("Uptime",publishString);
        Particle.publish("Mode",mode);
        sprintf(brightnessString, "%u",brightness);
        Particle.publish("Brightness",brightnessString);
        return 1;
    }
    else return -1;
}

int setMode(String value) {
    mode = value;
    return 1;
}

int setbrightness(String value) {
//    unsigned temp = value.toInt();
//    brightness = temp * 2.5;
    brightness = value.toInt();
    if ((brightness >= 0) && (brightness <= 255))    {            // Quick inefficent sanity check on valueBrightness.
        return 1;
    }
    else return -1;
}




// Max is 255, 32 is a conservative value to not overload
// a USB power supply (500mA) for 12x12 pixels.
//#define BRIGHTNESS 190

// Define matrix width and height.
#define mw 16
#define mh 16





 // Draw colored bitmap (each pixel is a uint16_t, with colors defined by
 // the drawpixel method of your graphical backend.
 // progmem defaults to true for bitmaps defined as static const uint16_t PROGMEM
 // but you can set it to false to send a bitmap array in RAM.

// Updated 10/20/17-   The Particle Neomatrix Includes didn't have the most recent library and called this function.  Changed things around a bit to make it a local function
// and the syntax on how objects are called for compatibility.

 void drawRGBBitmap(int16_t x, int16_t y,
 		const uint16_t *bitmap, int16_t w, int16_t h) {
   int16_t i, j;

   for(j=0; j<h; j++) {
     for(i=0; i<w; i++ ) {
       matrix.drawPixel(x+i, y+j, (uint16_t) *(bitmap + j * w + i));
                          }
                      }
  }

// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
    uint16_t RGB_bmp_fixed[w * h];
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
	uint8_t r,g,b;
	uint16_t color = pgm_read_word(bitmap + pixel);

	//Serial.print(color, HEX);
	b = (color & 0xF00) >> 8;
	g = (color & 0x0F0) >> 4;
	r = color & 0x00F;
	//Serial.print(" ");
	//Serial.print(b);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(r);
	//Serial.print(" -> ");
	// expand from 4/4/4 bits per color to 5/6/5
	b = map(b, 0, 15, 0, 31);
	g = map(g, 0, 15, 0, 63);
	r = map(r, 0, 15, 0, 31);
	//Serial.print(r);
	//Serial.print("/");
	//Serial.print(g);
	//Serial.print("/");
	//Serial.print(b);
	RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
	//Serial.print(" -> ");
	//Serial.println(RGB_bmp_fixed[pixel], HEX);
    }
    drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

// Fill the screen with multiple levels of white to gauge the quality
void display_four_white() {
    matrix.clear();
    matrix.fillRect(0,0, mw,mh, LED_WHITE_HIGH);
    matrix.drawRect(1,1, mw-2,mh-2, LED_WHITE_MEDIUM);
    matrix.drawRect(2,2, mw-4,mh-4, LED_WHITE_LOW);
    matrix.drawRect(3,3, mw-6,mh-6, LED_WHITE_VERYLOW);
    matrix.show();
}

void display_bitmap(uint8_t bmp_num, uint16_t color) {
    static uint16_t bmx,bmy;

    // Clear the space under the bitmap that will be drawn as
    // drawing a single color pixmap does not write over pixels
    // that are nul, and leaves the data that was underneath
    matrix.fillRect(bmx,bmy, bmx+8,bmy+8, LED_BLACK);
    matrix.drawBitmap(bmx, bmy, mono_bmp[bmp_num], 8, 8, color);
    bmx += 8;
    if (bmx >= mw) bmx = 0;
    if (!bmx) bmy += 8;
    if (bmy >= mh) bmy = 0;
    matrix.show();
}

void display_rgbBitmap(uint8_t bmp_num) {
    static uint16_t bmx,bmy;

    fixdrawRGBBitmap(bmx, bmy, RGB_bmp[bmp_num], 8, 8);
    bmx += 8;
    if (bmx >= mw) bmx = 0;
    if (!bmx) bmy += 8;
    if (bmy >= mh) bmy = 0;
    matrix.show();
}

void display_lines() {
    matrix.clear();

    // 4 levels of crossing red lines.
    matrix.drawLine(0,mh/2-2, mw-1,2, LED_RED_VERYLOW);
    matrix.drawLine(0,mh/2-1, mw-1,3, LED_RED_LOW);
    matrix.drawLine(0,mh/2,   mw-1,mh/2, LED_RED_MEDIUM);
    matrix.drawLine(0,mh/2+1, mw-1,mh/2+1, LED_RED_HIGH);

    // 4 levels of crossing green lines.
    matrix.drawLine(mw/2-2, 0, mw/2-2, mh-1, LED_GREEN_VERYLOW);
    matrix.drawLine(mw/2-1, 0, mw/2-1, mh-1, LED_GREEN_LOW);
    matrix.drawLine(mw/2+0, 0, mw/2+0, mh-1, LED_GREEN_MEDIUM);
    matrix.drawLine(mw/2+1, 0, mw/2+1, mh-1, LED_GREEN_HIGH);

    // Diagonal blue line.
    matrix.drawLine(0,0, mw-1,mh-1, LED_BLUE_HIGH);
    matrix.drawLine(0,mh-1, mw-1,0, LED_ORANGE_MEDIUM);
    matrix.show();
}

void display_boxes() {
    matrix.clear();
    matrix.drawRect(0,0, mw,mh, LED_BLUE_HIGH);
    matrix.drawRect(1,1, mw-2,mh-2, LED_GREEN_MEDIUM);
    matrix.fillRect(2,2, mw-4,mh-4, LED_RED_HIGH);
    matrix.fillRect(3,3, mw-6,mh-6, LED_ORANGE_MEDIUM);
    matrix.show();
}

void display_circles() {
    matrix.clear();
    matrix.drawCircle(mw/2,mh/2, 2, LED_RED_MEDIUM);
    matrix.drawCircle(mw/2-1-min(mw,mh)/8, mh/2-1-min(mw,mh)/8, min(mw,mh)/4, LED_BLUE_HIGH);
    matrix.drawCircle(mw/2+1+min(mw,mh)/8, mh/2+1+min(mw,mh)/8, min(mw,mh)/4-1, LED_ORANGE_MEDIUM);
    matrix.drawCircle(1,mh-2, 1, LED_GREEN_LOW);
    matrix.drawCircle(mw-2,1, 1, LED_GREEN_HIGH);
    if (min(mw,mh)>12) matrix.drawCircle(mw/2-1, mh/2-1, min(mh/2-1,mw/2-1), LED_CYAN_HIGH);
    matrix.show();
}

void display_resolution() {
    // not wide enough;
    if (mw<=16) return;
    matrix.clear();
    // Font is 5x7, if display is too small
    // 8 can only display 1 char
    // 16 can almost display 3 chars
    // 24 can display 4 chars
    // 32 can display 5 chars
    matrix.setCursor(0, 0);
    matrix.setTextColor(matrix.Color(255,0,0));
    if (mw>10) matrix.print(mw/10);
    matrix.setTextColor(matrix.Color(255,128,0));
    matrix.print(mw % 10);
    matrix.setTextColor(matrix.Color(0,255,0));
    matrix.print('x');
    // not wide enough to print 5 chars, go to next line
    if (mw<25) {
	if (mh==13) matrix.setCursor(6, 7);
	else if (mh>=13) {
	    matrix.setCursor(mw-11, 8);
	} else {
	    matrix.show();
	    delay(2000);
	    matrix.clear();
	    matrix.setCursor(mw-11, 0);
	}
    }
    matrix.setTextColor(matrix.Color(0,255,128));
    matrix.print(mh/10);
    matrix.setTextColor(matrix.Color(0,128,255));
    matrix.print(mh % 10);
    // enough room for a 2nd line
    if (mw>25 && mh >14 || mh>16) {
	     matrix.setCursor(0, mh-7);
	     matrix.setTextColor(matrix.Color(0,255,255));
	     if (mw>16) matrix.print('*');
	      matrix.setTextColor(matrix.Color(255,0,0));
	      matrix.print('R');
	      matrix.setTextColor(matrix.Color(0,255,0));
	      matrix.print('G');
	      matrix.setTextColor(matrix.Color(0,0,255));
	      matrix.print("B");
	      matrix.setTextColor(matrix.Color(255,255,0));
	      matrix.print("*");
    }
    matrix.show();
}


// display_scrollText() - 10/20/2017 -
int display_scrollText(String word1, String word2) {
    matrix.clear();
    matrix.setTextWrap(false);                                  // Text Wrapping is a PITA
    matrix.setTextSize(1);
    matrix.setRotation(0);
    for (int8_t x=7; x>=-42; x--) {
	     matrix.clear();                                        // Need to do some testing on TextSize and sanity checking on input values for word1 and word2
	     matrix.setCursor(x,0);
	     matrix.setTextColor(LED_BLUE_HIGH);
	     matrix.print(word1);
	     if (mh>11) {
	        matrix.setCursor(-20-x,mh-7);
	        matrix.setTextColor(LED_RED_HIGH);
	        matrix.print(word2);
	       }
	      matrix.show();
        delay(20);
    }
/*  matrix.setRotation(3);
    matrix.setTextColor(LED_BLUE_HIGH);
    for (int8_t x=7; x>=-45; x--) {                             // Example of rotation option.  Text approachs North from South
	     matrix.clear();
	     matrix.setCursor(x,mw/2-4);
	     matrix.print("Rotate");
	     matrix.show();
       delay(50);
    }
    matrix.setRotation(0);
    matrix.setCursor(0,0);
    matrix.show();
*/
}

// Scroll within big bitmap so that all if it becomes visible or bounce a small one.
// If the bitmap is bigger in one dimension and smaller in the other one, it will
// be both panned and bounced in the appropriate dimensions.
void display_panOrBounceBitmap (uint8_t bitmapSize) {
    // keep integer math, deal with values 16 times too big
    // start by showing upper left of big bitmap or centering if the display is big
    int16_t xf = max(0, (mw-bitmapSize)/2) << 4;
    int16_t yf = max(0, (mh-bitmapSize)/2) << 4;
    // scroll speed in 1/16th
    int16_t xfc = 6;
    int16_t yfc = 3;
    // scroll down and right by moving upper left corner off screen
    // more up and left (which means negative numbers)
    int16_t xfdir = -1;
    int16_t yfdir = -1;

    for (uint16_t i=1; i<1000; i++) {
	bool updDir = false;

	// Get actual x/y by dividing by 16.
	int16_t x = xf >> 4;
	int16_t y = yf >> 4;

	matrix.clear();
	// bounce 8x8 tri color smiley face around the screen
	if (bitmapSize == 8) fixdrawRGBBitmap(x, y, RGB_bmp[10], 8, 8);
	// pan 24x24 pixmap
	if (bitmapSize == 24) drawRGBBitmap(x, y, (const uint16_t *) yellowsmiley24, bitmapSize, bitmapSize);
	matrix.show();

	// Only pan if the display size is smaller than the pixmap
	// but not if the difference is too small or it'll look bad.
	if (bitmapSize-mw>2) {
	    if (mw>9) xf += xfc*xfdir;
	    if (xf >= 0)                      { xfdir = -1; updDir = true ; };
	    // we don't go negative past right corner, go back positive
	    if (xf <= ((mw-bitmapSize) << 4)) { xfdir = 1;  updDir = true ; };
	}
	if (bitmapSize-mh>2) {
	    yf += yfc*yfdir;
	    // we shouldn't display past left corner, reverse direction.
	    if (yf >= 0)                      { yfdir = -1; updDir = true ; };
	    if (yf <= ((mh-bitmapSize) << 4)) { yfdir = 1;  updDir = true ; };
	}
	// only bounce a pixmap if it's smaller than the display size
	if (mw>bitmapSize) {
	    xf += xfc*xfdir;
	    // Deal with bouncing off the 'walls'
	    if (xf >= (mw-bitmapSize) << 4) { xfdir = -1; updDir = true ; };
	    if (xf <= 0)           { xfdir =  1; updDir = true ; };
	}
	if (mh>bitmapSize) {
	    yf += yfc*yfdir;
	    if (yf >= (mh-bitmapSize) << 4) { yfdir = -1; updDir = true ; };
	    if (yf <= 0)           { yfdir =  1; updDir = true ; };
	}

	if (updDir) {
	    // Add -1, 0 or 1 but bind result to 1 to 1.
	    // Let's take 3 is a minimum speed, otherwise it's too slow.
	    xfc = constrain(xfc + random(-1, 2), 3, 16);
	    yfc = constrain(xfc + random(-1, 2), 3, 16);
	}
	delay(10);
    }
}


void loop() {
    Serial.println("Primary loop starting...");

    // clear the screen after X bitmaps have been displayed and we
    // loop back to the top left corner
    // 8x8 => 1, 16x8 => 2, 17x9 => 6
    static uint8_t pixmap_count = ((mw+7)/8) * ((mh+7)/8);

    Serial.print("Screen pixmap capacity: ");
    Serial.println(pixmap_count);

    Serial.println("Starting the display_scrollText() function");
    // Scroll Text Function
    display_scrollText("ABCDEFG","ZYXWVUT");


  //  multicolor bitmap sent as many times as we can display an 8x8 pixmap
  //  for (uint8_t i=0; i<=pixmap_count; i++)
  //  {
	//    display_rgbBitmap(0);
  //  }
  //  delay(1000);

  //  display_resolution();
  //  delay(3000);

// Cycle through red, green, blue, display 2 checkered patterns
// useful to debug some screen types and alignment.
//    uint16_t bmpcolor[] = { LED_GREEN_HIGH, LED_BLUE_HIGH, LED_RED_HIGH };
//    for (uint8_t i=0; i<3; i++)
//      {
//	       display_bitmap(0, bmpcolor[i]);
//	       delay(500);
//         display_bitmap(1, bmpcolor[i]);
// 	        delay(500);
//      }

    // Display 3 smiley faces.
//    for (uint8_t i=2; i<=4; i++)
//      {
//	       display_bitmap(i, bmpcolor[i-2]);
	       // If more than one pixmap displayed per screen, display more quickly.
//	       delay(mw>8?500:1500);
//      }
        // If we have multiple pixmaps displayed at once, wait a bit longer on the last.
//        delay(mw>8?1000:500);

//    display_lines();
//    delay(3000);

//    display_boxes();
//    delay(3000);

//    display_circles();
//    matrix.clear();
//    delay(3000);

    for (uint8_t i=0; i<=(sizeof(RGB_bmp)/sizeof(RGB_bmp[0])-1); i++)
      {
	       display_rgbBitmap(i);
	        delay(mw>8?500:1500);
      }
    // If we have multiple pixmaps displayed at once, wait a bit longer on the last.
    delay(mw>8?1000:500);

    display_four_white();
    delay(3000);

//    display_scrollText();

    // pan a big pixmap
//    display_panOrBounceBitmap(24);
    // bounce around a small one
//    display_panOrBounceBitmap(8);
}
