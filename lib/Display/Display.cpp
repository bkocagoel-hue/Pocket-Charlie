#include "Display.h"

#include <M5Unified.h>

#include <cstring>

#include "PcConfig.h"

namespace pc {
namespace {
// Dunkles Grau fuer Nav-Hinweise: sichtbar, aber bewusst leiser als der Inhalt.
constexpr std::uint16_t kColorDim = 0x7BEF;
}  // namespace

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

void Display::showScreen(const char* title, const char* mainText,
                         const char* sub) {
  clear();
  const int32_t cx = M5.Display.width() / 2;
  const int32_t cy = M5.Display.height() / 2;

  // Titel klein oben, in Charlies violettem Akzent.
  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(config::kColorEye, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString(title, cx, 30);

  // dezente violette Akzentlinie
  M5.Display.fillRect(cx - 42, 60, 84, 3, config::kColorEye);

  // Hauptinfo gross, zentriert, weiss. Laengere Texte (z. B. Online-Thought)
  // automatisch kleiner, damit nichts abgeschnitten wird: Size 4 fasst ~13
  // Zeichen (320 px / 24 px), Size 2 fasst ~26 Zeichen.
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(std::strlen(mainText) > 13 ? 2 : 4);
  M5.Display.drawString(mainText, cx, cy + 4);

  // optionale Sub-Zeile darunter, klein.
  if (sub != nullptr && sub[0] != '\0') {
    M5.Display.setTextDatum(top_center);
    M5.Display.setTextSize(2);
    M5.Display.drawString(sub, cx, cy + 42);
  }
}

void Display::drawNavBar(int index, int count, const char* action) {
  const int32_t w = M5.Display.width();
  const int32_t cx = w / 2;

  // Punktreihe: ein Punkt je Screen, der aktuelle groesser + violett.
  const int32_t dotY = 205;
  const int32_t gap = 14;
  int32_t x = cx - ((count - 1) * gap) / 2;
  for (int i = 0; i < count; ++i, x += gap) {
    if (i == index) {
      M5.Display.fillCircle(x, dotY, 3, config::kColorEye);
    } else {
      M5.Display.fillCircle(x, dotY, 2, kColorDim);
    }
  }

  // "<" / ">" ueber den BtnA-/BtnC-Zonen (unteres Touch-Band, Drittel-Teilung).
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(kColorDim, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString("<", w / 6, 227);
  M5.Display.drawString(">", w - w / 6, 227);

  // Optionale BtnB-Aktion mittig (ueber der BtnB-Zone).
  if (action != nullptr && action[0] != '\0') {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(config::kColorText, config::kColorBackground);
    M5.Display.drawString(action, cx, 227);
  }
}

}  // namespace pc
