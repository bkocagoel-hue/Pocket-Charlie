#pragma once
// ============================================================================
//  FocusCard - kleine Prompt-Karten (Sprint 7, Nougat: zweite Pocketindex-
//  Mini-App)
//
//  Drei Kategorien: Focus/Break/Reset. Kein Zeitverhalten noetig (wie Dice) -
//  nur "welche Kategorie" + "welche Karte zuletzt gezogen". A/C wechseln die
//  Kategorie (nicht destruktiv, keine neue Karte), B zieht eine Karte.
//  local-first, kein Netz-/Bridge-Bezug. Eigene, kurze Prompt-Texte -
//  bewusst getrennt von Phrases.h (das ist Charlies Sprechblasen-Microcopy,
//  ein anderer Ton/Zweck als lesbare Karten-Prompts).
// ============================================================================

#include <cstdint>

namespace pc {

enum class CardCategory : std::uint8_t { Focus = 0, Break, Reset };

class FocusCard {
 public:
  void begin();

  void cyclePrev();  // BtnA: vorherige Kategorie (keine neue Karte)
  void cycleNext();  // BtnC: naechste Kategorie (keine neue Karte)
  void draw();       // BtnB: neue zufaellige Karte aus der aktuellen Kategorie

  CardCategory category() const { return category_; }
  const char* categoryName() const;  // "focus" / "break" / "reset" (Screen-Titel)
  bool hasDrawn() const { return hasDrawn_; }
  const char* cardText() const;  // aktueller Prompt, oder "" wenn noch keiner gezogen

 private:
  CardCategory category_ = CardCategory::Focus;
  int lastIndex_ = 0;
  bool hasDrawn_ = false;
};

}  // namespace pc
