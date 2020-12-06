#include "FastLED.h"
#include <time.h>


//LED CONFIG
#define PIN 5
#define NUM_LEDS 120          //Number of LEDS on strip
#define LED_TYPE    WS2812B   //LED strip type
#define COLOR_ORDER GRB
#define LED_DENSITY 1         //1 = Every LED is used, 2 = Every second LED is used.. and so on

//WAVE CONFIG
#define W_COUNT 6                 //Number of simultaneous waves
#define W_SPEED_FACTOR 3          //Higher number, higher speed
#define W_WIDTH_FACTOR 3          //Higher number, smaller waves
#define W_COLOR_WEIGHT_PRESET 1   //What color weighting to choose
#define W_RANDOM_SEED 11          //Change this seed for a different pattern. If you read from an analog input here you can get a different pattern everytime.

#define BRIGHTNESS 255

//List of colors allowed for waves
//The first dimension of this array must match the second dimension of the colorwighting array
byte allowedcolors[5][3] ={
  { 17, 177, 13 },    //Greenish
  { 148, 242, 5 },    //Greenish
  { 25, 173, 121},    //Turquoise
  { 250, 77, 127 },   //Pink
  { 171, 101, 221 },  //Purple
};

//Colorweighing allows to give some colors more weight so it is more likely to be choosen for a wave.
//The second dimension of this array must match the first dimension of the allowedcolors array
//Here are 3 presets.
byte colorweighting[3][5] = {
  {10, 10, 10, 10, 10},   //Weighting equal (every color is equally likely)
  {2, 2, 2, 6, 6},        //Weighting reddish (red colors are more likely)
  {6, 6, 6, 2, 2}         //Weighting greenish (green colors are more likely)
};

//Function to get the color for a wave based on the weighting.
//Paramter weighting: First index of colorweighting array. Basically what preset to choose.
byte getWeightedColor(byte weighting) {
  byte sumOfWeights = 0;

  for(byte i = 0; i < sizeof colorweighting[0]; i++) {
    sumOfWeights += colorweighting[weighting][i];
  }

  byte randomweight = random(0, sumOfWeights);
  
  for(byte i = 0; i < sizeof colorweighting[0]; i++) {
    if(randomweight < colorweighting[weighting][i]) {
      return i;
    }
    
    randomweight -= colorweighting[weighting][i];
  }
}

class BorealisWave {
  private:
    int ttl = random(500, 1501);
    byte basecolor = getWeightedColor(W_COLOR_WEIGHT_PRESET);
    float basealpha = random(50, 101) / (float)100;
    int age = 0;
    int width = random(NUM_LEDS / 10, NUM_LEDS / W_WIDTH_FACTOR);
    float center = random(101) / (float)100 * NUM_LEDS;
    bool goingleft = random(0, 2) == 0;
    float speed = random(10, 30) / (float)100 * W_SPEED_FACTOR;
    bool alive = true;

  public:
    CRGB* getColorForLED(int ledIndex) {      
      if(ledIndex < center - width / 2 || ledIndex > center + width / 2) {
        //Position out of range of this wave
        return NULL;
      } else {
        CRGB* rgb = new CRGB();

        //Offset of this led from center of wave
        //The further away from the center, the dimmer the LED
        int offset = abs(ledIndex - center);
        float offsetFactor = (float)offset / (width / 2);

        //The age of the wave determines it brightness.
        //At half its maximum age it will be the brightest.
        float ageFactor = 1;        
        if((float)age / ttl < 0.5) {
          ageFactor = (float)age / (ttl / 2);
        } else {
          ageFactor = (float)(ttl - age) / ((float)ttl * 0.5);
        }

        //Calculate color based on above factors and basealpha value
        rgb -> r = allowedcolors[basecolor][0] * (1 - offsetFactor) * ageFactor * basealpha;
        rgb -> g = allowedcolors[basecolor][1] * (1 - offsetFactor) * ageFactor * basealpha;
        rgb -> b = allowedcolors[basecolor][2] * (1 - offsetFactor) * ageFactor * basealpha;
      
        return rgb;
      }
    };

    //Change position and age of wave
    //Determine if its sill "alive"
    void update() {
      if(goingleft) {
        center -= speed;
      } else {
        center += speed;
      }

      age++;

      if(age > ttl) {
        alive = false;
      } else {
        if(goingleft) {
          if(center + width / 2 < 0) {
            alive = false;
          }
        } else {
          if(center - width / 2 > NUM_LEDS) {
            alive = false;
          }
        }
      }
    };

    bool stillAlive() {
      return alive;
    };
};

CRGB leds[NUM_LEDS];
BorealisWave* waves[W_COUNT];

void setup() {
  randomSeed(W_RANDOM_SEED);
  Serial.begin(9600);
  
  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);

  //Initial creating of waves
  for(int i = 0; i < W_COUNT; i++) {
    waves[i] = new BorealisWave();
  }
}

void loop() {    
  for(int i = 0; i < W_COUNT; i++) {
    //Update values of wave
    waves[i] -> update();

    if(!(waves[i] -> stillAlive())) {
      //If a wave dies, remove it from memory and spawn a new one
      delete waves[i];
      waves[i] = new BorealisWave();
    }
  }

  //Loop through LEDs to determine color
  for(int i = 0; i < NUM_LEDS; i++) {
    if(i % LED_DENSITY != 0) {
      continue;
    }
    
    CRGB mixedRgb = CRGB::Black;

    //For each LED we must check each wave if it is "active" at this position.
    //If there are multiple waves active on a LED we multiply their values.
    for(int  j = 0; j < W_COUNT; j++) {
      CRGB* rgb = waves[j] -> getColorForLED(i);
      
      if(rgb != NULL) {       
        mixedRgb += *rgb;
      }
      
      delete []rgb;
    }
    setPixel(i, mixedRgb[0], mixedRgb[1], mixedRgb[2]);
  }

  showStrip();
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
}

void showStrip() {
   FastLED.show();
}
