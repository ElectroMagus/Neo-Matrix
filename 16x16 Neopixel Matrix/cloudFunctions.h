// Uptime Variables
unsigned long lastTime = 0UL;
char publishString[40];
char brightnessString[40];

// Light Settings and Internet Control Variables
uint8_t DEF_BRIGHTNESS=200;
String mode = "Blue";
int brightness = DEF_BRIGHTNESS;

void publishUptime() {
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
        Spark.publish("Uptime",publishString);
        Spark.publish("Mode",mode);
        sprintf(brightnessString, "%u",brightness);
        Spark.publish("Brightness",brightnessString);
    }
}
// Spark Cloud Functions
void int setBrightness(String value) {
//    unsigned temp = value.toInt();
//    brightness = temp * 2.5;
    brightness = value.toInt();
}

void int setMode(String value) {
    mode = value;
}
