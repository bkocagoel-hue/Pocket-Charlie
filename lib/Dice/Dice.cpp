#include "Dice.h"

#include <Arduino.h>
#include <cstdio>

namespace pc {

void Dice::begin() {
  mode_ = DiceMode::D6;
  lastValue_ = 0;
  hasRolled_ = false;
}

void Dice::cyclePrev() {
  mode_ = static_cast<DiceMode>((static_cast<int>(mode_) + 2) % 3);  // -1 mod 3
  hasRolled_ = false;  // neuer Modus zeigt kein altes Ergebnis mehr an
}

void Dice::cycleNext() {
  mode_ = static_cast<DiceMode>((static_cast<int>(mode_) + 1) % 3);
  hasRolled_ = false;
}

void Dice::roll() {
  switch (mode_) {
    case DiceMode::D6:   lastValue_ = random(1, 7);  break;   // 1..6
    case DiceMode::D20:  lastValue_ = random(1, 21); break;   // 1..20
    case DiceMode::Coin: lastValue_ = random(0, 2);  break;   // 0/1
  }
  hasRolled_ = true;
}

const char* Dice::modeName() const {
  switch (mode_) {
    case DiceMode::D6:   return "d6";
    case DiceMode::D20:  return "d20";
    case DiceMode::Coin: return "coin";
    default:             return "?";
  }
}

void Dice::resultText(char* out, std::size_t n) const {
  if (!hasRolled_) {
    std::snprintf(out, n, "");
    return;
  }
  if (mode_ == DiceMode::Coin) {
    std::snprintf(out, n, "%s", lastValue_ == 0 ? "heads" : "tails");
  } else {
    std::snprintf(out, n, "%d", lastValue_);
  }
}

}  // namespace pc
