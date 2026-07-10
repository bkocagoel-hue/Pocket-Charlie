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
// Sprint 8, Einheit 3: gleiche Zeile wie die Positions-Punktreihe in
// drawNavBar() - eine Quelle der Wahrheit, damit Chip und Dots nie
// auseinanderdriften.
constexpr std::int32_t kDotY = 205;

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

void Display::drawNavBar(int index, int count, const char* aHint,
                         const char* bHint, const char* cHint) {
  const int32_t w = M5.Display.width();
  const int32_t cx = w / 2;

  // Punktreihe: ein Punkt je Screen, der aktuelle groesser + violett - reine
  // Positionsanzeige (Screenwechsel laeuft ueber den Pocketindex).
  const int32_t gap = 14;
  int32_t x = cx - ((count - 1) * gap) / 2;
  for (int i = 0; i < count; ++i, x += gap) {
    if (i == index) {
      M5.Display.fillCircle(x, kDotY, 3, config::kColorEye);
    } else {
      M5.Display.fillCircle(x, kDotY, 2, kColorDim);
    }
  }

  // A/B/C-Hinweise ueber den jeweiligen Touch-Zonen - screen-eigene
  // Funktionen statt der frueheren "<"/">"-Navigationspfeile. Leere
  // Strings zeichnen bewusst nichts (kein Platzhalter, der eine Aktion
  // vortaeuscht, wo keine ist).
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(1);
  if (aHint != nullptr && aHint[0] != '\0') {
    M5.Display.setTextColor(kColorDim, config::kColorBackground);
    M5.Display.drawString(aHint, w / 6, 227);
  }
  if (cHint != nullptr && cHint[0] != '\0') {
    M5.Display.setTextColor(kColorDim, config::kColorBackground);
    M5.Display.drawString(cHint, w - w / 6, 227);
  }
  if (bHint != nullptr && bHint[0] != '\0') {
    M5.Display.setTextColor(config::kColorText, config::kColorBackground);
    M5.Display.drawString(bHint, cx, 227);
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

bool Display::isButtonBandZone(std::int16_t y) {
  const std::int32_t h = M5.Display.height();
  return y >= h - static_cast<std::int32_t>(config::kTouchButtonHeight);
}

void Display::showPocketindex(const char* title, const char* const* lines,
                              int lineCount, int index, int count,
                              bool pulse) {
  clear();
  const std::int32_t w = M5.Display.width();
  const std::int32_t cx = w / 2;

  // Kopfzeile: "POCKETINDEX" links, Position ("03/07") rechts - endet vor
  // der Menue-Icon-Zone, die der Aufrufer separat dazu zeichnet (siehe
  // drawMenuIcon()), genau wie bei den Widget-Screens.
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString("POCKETINDEX", 8, 8);

  char posBuf[8];
  std::snprintf(posBuf, sizeof(posBuf), "%02d/%02d", index + 1, count);
  M5.Display.setTextDatum(top_right);
  M5.Display.setTextColor(kColorDim, config::kColorBackground);
  M5.Display.setTextSize(1);
  M5.Display.drawString(posBuf, w - kMenuIconZonePx - 6, 11);

  // Trennlinie unter dem Header.
  constexpr std::int32_t kHeaderLineY = 26;
  M5.Display.fillRect(6, kHeaderLineY, w - 12, 1, kColorDim);

  // Kartentitel in Klammern, zentriert (Rolodex-Karte). "pulse" laesst ihn
  // fuer einen Redraw heller aufblitzen (Oeffnen-/Wechsel-/Auswahl-
  // Feedback) - Teil desselben Redraw-Aufrufs, kein Extra-Draw pro Frame.
  char titleBuf[24];
  std::snprintf(titleBuf, sizeof(titleBuf), "[ %s ]", title);
  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(pulse ? config::kColorText : config::kColorEye,
                          config::kColorBackground);
  M5.Display.setTextSize(2);
  M5.Display.drawString(titleBuf, cx, 56);

  // Kartenbeschreibung: 1-3 kurze Zeilen, zentriert, gegen Ueberlauf
  // abgesichert (wie schon in showScreen()) - in der Praxis greift das bei
  // den kurzen Karten-Texten nie, bleibt aber ein Auffangnetz.
  constexpr std::int32_t kLineTop = 96;
  constexpr std::int32_t kLineH   = 20;
  const std::int32_t maxLineWidth = w - 2 * kSideMargin - 20;
  M5.Display.setTextDatum(top_center);
  M5.Display.setTextColor(config::kColorText, config::kColorBackground);
  M5.Display.setTextSize(1);
  char fitted[32];
  for (int i = 0; i < lineCount; ++i) {
    const char* lineText = lines[i];
    if (M5.Display.textWidth(lineText) > maxLineWidth) {
      truncateWithEllipsis(lineText, maxLineWidth, fitted, sizeof(fitted));
      lineText = fitted;
    }
    M5.Display.drawString(lineText, cx, kLineTop + i * kLineH);
  }

  // Trennlinie ueber dem Footer + Bedien-Hinweise (gleiche Rolle wie in
  // drawNavBar()) - eigene Hinweise, da der Pocketindex A/B/C waehrend er
  // offen ist exklusiv fuer sich beansprucht (siehe App::handleButtons()).
  constexpr std::int32_t kFooterLineY = 200;
  M5.Display.fillRect(6, kFooterLineY, w - 12, 1, kColorDim);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(kColorDim, config::kColorBackground);
  M5.Display.setTextSize(1);
  M5.Display.drawString("A prev      B open      C next", cx,
                        kFooterLineY + 12);

  // Schmale Positions-Leiste rechts: ein Tick je Karte, aktuelle in
  // Akzentfarbe. Ersetzt im Einzelkarten-Modus die Scrollbar - kein
  // Viewport-Fenster noetig (siehe Menu::pocketIndex()/count()).
  const std::int32_t tickX = w - 6;
  const std::int32_t tickTop = kHeaderLineY + 8;
  const std::int32_t tickBottom = kFooterLineY - 8;
  const std::int32_t tickSpan = tickBottom - tickTop;
  for (int i = 0; i < count; ++i) {
    const std::int32_t y =
        (count > 1) ? (tickTop + (tickSpan * i) / (count - 1)) : tickTop;
    if (i == index) {
      M5.Display.fillCircle(tickX, y, 2, config::kColorEye);
    } else {
      M5.Display.fillCircle(tickX, y, 1, kColorDim);
    }
  }
}

int Display::beatboxZoneAt(std::int16_t x, std::int16_t y) {
  const std::int32_t w = M5.Display.width();
  const std::int32_t contentH =
      M5.Display.height() - static_cast<std::int32_t>(config::kTouchButtonHeight);
  const int col = (x < w / 2) ? 0 : 1;
  const int row = (y < contentH / 2) ? 0 : 1;
  return row * 2 + col;
}

void Display::showBeatboxGrid(const char* const kitNames[4], int flashZone) {
  clear();
  const std::int32_t w = M5.Display.width();
  const std::int32_t contentH =
      M5.Display.height() - static_cast<std::int32_t>(config::kTouchButtonHeight);
  const std::int32_t colW = w / 2;
  const std::int32_t rowH = contentH / 2;

  // Trennlinien zwischen den vier Zonen.
  M5.Display.fillRect(colW - 1, 0, 2, contentH, kColorDim);
  M5.Display.fillRect(0, rowH - 1, w, 2, kColorDim);

  M5.Display.setTextDatum(middle_center);
  for (int zone = 0; zone < 4; ++zone) {
    const int col = zone % 2;
    const int row = zone / 2;
    const std::int32_t zx = col * colW;
    const std::int32_t zy = row * rowH;

    if (zone == flashZone) {
      // Kurzer Tap-Flash: Zone fuellen, Text in Hintergrundfarbe darauf.
      M5.Display.fillRect(zx + 2, zy + 2, colW - 4, rowH - 4, config::kColorEye);
      M5.Display.setTextColor(config::kColorBackground, config::kColorEye);
    } else {
      M5.Display.setTextColor(config::kColorText, config::kColorBackground);
    }

    M5.Display.setTextSize(2);
    char fitted[16];
    const std::int32_t maxW = colW - 16;
    const char* label = kitNames[zone];
    if (M5.Display.textWidth(label) > maxW) {
      truncateWithEllipsis(label, maxW, fitted, sizeof(fitted));
      label = fitted;
    }
    M5.Display.drawString(label, zx + colW / 2, zy + rowH / 2);
  }
}

void Display::drawReactionChip(const char* text) {
  if (text == nullptr || text[0] == '\0') return;  // Neutral -> kein Chip.
  // Links neben der Punktreihe (dieselbe Zeile, kDotY) - auf allen Nicht-
  // Home-Modi reichlich freier Platz, kollidiert nicht mit den mittig
  // zentrierten Dots oder den A/B/C-Hints darunter (siehe drawNavBar()).
  //
  // Sprint 8, Einheit 3 (Nachschaerfen - Variante A "gefuellte Pill"):
  // dieselbe Technik wie schon bei showBeatboxGrid()s Tap-Flash und
  // showPocketindex()s Puls (fillRoundRect + invertierte Textfarbe) - kein
  // neuer visueller Trick, nur an neuer Stelle wiederverwendet. Textgroesse
  // bleibt bewusst 1 (siehe kPillH-Bemessung); die Pill-Faerbung liefert den
  // Kontrast, keine groessere Schrift noetig - haelt genug Abstand zu den
  // Dots auch bei laengeren Chip-Texten.
  M5.Display.setTextSize(1);
  const std::int32_t textW = M5.Display.textWidth(text);

  constexpr std::int32_t kPadX  = 6;
  constexpr std::int32_t kPillH = 14;
  const std::int32_t pillW = textW + 2 * kPadX;
  const std::int32_t pillX = kSideMargin;
  const std::int32_t pillY = kDotY - kPillH / 2;

  M5.Display.fillRoundRect(pillX, pillY, pillW, kPillH, kPillH / 2,
                          config::kColorEye);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextColor(config::kColorBackground, config::kColorEye);
  M5.Display.drawString(text, pillX + pillW / 2, kDotY);
}

}  // namespace pc
