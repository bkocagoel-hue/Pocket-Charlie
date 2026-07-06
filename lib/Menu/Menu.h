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

enum class Screen : std::uint8_t { Face = 0, Clock, Mood, Online, Info };

class Menu {
 public:
  void begin() { cur_ = Screen::Face; }

  void next() { cur_ = static_cast<Screen>((static_cast<int>(cur_) + 1) % kCount); }
  void prev() {
    cur_ = static_cast<Screen>((static_cast<int>(cur_) + kCount - 1) % kCount);
  }

  Screen current() const { return cur_; }

  const char* name() const {
    switch (cur_) {
      case Screen::Face:   return "Face";
      case Screen::Clock:  return "Clock";
      case Screen::Mood:   return "Mood";
      case Screen::Online: return "Online";
      case Screen::Info:   return "Info";
      default:             return "?";
    }
  }

 private:
  static constexpr int kCount = 5;
  Screen cur_ = Screen::Face;
};

}  // namespace pc
