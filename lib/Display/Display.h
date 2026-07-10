#pragma once
// ============================================================================
//  Display - Kapselung screen-weiter Grundfunktionen (M5.Display / M5GFX)
//
//  Verantwortung (Single Responsibility Principle):
//  - Grundeinstellungen des Panels (Rotation, Helligkeit).
//  - Screen-weite, statische Ausgaben wie der Boot-Screen.
//
//  Bewusst NICHT hier:
//  - Charlies animiertes Gesicht -> das ist Aufgabe des Face-Moduls, das mit
//    einem eigenen Off-Screen-Puffer (Canvas) flicker-frei rendert.
// ============================================================================

#include <cstdint>

namespace pc {

class Display {
 public:
  // Panel-Grundeinstellungen. Voraussetzung: M5.begin() lief bereits.
  void begin();

  // Kurzer Start-Bildschirm mit Name + Version (Sprint-1-Bootscreen).
  void showBootScreen();

  // Generischer Text-Screen (Sprint 3): Titel klein + Hauptinfo gross + Sub.
  void showScreen(const char* title, const char* mainText, const char* sub);

  // Navigations-Hinweise (Sprint 5, Sprint 7 Fix): dezente Leiste am
  // unteren Rand, direkt ueber den A/B/C-Touch-Zonen. Punktreihe = Screens
  // (aktueller violett) - reine "du bist hier"-Information, denn der
  // Wechsel zwischen Screens laeuft seit Sprint 7 ausschliesslich ueber den
  // Pocketindex (Menue-Icon), nicht mehr ueber A/C. "aHint"/"bHint"/
  // "cHint" zeigen deshalb screen-eigene Tastenfunktionen statt der
  // frueheren "<"/">"-Navigationspfeile - leerer String zeichnet bewusst
  // nichts (keine Pfeile mehr, die eine Navigation vortaeuschen wuerden).
  // Nach showScreen() aufrufen; das Face zeichnet bewusst keine Leiste.
  void drawNavBar(int index, int count, const char* aHint, const char* bHint,
                  const char* cHint);

  // Sprint 7, E1 (gefixt): dezentes Menue-Icon oben RECHTS (Touch-
  // Einstiegspunkt fuer den Pocketindex). Nur auf den Widget-Screens
  // aufrufen (Teil von showScreen()s Redraw-Zyklus) - NICHT zusaetzlich pro
  // Frame auf dem Face-Screen, das verursachte Flackern.
  // "highlight" zeigt kurzes Tap-Feedback (heller statt dezent).
  void drawMenuIcon(bool highlight = false);

  // Trefferzone oben rechts (dieselbe Breite wie beim Zeichnen - eine
  // Quelle der Wahrheit, kein Zahlendrift). Nutzt die tatsaechliche
  // Panel-Breite (M5.Display.width()), kein hardcodiertes 320.
  static bool isMenuIconZone(std::int16_t x, std::int16_t y);

  // Sprint 7, Fix (Pocketindex-Buttons): geometrische A/B/C-Touch-Zone
  // (unteres Band, siehe M5.setTouchButtonHeight() in App::setup()). Rein
  // positionsbasiert und bewusst UNABHAENGIG vom M5.BtnA/B/C-Debouncing -
  // ein generischer "Touch schliesst den Pocketindex"-Handler darf einen
  // A/B/C-Tap niemals faelschlich als Dismiss werten, auch wenn die
  // (debounced) Button-Objekte den Druck ein paar Frames spaeter melden
  // als der rohe Touch. Kein hardcodiertes 240 - nutzt M5.Display.height().
  static bool isButtonBandZone(std::int16_t y);

  // Sprint 7 (Pocketindex - Rolodex Notebook): Vollbild-Kartenindex (kein
  // Overlay/Popup, kein Alpha-Blending - ein sauberer clear() + Redraw wie
  // showScreen()). Zeigt genau EINE Karte (Titel + kurze Beschreibung) plus
  // Kopfzeile mit Position ("03/07") und eine schmale Positions-Leiste
  // rechts. "title"/"lines"/"lineCount"/"index"/"count" kommen von aussen
  // (Menu::nameAt()/cardLines()/pocketIndex()/count()) - Display bleibt
  // reine Render-Schicht und kennt die Menu/Screen-Struktur bewusst nicht
  // (Single Responsibility). "pulse" laesst den Kartentitel kurz heller
  // aufblitzen (Oeffnen/Wechsel/Auswahl-Feedback) - Teil desselben
  // Redraw-Aufrufs, kein Extra-Draw pro Frame (kein Flackern). Menue-Icon
  // zeichnet der Aufrufer separat dazu (siehe drawMenuIcon()), genau wie
  // bei den Widget-Screens.
  void showPocketindex(const char* title, const char* const* lines,
                       int lineCount, int index, int count, bool pulse);

  // Sprint 7 (Beatbox Touch-Zonen): 2x2-Raster im Content-Bereich (oberhalb
  // des Button-Bands, siehe isButtonBandZone()). Reine Geometrie - Display
  // kennt Beatbox/DrumKit bewusst nicht (Single Responsibility), Aufrufer
  // mappt den zurueckgegebenen Index 0..3 selbst auf seine Domaene.
  // Zonen-Reihenfolge: 0=oben-links, 1=oben-rechts, 2=unten-links,
  // 3=unten-rechts - dieselbe Reihenfolge wie "kitNames" in
  // showBeatboxGrid().
  static int beatboxZoneAt(std::int16_t x, std::int16_t y);

  // Zeichnet das Beatbox-Raster: vier Labels (kitNames[0..3], siehe
  // beatboxZoneAt() fuer die Zonen-Reihenfolge) plus Trennlinien.
  // "flashZone" (-1 = keiner) hebt eine Zone kurz farblich hervor
  // (Tap-Feedback) - Teil desselben Redraw-Aufrufs, kein Extra-Draw pro
  // Frame (kein Flackern), gleiches Muster wie showPocketindex()s "pulse".
  void showBeatboxGrid(const char* const kitNames[4], int flashZone);

  // Sprint 8 (Awake, Einheit 3 - Emotion Visibility v1): dezente Reaction-
  // Chip-Pill links neben der Positions-Punktreihe, auf Nicht-Home-Modi.
  // "text" kommt fertig gemappt vom Aufrufer (App: Emotion -> kurzer
  // Charlie-Voice-Text) - Display kennt Persona/Emotion bewusst nicht
  // (Single Responsibility, gleiches Prinzip wie showPocketindex()). Leerer/
  // nullptr String zeichnet bewusst nichts (Neutral -> kein Chip). Gefuellte
  // Pill (fillRoundRect + invertierte Textfarbe, Textgroesse bleibt 1) statt
  // reinem Text - mehr Kontrast, gleiche Technik wie showBeatboxGrid()s Tap-
  // Flash. Teil desselben Redraw-Aufrufs wie drawNavBar()/drawMenuIcon() -
  // kein Extra-Draw pro Frame, kein Flackern.
  void drawReactionChip(const char* text);

 private:
  void clear();

  static constexpr std::int32_t kMenuIconZonePx = 48;
};

}  // namespace pc
