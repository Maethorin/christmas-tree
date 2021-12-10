#include <FastLED.h>

#define LED_PIN 2
#define NUM_LEDS 300
#define BRANCH_SIZE 60
#define BRANCHS 5

CRGB leds[NUM_LEDS];

void setup() {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
    Serial.begin( 9600 );
    random16_set_seed(millis());
}

void showStrip() {
    FastLED.show();
}

void setPixel(int pixel, byte red, byte green, byte blue, int darkness) {
    leds[pixel].r = red;
    leds[pixel].g = green;
    leds[pixel].b = blue;
    if (darkness > 250) {
        darkness = 250;
    }
    if (darkness > 0) {
        leds[pixel].fadeToBlackBy(darkness);
    }
}

void setAll(byte red, byte green, byte blue) {
    for (int i = 0; i < NUM_LEDS; i++) {
        setPixel(i, red, green, blue, 0);
    }
    showStrip();
}

void setEyeColors(int eyeIndex, int eyeSize, byte red, byte green, byte blue) {
    setPixel(eyeIndex, red / 10, green / 10, blue / 10, 0);
    for (int j = 1; j <= eyeSize; j++) {
        setPixel(eyeIndex + j, red, green, blue, 0);
    }
    setPixel(eyeIndex + eyeSize + 1, red / 10, green / 10, blue / 10, 0);
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
            setPixel(index, starsColors.r, starsColors.g, starsColors.b, 0);
        }
        showStrip();
        delay(blinkTime);
    }
}

byte flyAway(int start, bool inverted, byte tailColor, int index, int branch) {
    int invertedIndex = (BRANCH_SIZE - 1) - index;
    int pixelDown = index + (branch * BRANCH_SIZE);
    int pixelUp = invertedIndex + (branch * BRANCH_SIZE);
    setPixel((inverted ? pixelUp : pixelDown), 0, 0xff, 0, 0);
    setPixel((inverted ? pixelDown : pixelUp), 0xff, 0, 0, 0);
    for (int tailSize = 1; tailSize < 8; ++tailSize) {
        tailColor = 0x22 * tailSize;
        byte red = 0xff - (tailSize * 0x22);
        byte green = 0xff - (tailSize * 0x22);
        if (green < tailColor) {
            green = tailColor;
            red = tailColor;
        }
        if (index > (start + tailSize - 1)) {
            setPixel(pixelUp + tailSize, (inverted ? tailColor : red), (inverted ? green : tailColor), tailColor, 0);
            setPixel(pixelDown - tailSize, (inverted ? red : tailColor), (inverted ? tailColor : green), tailColor, 0);
        }
    }
    return tailColor;
}

byte explosionHalf(int start, int end, bool inverted, int delayMs) {
    byte tailColor;
    for (int index = start; index < end; ++index) {
        for (int branch = 0; branch < BRANCHS; ++branch) {
            tailColor = flyAway(start, inverted, tailColor, index, branch);
        }
        showStrip();
        delay(delayMs);
    }
    return tailColor;
}

void explosion() {
    setAll(0, 0, 0);
    int halfWay = (BRANCH_SIZE / 2);
    int fillDelay = 120;
    byte tailColor = explosionHalf(halfWay, BRANCH_SIZE, false, fillDelay);
    for (int index = BRANCH_SIZE - 8; index <= BRANCH_SIZE; ++index) {
        for (int branch = 0; branch < BRANCHS; ++branch) {
            int invertedIndex = (BRANCH_SIZE - 1) - index;
            int pixelDown = index + (branch * BRANCH_SIZE);
            int pixelUp = invertedIndex + (branch * BRANCH_SIZE);
            setPixel(pixelUp, tailColor, tailColor, tailColor, 0);
            setPixel(pixelDown, tailColor, tailColor, tailColor, 0);
        }
        showStrip();
        delay(fillDelay);
    }
    delay(1000);
    for (int i = 0; i < 12; ++i) {
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
                setPixel(i, color1.r, color1.g, color1.b, 0);
            }
            else {
                setPixel(i, color2.r, color2.g, color2.b, 0);
            }
            showColor1 = !showColor1;
        }
    }
    blinkOnce();
}

void simpleBlink(CRGB color1, CRGB color2, CRGB color3, CRGB color4) {
    for (int i = 0; i < 5; ++i) {
        blinkColors(color1, color2, 0);
        blinkColors(color3, color4, 1);
        blinkColors(color3, color1, 0);
        blinkColors(color4, color2, 1);
        blinkColors(color1, color4, 0);
        blinkColors(color3, color2, 1);
    }
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

void doExplosion() {
    for (int i = 0; i < 2; ++i) {
        explosion();
    }
}

void doShootingStar() {
    for (int times = 4; times > 0; --times) {
        for (int loop = 0; loop < 2; ++loop) {
            for (int i = 0; i <= BRANCH_SIZE; ++i) {
                setAll(0, 0, 0);
                for (int branch = 0; branch < BRANCHS; ++branch) {
                    int pixel = i + (branch * BRANCH_SIZE);
                    for (int tailSize = 1; tailSize <= 8; ++tailSize) {
                        byte blue = 0x11 * tailSize;
                        if (i > tailSize) {
                            setPixel(pixel - tailSize, 0xff, 0xff, blue, 0);
                        }
                    }
                    setPixel(pixel, 0xff, 0xa5, 0, 0);
                }
                showStrip();
                delay(times * 10);
            }
        }
    }
}

void doLoopStarBlink() {
    for (int i = 0; i < 1; ++i) {
        loopStarBlink();
    }
}

void doFillingUp() {
    setAll(0, 0, 0);
    int stopAt = 0;
    while (stopAt <= BRANCH_SIZE) {
        for (int i = BRANCH_SIZE; i > stopAt; --i) {
            for (int branch = 0; branch < BRANCHS; ++branch) {
                int pixel = i + (branch * BRANCH_SIZE);
                setPixel(pixel, 0, 0xff, 0, 0);
                byte tailColor;
                for (int tailSize = 1; tailSize < 15; ++tailSize) {
                    tailColor = 0x11 * tailSize;
                    byte green = 0xff - (tailSize * 0x11);
                    if (green < tailColor) {
                        green = tailColor;
                    }
                    if (i < (BRANCH_SIZE - tailSize)) {
                        setPixel(pixel + tailSize, tailColor, green, tailColor, 0);
                    }
                }
            }
            showStrip();
            int delaying = 10;
            if (stopAt > (BRANCH_SIZE / 3)) {
                delaying = 5;
            }
            if (stopAt > (BRANCH_SIZE / 3 * 2)) {
                delaying = 1;
            }
            delay(delaying);
        }
        stopAt++;
    }

    CRGB colors[5] = {CRGB(0, 255, 0), CRGB(0, 255, 255), CRGB(0, 100, 100), CRGB( 0, 0, 0)};
    starsBlink(30, true, 20, 40, 80, 0, colors, 5);
}

void fadeOut(int steps, uint8_t progress) {
    for (int step = 0; step < steps; ++step) {
        for (int i = 0; i < NUM_LEDS; ++i) {
            leds[i].fadeToBlackBy(progress);
        }
        showStrip();
    }
}

void impact(int blastRadius, int tailTotal, CRGB centralColor, CRGB mainColor1, CRGB mainColor2, int delayFactor) {
    random16_add_entropy(analogRead(A0));
    setAll(0, 0, 0);
    int branchIndex = random16(BRANCHS);
    int start = random16(BRANCH_SIZE);
    int end = start + blastRadius;
    int branchStart = branchIndex * BRANCH_SIZE;
    int relativeStart = start + branchStart;
    int branchEnd = branchStart + BRANCH_SIZE;
    int pixelDown = relativeStart;
    int pixelUp = relativeStart;
    CRGB color1 = mainColor1;
    CRGB color2 = mainColor2;
    for (int index = start; index < end; ++index) {
        if (pixelDown <= branchEnd) {
            setPixel(pixelDown, mainColor1.red, mainColor1.green, mainColor1.blue, 0);
            if (index > (start + tailTotal)) {
                setPixel(pixelDown - tailTotal, 0, 0, 0, 0);
            }
        }
        if (pixelUp >= branchStart) {
            setPixel(pixelUp, mainColor2.red, mainColor2.green, mainColor2.blue, 0);
            if (index > (start + tailTotal)) {
                setPixel(pixelUp + tailTotal, 0, 0, 0, 0);
            }
        }
        int darknessFactor = 45;
        for (int tailSize = 1; tailSize < tailTotal; ++tailSize) {
            color1.addToRGB(0x01);
            color2.addToRGB(0x01);
            if (index > (start + tailSize - 1)) {
                if (pixelDown - tailSize <= branchEnd) {
                    setPixel(pixelDown - tailSize, color1.red, color1.green, color1.blue, tailSize * darknessFactor);
                }
                if (pixelUp + tailSize >= branchStart) {
                    setPixel(pixelUp + tailSize, color2.red , color2.green, color2.blue, tailSize * darknessFactor);
                }
            }
        }
        setPixel(relativeStart, centralColor.red, centralColor.green, centralColor.blue, 0);
        showStrip();
        delay((index - start) * delayFactor);
        pixelDown++;
        pixelUp--;
    }
    fadeOut(50, 20);
}

void impacts(int blastRadius, int tailTotal, CRGB centralColor, CRGB mainColor1, CRGB mainColor2, int delayFactor, bool fadingOut) {
    random16_add_entropy(analogRead(A0));
    setAll(0, 0, 0);
    CRGB color1 = mainColor1;
    CRGB color2 = mainColor2;
    int branchIndexes[BRANCHS] = {2, 0, 3, 1};
    int starts[BRANCHS];
    int ends[BRANCHS];
    int branchStarts[BRANCHS];
    int relativeStarts[BRANCHS];
    int branchEnds[BRANCHS];
    int pixelDowns[BRANCHS];
    int pixelUps[BRANCHS];
    for (int i = 0; i < BRANCHS; ++i) {
        starts[i] = random16(BRANCH_SIZE);
        ends[i] = starts[i] + blastRadius;
        branchStarts[i] = branchIndexes[i] * BRANCH_SIZE;
        relativeStarts[i] = starts[i] + branchStarts[i];
        branchEnds[i] = branchStarts[i] + BRANCH_SIZE;
        pixelDowns[i] = relativeStarts[i];
        pixelUps[i] = relativeStarts[i];
    }

    for (int step = 0; step < blastRadius; ++step) {
        for (int i = 0; i < BRANCHS; ++i) {
            int index = step + starts[i];
            if (pixelDowns[i] <= branchEnds[i]) {
                setPixel(pixelDowns[i], mainColor1.red, mainColor1.green, mainColor1.blue, 0);
                if (index > (starts[i] + tailTotal)) {
                    setPixel(pixelDowns[i] - tailTotal, 0, 0, 0, 0);
                }
            }
            if (pixelUps[i] >= branchStarts[i]) {
                setPixel(pixelUps[i], mainColor2.red, mainColor2.green, mainColor2.blue, 0);
                if (index > (starts[i] + tailTotal)) {
                    setPixel(pixelUps[i] + tailTotal, 0, 0, 0, 0);
                }
            }
            int darknessFactor = 45;
            for (int tailSize = 1; tailSize < tailTotal; ++tailSize) {
                color1.addToRGB(0x01);
                color2.addToRGB(0x01);
                if (index > (starts[i] + tailSize - 1)) {
                    if (pixelDowns[i] - tailSize <= branchEnds[i]) {
                        setPixel(pixelDowns[i] - tailSize, color1.red, color1.green, color1.blue, tailSize * darknessFactor);
                    }
                    if (pixelUps[i] + tailSize >= branchStarts[i]) {
                        setPixel(pixelUps[i] + tailSize, color2.red , color2.green, color2.blue, tailSize * darknessFactor);
                    }
                }
            }
            setPixel(relativeStarts[i], centralColor.red, centralColor.green, centralColor.blue, 0);
            pixelDowns[i]++;
            pixelUps[i]--;
        }
        showStrip();
        delay((step) * delayFactor);
    }
    if (fadingOut) {
        fadeOut(80, 20);
    }
}

void doImpact() {
    CRGB colors[7] = {CRGB(0xff, 0xff, 0xff), CRGB(0xff, 0, 0), CRGB(0xff, 0xff, 0), CRGB(0xff, 0xa5, 0), CRGB(0, 0xff, 0), CRGB(0, 0, 0xff), CRGB(0, 0, 0)};
    for (int i = 1; i < 15; ++i) {
        int colorIndex = random(1, 6);
        impact(15, 8, CRGB(0xff, 0xff, 0), colors[colorIndex], colors[colorIndex], 8);
    }
    for (int i = 0; i < 10; ++i) {
        int colorIndex = random(1, 6);
        if (i < 9) {
            impacts(10, 6, CRGB(0xff, 0xff, 0), colors[colorIndex], colors[colorIndex], 12, true);
        }
        else {
            impacts(30, 10, CRGB(0xff, 0xff, 0), colors[colorIndex], colors[colorIndex], 2, false);
        }
    }

    starsBlink(30, true, 5, 50, 10, 0, colors, 7);
}

void train(CRGB color1, CRGB color2, int speed, bool reverse) {
    CRGB currentColor = color1;
    int moder = 6;
    for (int i = 0; i < NUM_LEDS; ++i) {
        if (i % moder == 0) {
            currentColor = currentColor == color1 ? color2 : color1;
            moder = moder == 6 ? 2 : 6;
        }
        setPixel(i, currentColor.red, currentColor.green, currentColor.blue, 0);
    }
    showStrip();
    delay(speed);
    for (int zas = 0; zas < 100; ++zas) {
        if (reverse) {
            int nextIndex = 0;
            for (int i = 0; i < NUM_LEDS; ++i) {
                nextIndex++;
                if (nextIndex == NUM_LEDS - 1) {
                    nextIndex = 0;
                }
                leds[i] = leds[nextIndex];
            }
        }
        else {
            int nextIndex = NUM_LEDS - 1;
            for (int i = NUM_LEDS - 1; i >= 0; --i) {
                nextIndex--;
                if (nextIndex < 0) {
                    nextIndex = NUM_LEDS - 1;
                }
                leds[i] = leds[nextIndex];
            }
        }
        showStrip();
        delay(speed);
    }
}

void doTrain() {
    int colorsSize = 4;
    CRGB colors[4][4] = {
        {CRGB(0xff, 0, 0), CRGB(0, 0x33, 0), CRGB(0, 0xff, 0xff), CRGB(0xaa, 0x21, 0)},
        {CRGB(0, 0, 0xff), CRGB(0, 0x33, 0), CRGB(0xff, 0, 0), CRGB(0, 0x33, 0)},
        {CRGB(0, 0xff, 0xff), CRGB(0xaa, 0x21, 0), CRGB(0, 0, 0xff), CRGB(0, 0x33, 0)},
        {CRGB(0, 0xff, 0), CRGB(0x22, 0, 0), CRGB(0, 0x22, 0xff), CRGB(0xaa, 0, 0x34)}
    };
    for (int i = 0; i < colorsSize; ++i) {
        train(colors[i][0], colors[i][1], 65, false);
        train(colors[i][2], colors[i][3], 65, true);
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
    int effects[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    int currentIndex = 0;
    int effectsQtd = 8;
    while (currentIndex < effectsQtd) {
        int number = random8(effectsQtd);
        if (!isNumberInArray(number, effects, effectsQtd)) {
            effects[currentIndex] = number;
            ++currentIndex;
        }
    }
    for (int i = 0; i < effectsQtd; ++i) {
        int effect = effects[i];
        if (effect == 0) {
            simpleBlink(CRGB(255, 0, 0), CRGB(255, 255, 0), CRGB(0, 0, 255), CRGB(0, 255, 0));
        }
        if (effect == 1) {
            doLoopStarBlink();
        }
        if (effect == 2) {
            doCylon();
        }
        if (effect == 3) {
            doExplosion();
        }
        if (effect == 4) {
            doShootingStar();
        }
        if (effect == 5) {
            doFillingUp();
        }
        if (effect == 6) {
            doImpact();
        }
        if (effect == 7) {
            doTrain();
        }
    }
}
