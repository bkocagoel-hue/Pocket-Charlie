#pragma once
// ============================================================================
//  Menu - kleine lokale Navigations-State-Machine (Sprint 3)
//
//  Haelt nur den aktuellen Screen und den Wechsel dazwischen. Bewusst getrennt
//  von Emotion/Persona: die Emotion Engine soll NICHT zur Menue-Logik werden.
//  Header-only, keine Abhaengigkeiten - leicht testbar/erweiterbar.
// ============================================================================

#include <cstdint>

namespace pc {

enum class Screen : std::uint8_t {
  Face = 0, Clock, Mood, Online, Productivity, Settings, Info, Dice, FocusCard
};

class Menu {
 public:
  void begin() { cur_ = Screen::Face; }

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
    static const char* kFace[] = {"Charlie home -", "mood & idle", "companion."};
    static const char* kClock[] = {"Uptime -", "how long Charlie's", "been awake."};
    static const char* kMood[] = {"Charlie's mood -", "light & current", "state."};
    static const char* kOnline[] = {"Local AI bridge -", "ollama / mock,", "thought source."};
    static const char* kProductivity[] = {"Focus tools -", "stopwatch, timer &", "pomodoro flow."};
    static const char* kSettings[] = {"Sound &", "preferences."};
    static const char* kInfo[] = {"Version &", "system info."};
    static const char* kDice[] = {"Quick decisions -", "d6, d20 or a", "coin flip."};
    static const char* kFocusCard[] = {"A quick prompt -", "focus, break or", "reset, your call."};
    switch (static_cast<Screen>(idx)) {
      case Screen::Face:         *outCount = 3; return kFace;
      case Screen::Clock:        *outCount = 3; return kClock;
      case Screen::Mood:         *outCount = 3; return kMood;
      case Screen::Online:       *outCount = 3; return kOnline;
      case Screen::Productivity: *outCount = 3; return kProductivity;
      case Screen::Settings:     *outCount = 2; return kSettings;
      case Screen::Info:         *outCount = 2; return kInfo;
      case Screen::Dice:         *outCount = 3; return kDice;
      case Screen::FocusCard:    *outCount = 3; return kFocusCard;
      default:                   *outCount = 0; return nullptr;
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
      case Screen::Face:         return "Face";
      case Screen::Clock:        return "Clock";
      case Screen::Mood:         return "Mood";
      case Screen::Online:       return "Online";
      case Screen::Productivity: return "Productivity";
      case Screen::Settings:     return "Settings";
      case Screen::Info:         return "Info";
      case Screen::Dice:         return "Dice";
      case Screen::FocusCard:    return "Focus Card";
      default:                   return "?";
    }
  }

  static constexpr int kCount = 9;
  Screen cur_ = Screen::Face;
  bool pocketOpen_ = false;
  int pocketIndex_ = 0;
};

}  // namespace pc
