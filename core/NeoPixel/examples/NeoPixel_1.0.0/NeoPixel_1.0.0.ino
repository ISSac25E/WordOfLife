#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\NeoPixel\NeoPixel_1.0.0.h"

#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\LedMacro\LedMacro_1.0.0.h"
#include "C:\Users\AVG\Documents\Electrical Main (Amir)\Arduino\Projects\WordOfLife\core\LedMacros\SequenceBuild\SequenceBuild_1.0.0.h"

LedMacro macros[3];
LedMacroManager macro(macros, 3);

SequenceBuild pixel_Build_blue;
SequenceBuild pixel_Build_red;
SequenceBuild pixel_Build_green;

uint8_t pixelBuffer[6];
NeoPixelDriver pixelDriver(12, pixelBuffer, 3);
NeoPixel green(pixelDriver);
NeoPixel red(pixelDriver);
NeoPixel blue(pixelDriver);

void setup()
{
  pixel_Build_blue.setSequence(fade_blue, 2);
  pixel_Build_red.setSequence(fade_red, 1);
  pixel_Build_green.setSequence(fade_green, 1);

}

void loop()
{
  pixelDriver.run();
  pixel_Build_blue.run();
  pixel_Build_red.run();
  pixel_Build_green.run();
  macro.run();
}

SB_FUNCT(fade_blue, macro.ready(blue))
SB_STEP(macro.set(blue, 0, 50);)
SB_STEP(macro.quadEase(blue, 255, 10);)
SB_STEP(macro.quadEase(blue, 255, 20);)
// SB_STEP(macro.delay(led_2, 100);)
SB_STEP(macro.quadEase(blue, 0, 20);)
SB_STEP(macro.delay(blue, 100);)
SB_STEP(pixel_Build_blue.loop(2);)
SB_END

SB_FUNCT(fade_red, macro.ready(red))
SB_STEP(macro.set(red, 0, 50);)
SB_STEP(macro.delay(red, 30);)
SB_STEP(macro.quadEase(red, 255, 30);)
// SB_STEP(macro.delay(led_2, 100);)
SB_STEP(macro.quadEase(red, 0, 30);)
SB_STEP(macro.delay(red, 100);)
SB_STEP(pixel_Build_red.loop(2);)
SB_END

SB_FUNCT(fade_green, macro.ready(green))
SB_STEP(macro.set(green, 0, 50);)
SB_STEP(macro.delay(green, 60);)
SB_STEP(macro.quadEase(green, 255, 25);)
// SB_STEP(macro.delay(led_2, 100);)
SB_STEP(macro.quadEase(green, 0, 25);)
SB_STEP(macro.delay(green, 100);)
SB_STEP(pixel_Build_green.loop(2);)
SB_END
