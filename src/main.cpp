#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

//function decs
void doEncoderA();
int ledCount(int encoder0Pos);
void loadingChase(int speed, uint32_t color, int loops, Adafruit_NeoPixel strip);

//PIN definition
#define encoder0PinA  2
#define encoder0PinB  3

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN_1     6
#define LED_PIN_2     7
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  24
// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 20

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip1(LED_COUNT, LED_PIN_1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT, LED_PIN_2, NEO_GRBW + NEO_KHZ800);

volatile int encoder0Pos,wheelRevs = 0;

/* Pololu 3499 has 20 counts per rev when counting both edges of both channels
*  So just counting one edge of one channel, I have 5 counts per rev, and gear ratio of 31.25 = 156.25 counts per rev of gearbox output shaft.
*  Spur gear to ring gear ratio is: Spur gear PCD=15mm, PCD internal ring =120mm 120/15=8
*  156.25*8= 1250 counts per rev of wheel
*/
float ratio = 1250.0;


void setup()
{
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  digitalWrite(encoder0PinA, HIGH);  //turn on pullup resistor
  digitalWrite(encoder0PinB, HIGH);  //turn on pullup resistor

  // encoder A channel on interrupt 0 (Arduino's pin 2)
  attachInterrupt(digitalPinToInterrupt(2), doEncoderA, RISING); //Due to gear ratio we dont need to track every pulse so just using interup on one channel Rising.
  // encoder B channel pin on interrupt 1 (Arduino's pin 3)
  //attachInterrupt(digitalPinToInterrupt(3), doEncoderB, RISING);

  strip1.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip1.show();            // Turn OFF all pixels ASAP
  strip1.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip2.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip2.show();            // Turn OFF all pixels ASAP
  strip2.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
  loadingChase(10, strip1.Color(0,   0,   100, 0), 24, strip1);
  loadingChase(10, strip2.Color(0,   0,   100, 0), 24, strip2);
  strip1.fill(strip1.Color(0,   0,   100, 0));
}


void loop()
{
  Serial.print("LED");
  Serial.println(ledCount(encoder0Pos));
  Serial.print("Wheel Revs:");
  Serial.println(wheelRevs);
  strip1.setPixelColor(abs(ledCount(encoder0Pos)),strip1.Color(0,   255,   0, 0) );
  strip1.show();

//  delay(100);
}

void doEncoderA(){
  if(digitalRead(encoder0PinA)==digitalRead(encoder0PinB)){
    encoder0Pos++;
    if (encoder0Pos >1250){
      encoder0Pos = 0;
      wheelRevs++;
    }
  }
  else{
    encoder0Pos--;
    if (encoder0Pos <-1250){
      encoder0Pos = 0;
      wheelRevs--;
    }
  }
}

int ledCount(int encoder0Pos){ //given encoder position calc which LED to turn on
    float wheelRev = encoder0Pos/ratio; //calc wheel revs
    int ledNum = wheelRev * 24; //24 LEDS in ring
    return ledNum;
}

void loadingChase(int speed, uint32_t color, int loops, Adafruit_NeoPixel strip) {
  int      length        = 1;
  int      head          = length - 1;
  int      tail          = 0;
  int      loopNum       = 0;
  uint32_t lastTime      = millis();

  for(;;) { // Repeat forever (or until a 'break' or 'return')
    for(int i=0; i<strip.numPixels(); i++) {  // For each pixel in strip...
      if(((i >= tail) && (i <= head)) ||      //  If between head & tail...
         ((tail > head) && ((i >= tail) || (i <= head)))) {
        strip.setPixelColor(i, color); // Set colour
      } else {                                             // else off
        strip.setPixelColor(i, strip.gamma32(strip.Color(0,0,0,0)));
      }
    }

    strip.show(); // Update strip with new contents
    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.

    if((millis() - lastTime) > speed) { // Time to update head/tail?
      if(++head >= strip.numPixels()) {      // Advance head, wrap around
        if(++loopNum >= loops) return;
        head = 0;
        Serial.println(loopNum);
      }
      if(++tail >= strip.numPixels()) {      // Advance tail, wrap around
        tail = 0;
        head = ++head;
      }
      lastTime = millis();                   // Save time of last movement
    }
  }
}
