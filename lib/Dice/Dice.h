#pragma once
// ============================================================================
//  Dice - kleines Zufalls-Werkzeug (Sprint 7, Nougat: erste Pocketindex-
//  Mini-App)
//
//  Drei Modi: d6, d20, Muenze. Kein Zeitverhalten noetig (anders als
//  Productivity) - nur "welcher Modus" + "letztes Ergebnis". A/C wechseln
//  den Modus (nicht destruktiv, kein Reroll), B wuerfelt/wirft.
//  local-first, kein Netz-/Bridge-Bezug.
// ============================================================================

#include <cstddef>
#include <cstdint>

namespace pc {

enum class DiceMode : std::uint8_t { D6 = 0, D20, Coin };

class Dice {
 public:
  void begin();

  void cyclePrev();  // BtnA: vorheriger Modus (kein Reroll)
  void cycleNext();  // BtnC: naechster Modus (kein Reroll)
  void roll();       // BtnB: neues Ergebnis fuer den aktuellen Modus

  DiceMode mode() const { return mode_; }
  const char* modeName() const;  // "d6" / "d20" / "coin" (Screen-Titel)
  bool hasRolled() const { return hasRolled_; }

  // "4" / "17" / "heads" / "tails" - noch kein Ergebnis: leerer String.
  void resultText(char* out, std::size_t n) const;

 private:
  DiceMode mode_ = DiceMode::D6;
  int lastValue_ = 0;      // Wuerfel: 1..6/1..20; Muenze: 0=heads, 1=tails
  bool hasRolled_ = false;
};

}  // namespace pc
