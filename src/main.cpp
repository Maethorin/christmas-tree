#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 240
#define BRANCH_SIZE 60
#define BRANCHS 6

CRGB leds[NUM_LEDS];

void setup() {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
    FastLED.setBrightness(200);
    FastLED.clear();
    FastLED.show();
    Serial.begin( 9600 );
}

void showStrip() {
    FastLED.show();
}

void setPixel(int pixel, byte red, byte green, byte blue) {
    leds[pixel].r = red;
    leds[pixel].g = green;
    leds[pixel].b = blue;
}

void setAll(byte red, byte green, byte blue) {
    for (int i = 0; i < NUM_LEDS; i++) {
        setPixel(i, red, green, blue);
    }
    showStrip();
}

void setEyeColors(int eyeIndex, int eyeSize, byte red, byte green, byte blue) {
    setPixel(eyeIndex, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= eyeSize; j++) {
        setPixel(eyeIndex + j, red, green, blue);
    }
    setPixel(eyeIndex + eyeSize + 1, red / 10, green / 10, blue / 10);
}

void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount) {
    if( cur == target) return;

    if( cur < target ) {
        uint8_t delta = target - cur;
        delta = scale8_video( delta, amount);
        cur += delta;
    } else {
        uint8_t delta = cur - target;
        delta = scale8_video( delta, amount);
        cur -= delta;
    }
}

CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount) {
    nblendU8TowardU8( cur.red,   target.red,   amount);
    nblendU8TowardU8( cur.green, target.green, amount);
    nblendU8TowardU8( cur.blue,  target.blue,  amount);
    return cur;
}

void cylonBounce(byte goRed, byte goGreen, byte goBlue, byte backRed, byte backGreen, byte backBlue, int eyeSize, int speedDelay, int returnDelay, int chunkSize) {
    int multiplier = NUM_LEDS / chunkSize;
    for (int i = 0; i < chunkSize - eyeSize - 2; i++) {
        setAll(0, 0, 0);
        for (int multi = 0; multi < multiplier; multi++) {
            int sumValue = multi * chunkSize;
            setEyeColors(i + sumValue, eyeSize, goRed, goGreen, goBlue);
        }
        showStrip();
        delay(speedDelay);
    }

    delay(returnDelay);

    for (int i = chunkSize - eyeSize - 2; i > 0; i--) {
        setAll(0, 0, 0);
        for (int multi = 0; multi < multiplier; multi++) {
            int SumValue = multi * chunkSize;
            setEyeColors(i + SumValue, eyeSize, backRed, backGreen, backBlue);
        }
        showStrip();
        delay(speedDelay);
    }

    delay(returnDelay);
}

void starsBlink(int starsQuantity, bool decreaseNumber, int startLimit, int fadeTime, int fadeAmount, int blinkTime, CRGB colors[], int colorsQuantity) {
    CRGB targetColors;
    CRGB starsColors = colors[0];
    int endLimit = (startLimit + fadeTime) * (colorsQuantity + 1) + fadeTime;
    int tenthPart = int(endLimit / 10);
    for (int times = 0; times < endLimit; ++times) {
        setAll(0, 0, 0);
        int firstLimit = startLimit;
        int secondLimit = firstLimit + fadeTime;
        for (int color = 0; color < colorsQuantity; ++color) {
            if (times > firstLimit && times <= secondLimit) {
                targetColors = colors[color];
            }
            firstLimit = secondLimit + startLimit;
            secondLimit = firstLimit + fadeTime;
        }
        if (times > startLimit) {
            fadeTowardColor(starsColors, targetColors, fadeAmount);
        }
        int ledQtd = decreaseNumber ? int(NUM_LEDS / (int(times / tenthPart) * 4 + 1)) : starsQuantity;
        int randLeds[ledQtd];
        for (int & randLed : randLeds) {
            randLed = random(NUM_LEDS);
        }
        for (int & index : randLeds) {
            setPixel(index, starsColors.r, starsColors.g, starsColors.b);
        }
        showStrip();
        delay(blinkTime);
    }
}

void doCylon() {
    int eyeSize;
    int returnDelay = 80;
    int chunkSize = BRANCH_SIZE;
    int times = 4;
    for (int cylonLoop = 1; cylonLoop <= 10; cylonLoop++) {
        int speedMultiplier = int(cylonLoop / 2);
        if (speedMultiplier < 1) {
            speedMultiplier = 1;
        }
        int speedDelay = 10 * speedMultiplier;
        if (BRANCH_SIZE % cylonLoop == 0) {
            chunkSize = BRANCH_SIZE / cylonLoop;
        }
        eyeSize = int(chunkSize / 10);
        if (eyeSize < 1) {
            eyeSize = 1;
        }
        if (cylonLoop > 3 && cylonLoop < 6) {
            times = 6;
        }
        else if (cylonLoop >= 6 && cylonLoop < 10) {
            times = 3;
        }
        else if (cylonLoop == 10) {
            times = 16;
        }

        for (int innerLoop = 0; innerLoop < times; innerLoop++) {
            cylonBounce(
                    0xff, 0, 0,
                    0, 0xff, 0,
                    eyeSize, speedDelay, returnDelay, chunkSize
            );
        }
    }
}

void shootingStar() {
    for (int times = 4; times > 0; --times) {
        for (int loop = 0; loop < 2; ++loop) {
            for (int i = 0; i <= BRANCH_SIZE; ++i) {
                setAll(0, 0, 0);
                for (int branch = 0; branch < BRANCHS; ++branch) {
                    int pixel = i + (branch * BRANCH_SIZE);
                    for (int tailSize = 1; tailSize <= 8; ++tailSize) {
                        byte blue = 0x11 * tailSize;
                        if (i > tailSize) {
                            setPixel(pixel - tailSize, 0xff, 0xff, blue);
                        }
                    }
                    setPixel(pixel, 0xff, 0xa5, 0);
                }
                showStrip();
                delay(times * 10);
            }
        }
    }
}

void fillingUp() {
    setAll(0, 0, 0);
    int stopAt = 0;
    while (stopAt <= BRANCH_SIZE) {
        for (int i = BRANCH_SIZE; i > stopAt; --i) {
            for (int branch = 0; branch < BRANCHS; ++branch) {
                int pixel = i + (branch * BRANCH_SIZE);
                setPixel(pixel, 0, 0xff, 0);
                byte tailColor;
                for (int tailSize = 1; tailSize < 15; ++tailSize) {
                    tailColor = 0x11 * tailSize;
                    byte green = 0xff - (tailSize * 0x11);
                    if (green < tailColor) {
                        green = tailColor;
                    }
                    if (i < (BRANCH_SIZE - tailSize)) {
                        setPixel(pixel + tailSize, tailColor, green, tailColor);
                    }
                }
            }
            showStrip();
            int delaying = 15;
            if (stopAt > (BRANCH_SIZE / 3)) {
                delaying = 10;
            }
            if (stopAt > (BRANCH_SIZE / 3 * 2)) {
                delaying = 5;
            }
            delay(delaying);
        }
        stopAt++;
    }

    CRGB colors[5] = {CRGB(0, 255, 0), CRGB(0, 255, 255), CRGB(0, 100, 100), CRGB( 0, 0, 0)};
    starsBlink(30, true, 20, 40, 80, 0, colors, 4);
}

byte explosionHalf(int start, int end, bool inverted, int delayMs) {
    byte tailColor;
    for (int index = start; index < end; ++index) {
        for (int branch = 0; branch < BRANCHS; ++branch) {
            int invertedIndex = (BRANCH_SIZE - 1) - index;
            int pixelDown = index + (branch * BRANCH_SIZE);
            int pixelUp = invertedIndex + (branch * BRANCH_SIZE);
            setPixel((inverted ? pixelUp : pixelDown), 0, 0xff, 0);
            setPixel((inverted ? pixelDown : pixelUp), 0xff, 0, 0);
            for (int tailSize = 1; tailSize < 8; ++tailSize) {
                tailColor = 0x22 * tailSize;
                byte red = 0xff - (tailSize * 0x22);
                byte green = 0xff - (tailSize * 0x22);
                if (green < tailColor) {
                    green = tailColor;
                    red = tailColor;
                }
                if (index > (start + tailSize - 1)) {
                    setPixel(pixelUp + tailSize, (inverted ? tailColor : red), (inverted ? green : tailColor), tailColor);
                    setPixel(pixelDown - tailSize, (inverted ? red : tailColor), (inverted ? tailColor : green), tailColor);
                }
            }
        }
        showStrip();
        delay(delayMs);
    }
    return tailColor;
}

void explosion() {
    setAll(0, 0, 0);
    int halfWay = (BRANCH_SIZE / 2);
    int fillDelay = 80;
    byte tailColor = explosionHalf(halfWay, BRANCH_SIZE, false, fillDelay);
    for (int index = BRANCH_SIZE - 8; index <= BRANCH_SIZE; ++index) {
        for (int branch = 0; branch < BRANCHS; ++branch) {
            int invertedIndex = (BRANCH_SIZE - 1) - index;
            int pixelDown = index + (branch * BRANCH_SIZE);
            int pixelUp = invertedIndex + (branch * BRANCH_SIZE);
            setPixel(pixelUp, tailColor, tailColor, tailColor);
            setPixel(pixelDown, tailColor, tailColor, tailColor);
        }
        showStrip();
        delay(fillDelay);
    }
    delay(1000);
    for (int i = 0; i < 8; ++i) {
        if (i % 2 == 0) {
            explosionHalf(0, halfWay, true, 10);
        }
        else {
            explosionHalf(halfWay, BRANCH_SIZE, false, 10);
        }
    }
    CRGB colors[5] = {CRGB(255, 255, 255), CRGB(255, 255, 0), CRGB(255, 165, 0), CRGB(255, 0, 0), CRGB( 0, 0, 0)};
    starsBlink(10, true, 10, 20, 80, 0, colors, 5);
}

void loopStarBlink() {
    int startQuantity = 1;
    for (int i = 0; i < 5; ++i) {
        if (i == 1) {
            startQuantity = 5;
        }
        if (i == 2) {
            startQuantity = 15;
        }
        if (i > 2) {
            startQuantity *= 2;
        }
        CRGB colors[6] = {CRGB(0, 255, 0), CRGB(0, 0, 255), CRGB(255, 255, 0), CRGB(255, 0, 0), CRGB( 255, 255, 255), CRGB(0, 255, 0)};
        starsBlink(startQuantity, false, 80, 10, 4, 0, colors, 6);
    }
}

void blinkOnce() {
    int lightOn = 500;
    int lightOff = 300;
    showStrip();
    delay(lightOn);
    setAll(0, 0, 0);
    showStrip();
    delay(lightOff);
}

void blinkColors(CRGB color1, CRGB color2, int usePair) {
    bool showColor1 = true;
    for (int i = 0; i < NUM_LEDS; ++i) {
        if (i % 2 == usePair) {
            if (showColor1) {
                setPixel(i, color1.r, color1.g, color1.b);
            }
            else {
                setPixel(i, color2.r, color2.g, color2.b);
            }
            showColor1 = !showColor1;
        }
    }
    blinkOnce();
}

void simpleBlink() {
    for (int i = 0; i < 5; ++i) {
        blinkColors(CRGB(255, 0, 0), CRGB(255, 255, 0), 0);
        blinkColors(CRGB(0, 0, 255), CRGB(0, 255, 0), 1);
        blinkColors(CRGB(255, 0, 255), CRGB(255, 255, 255), 0);
        blinkColors(CRGB(0, 255, 255), CRGB(100, 255, 0), 1);
    }
}

void doExplosion() {
    for (int i = 0; i < 3; ++i) {
        explosion();
    }
}

void doLoopStarBlink() {
    for (int i = 0; i < 2; ++i) {
        loopStarBlink();
    }
}

bool isNumberInArray(int number, int array[], int arraySize) {
    for (int i = 0; i < arraySize; ++i) {
        if (array[i] == number) {
            return true;
        }
    }
    return false;
}

void loop() {
    doLoopStarBlink();
//    int effects[6] = {-1, -1, -1, -1, -1, -1};
//    int currentIndex = 0;
//    while (currentIndex < 6) {
//        int number = random8(6);
//        if (!isNumberInArray(number, effects, 6)) {
//            effects[currentIndex] = number;
//            ++currentIndex;
//        }
//    }
//    for (int i = 0; i < 6; ++i) {
//        int effect = effects[i];
//        if (effect == 0) {
//            simpleBlink();
//        }
//        if (effect == 1) {
//            doLoopStarBlink();
//        }
//        if (effect == 2) {
//            doCylon();
//        }
//        if (effect == 3) {
//            doExplosion();
//        }
//        if (effect == 4) {
//            shootingStar();
//        }
//        if (effect == 5) {
//            fillingUp();
//        }
//    }
}
