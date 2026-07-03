#include "Display.h"

#include <M5Unified.h>

#include "PcConfig.h"

namespace pc {

void Display::begin() {
  // Rotation 1 = Landscape (320x240), USB-Buchse rechts -> typische
  // Tisch-Ausrichtung fuer einen Desktop-Begleiter.
  M5.Display.setRotation(1);
  M5.Display.setTextWrap(false);
  clear();
}

void Display::clear() {
  M5.Display.fillScreen(config::kColorBackground);
}

void Display::showGreeting() {
  clear();

  const int32_t centerX = M5.Display.width() / 2;
  const int32_t centerY = M5.Display.height() / 2;

  // Haupt-Begruessung: zentriert, mittelgross, in Charlies Akzentfarbe.
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(config::kColorAccent, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString(config::kGreeting, centerX, centerY - 12);

  // Untertitel: kleiner und dezent darunter.
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(1);
  M5.Display.drawString(config::kSubtitle, centerX, centerY + 16);

  // Versions-Info als kleine Fusszeile unten rechts.
  M5.Display.setTextDatum(bottom_right);
  M5.Display.drawString(config::kAppVersion, M5.Display.width() - 6,
                        M5.Display.height() - 6);
}

}  // namespace pc
