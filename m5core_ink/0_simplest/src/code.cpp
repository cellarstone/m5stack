#include <M5CoreInk.h>

Ink_Sprite InkPageSprite(&M5.M5Ink);

void setup() {
    M5.begin();
    if (!M5.M5Ink.isInit()) {
        Serial.println("Ink Init Failed");
        while (1) delay(100);
    }
    
    M5.M5Ink.clear();
    delay(1000);
    
    InkPageSprite.creatSprite(0, 0, 200, 200);
    InkPageSprite.drawString(10, 50, "Hello M5CoreInk!", &AsciiFont8x16);
    InkPageSprite.pushSprite();
}

void loop() {
    delay(1000);
}