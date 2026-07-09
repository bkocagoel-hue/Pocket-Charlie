#include "Display.h"

#include <M5Unified.h>

#include <cstring>

#include "PcConfig.h"

namespace pc {
namespace {
// Dunkles Grau fuer Nav-Hinweise: sichtbar, aber bewusst leiser als der Inhalt.
constexpr std::uint16_t kColorDim = 0x7BEF;
// Luft zum Bildschirmrand, damit Text nie ganz an der Kante klebt.
constexpr std::int32_t kSideMargin = 6;

// Kuerzt "text" bei der AKTUELL gesetzten Textgroesse schrittweise und
// haengt "..." an, bis es in maxWidth passt (M5.Display.textWidth()).
// Auffangnetz fuer den Fall, dass selbst die kleinste zulaessige Groesse
// nicht reicht - kein Crash, kein Ueberlauf, nur ein kuerzerer Text.
void truncateWithEllipsis(const char* text, std::int32_t maxWidth, char* out,
                          std::size_t outSize) {
  std::size_t len = std::strlen(text);
  if (len >= outSize) len = outSize - 1;
  while (len > 0) {
    std::snprintf(out, outSize, "%.*s...", static_cast<int>(len), text);
    if (M5.Display.textWidth(out) <= maxWidth) return;
    --len;
  }
  std::snprintf(out, outSize, "...");
}

// Waehlt die groesste Textgroesse aus {4, 3, 2}, die "text" innerhalb von
// maxWidth Pixel darstellt (echte Pixelbreite statt Zeichenanzahl, da die
// Schrift proportional ist). Passt selbst Groesse 2 nicht, wird hart
// gekuerzt (truncateWithEllipsis). Schreibt das darzustellende Ergebnis
// nach out/outSize, setzt die gewaehlte Groesse direkt auf M5.Display und
// gibt sie zurueck.
std::uint8_t fitText(const char* text, std::int32_t maxWidth, char* out,
                     std::size_t outSize) {
  static constexpr std::uint8_t kSizes[] = {4, 3, 2};
  for (std::uint8_t size : kSizes) {
    M5.Display.setTextSize(size);
    if (M5.Display.textWidth(text) <= maxWidth) {
      std::snprintf(out, outSize, "%s", text);
      return size;
    }
  }
  M5.Display.setTextSize(2);
  truncateWithEllipsis(text, maxWidth, out, outSize);
  return 2;
}
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
  const int32_t maxTextWidth = M5.Display.width() - 2 * kSideMargin;

  // Titel klein oben, in Charlies violettem Akzent.
  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(config::kColorEye, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString(title, cx, 30);

  // dezente violette Akzentlinie
  M5.Display.fillRect(cx - 42, 60, 84, 3, config::kColorEye);

  // Hauptinfo gross, zentriert, weiss. Groesse richtet sich nach der
  // tatsaechlichen Pixelbreite (nicht nur Zeichenanzahl), damit z. B. lange
  // Online-Thoughts nie ueber den Rand laufen; reicht selbst Groesse 2
  // nicht, wird hart gekuerzt ("...").
  char fittedMain[32];
  const std::uint8_t mainSize =
      fitText(mainText, maxTextWidth, fittedMain, sizeof(fittedMain));
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(mainSize);
  M5.Display.drawString(fittedMain, cx, cy + 4);

  // optionale Sub-Zeile darunter, klein - gegen Ueberlauf abgesichert.
  if (sub != nullptr && sub[0] != '\0') {
    M5.Display.setTextDatum(top_center);
    M5.Display.setTextSize(2);
    char fittedSub[32];
    const char* subToShow = sub;
    if (M5.Display.textWidth(sub) > maxTextWidth) {
      truncateWithEllipsis(sub, maxTextWidth, fittedSub, sizeof(fittedSub));
      subToShow = fittedSub;
    }
    M5.Display.drawString(subToShow, cx, cy + 42);
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

void Display::drawMenuIcon(bool highlight) {
  // Drei kurze Balken (Hamburger-Icon) oben rechts - im Normalzustand
  // dezent, bei "highlight" kurz heller als sichtbares Tap-Feedback.
  const std::int32_t w = M5.Display.width();
  constexpr std::int32_t kBarW = 16;
  constexpr std::int32_t kBarH = 2;
  constexpr std::int32_t kGap  = 5;
  constexpr std::int32_t kRightMargin = 6;
  constexpr std::int32_t kTopMargin   = 8;
  const std::int32_t x = w - kRightMargin - kBarW;
  const std::uint16_t color =
      highlight ? config::kColorText : kColorDim;
  for (int i = 0; i < 3; ++i) {
    M5.Display.fillRect(x, kTopMargin + i * kGap, kBarW, kBarH, color);
  }
}

bool Display::isMenuIconZone(std::int16_t x, std::int16_t y) {
  const std::int32_t w = M5.Display.width();
  return x >= w - kMenuIconZonePx && y < kMenuIconZonePx;
}

}  // namespace pc
