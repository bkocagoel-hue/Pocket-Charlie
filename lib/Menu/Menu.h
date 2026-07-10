#pragma once
// ============================================================================
//  Menu - kleine lokale Navigations-State-Machine (Sprint 3)
//
//  Haelt nur den aktuellen Screen und den Wechsel dazwischen. Bewusst getrennt
//  von Emotion/Persona: die Emotion Engine soll NICHT zur Menue-Logik werden.
//  Header-only, keine Abhaengigkeiten - leicht testbar/erweiterbar.
//
//  Sprint 8 (Awake - Charlie Core Reboot, Einheit 1): von 11 gleichrangigen
//  Screens (App-Drawer-Gefuehl) auf fuenf Companion-Modi reduziert. Dice/
//  FocusCard/Beatbox/EightBall sowie Clock/Mood sind bewusst aus der
//  Navigation genommen (geparkt, Code bleibt in lib/ erhalten) - Clock
//  (Uptime) lebt als Unterseite in System weiter, Settings/Info ebenso.
// ============================================================================

#include <cstdint>

namespace pc {

enum class Screen : std::uint8_t { Home = 0, Think, Focus, Care, System };

class Menu {
 public:
  void begin() { cur_ = Screen::Home; }

  void next() { cur_ = static_cast<Screen>((static_cast<int>(cur_) + 1) % kCount); }
  void prev() {
    cur_ = static_cast<Screen>((static_cast<int>(cur_) + kCount - 1) % kCount);
  }

  Screen current() const { return cur_; }

  // Fuer die Navigations-Hinweise (Sprint 5): Position + Gesamtzahl.
  int index() const { return static_cast<int>(cur_); }
  static constexpr int count() { return kCount; }

  const char* name() const { return screenName(cur_); }

  // Sprint 7: Anzeige-Name je Index fuer den Pocketindex - dieselbe Quelle
  // wie name(), keine zweite Namensliste pflegen.
  static const char* nameAt(int idx) {
    return screenName(static_cast<Screen>(idx));
  }

  // Sprint 7 (Pocketindex - Rolodex Notebook): kurze Kartenbeschreibung je
  // Index, 1-3 Zeilen. Lebt bewusst hier (wie name()/nameAt()) statt in
  // Display - Display bleibt reine Render-Schicht und bekommt nur fertige
  // Strings, kennt Menu/Screen bewusst nicht (Single Responsibility).
  static const char* const* cardLines(int idx, int* outCount) {
    static const char* kHome[] = {"Charlie home -", "mood & idle", "companion."};
    static const char* kThink[] = {"Local AI bridge -", "ollama / mock,", "thought source."};
    static const char* kFocus[] = {"Focus tools -", "stopwatch, timer &", "pomodoro flow."};
    static const char* kCare[] = {"Care steht bereit -", "bald mehr hier."};
    static const char* kSystem[] = {"System -", "sound, uptime &", "version info."};
    switch (static_cast<Screen>(idx)) {
      case Screen::Home:   *outCount = 3; return kHome;
      case Screen::Think:  *outCount = 3; return kThink;
      case Screen::Focus:  *outCount = 3; return kFocus;
      case Screen::Care:   *outCount = 2; return kCare;
      case Screen::System: *outCount = 3; return kSystem;
      default:             *outCount = 0; return nullptr;
    }
  }

  // Sprint 7 (Pocketindex - Rolodex Notebook): Zustand des Karten-Index.
  // Lebt bewusst hier (nicht in Display) - Menu ist bereits die einzige
  // Quelle der Wahrheit fuer Screen-Navigation; Display bekommt nur fertige
  // Werte zum Zeichnen. Einzelkarten-Modus (eine Karte pro Screen, siehe
  // Design-Spike) braucht keine Viewport-/Scroll-Fensterlogik - Position
  // und Gesamtzahl (z. B. "03/07") reichen als Anzeige.
  bool pocketOpen() const { return pocketOpen_; }
  int pocketIndex() const { return pocketIndex_; }

  // Oeffnet mit der Karte des aktuell sichtbaren Screens markiert.
  void openPocketindex() {
    pocketIndex_ = index();
    pocketOpen_ = true;
  }
  void closePocketindex() { pocketOpen_ = false; }
  void togglePocketindex() {
    if (pocketOpen_) closePocketindex(); else openPocketindex();
  }
  void pocketPrev() {
    pocketIndex_ = (pocketIndex_ + kCount - 1) % kCount;
  }
  void pocketNext() {
    pocketIndex_ = (pocketIndex_ + 1) % kCount;
  }
  // Markierte Karte als aktiven Screen uebernehmen und Pocketindex schliessen
  // (Sprint-7-Vorgabe: eine Auswahl schliesst den Pocketindex immer).
  void selectPocketCard() {
    cur_ = static_cast<Screen>(pocketIndex_);
    closePocketindex();
  }

 private:
  static const char* screenName(Screen s) {
    switch (s) {
      case Screen::Home:   return "Home";
      case Screen::Think:  return "Think";
      case Screen::Focus:  return "Focus";
      case Screen::Care:   return "Care";
      case Screen::System: return "System";
      default:             return "?";
    }
  }

  static constexpr int kCount = 5;
  Screen cur_ = Screen::Home;
  bool pocketOpen_ = false;
  int pocketIndex_ = 0;
};

}  // namespace pc
