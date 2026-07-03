#include "Display.h"

#include <M5Unified.h>

#include "PcConfig.h"

namespace pc {

void Display::begin() {
  // Rotation 1 = Landscape (320x240), USB-Buchse rechts.
  M5.Display.setRotation(1);
  M5.Display.setBrightness(config::kBrightness);
  M5.Display.setTextWrap(false);
  clear();
}

void Display::clear() {
  M5.Display.fillScreen(config::kColorBackground);
}

void Display::showBootScreen() {
  clear();

  const int32_t cx = M5.Display.width() / 2;
  const int32_t cy = M5.Display.height() / 2;

  // Projektname, zentriert, in Charlies Akzentfarbe.
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(config::kColorAccent, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString(config::kAppName, cx, cy - 16);

  // Bootnachricht, dezent darunter.
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(1);
  M5.Display.drawString(config::kBootHint, cx, cy + 14);

  // Version als kleine Fusszeile.
  M5.Display.setTextDatum(bottom_right);
  M5.Display.drawString(config::kAppVersion, M5.Display.width() - 6,
                        M5.Display.height() - 6);
}

}  // namespace pc
