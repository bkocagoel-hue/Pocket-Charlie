#include "FocusCard.h"

#include <Arduino.h>

namespace pc {
namespace {
// Kurze, lesbare Prompts - ruhig und charmant, kein Motivationscoach-
// Gebruell (gleicher Ton wie Phrases.h, aber eigene, vollstaendige Saetze
// statt winziger Sprechblasen-Microcopy).
constexpr const char* kFocus[] = {
    "Do one small thing.",
    "Close the extra tabs.",
    "Pick the next step.",
    "Write the first line.",
    "Set a 5-minute timer.",
    "Clear your desk.",
};
constexpr int kFocusN = 6;

constexpr const char* kBreak[] = {
    "Look away from the screen.",
    "Stretch for a moment.",
    "Drink some water.",
    "Breathe out, slowly.",
    "Stand up and walk.",
};
constexpr int kBreakN = 5;

constexpr const char* kReset[] = {
    "Start smaller.",
    "It's okay to pause.",
    "Reread your goal.",
    "Let go of perfect.",
    "Begin again.",
};
constexpr int kResetN = 5;
}  // namespace

void FocusCard::begin() {
  category_ = CardCategory::Focus;
  lastIndex_ = 0;
  hasDrawn_ = false;
}

void FocusCard::cyclePrev() {
  category_ = static_cast<CardCategory>((static_cast<int>(category_) + 2) % 3);
  hasDrawn_ = false;  // neue Kategorie zeigt keine alte Karte mehr an
}

void FocusCard::cycleNext() {
  category_ = static_cast<CardCategory>((static_cast<int>(category_) + 1) % 3);
  hasDrawn_ = false;
}

void FocusCard::draw() {
  switch (category_) {
    case CardCategory::Focus: lastIndex_ = random(kFocusN); break;
    case CardCategory::Break: lastIndex_ = random(kBreakN); break;
    case CardCategory::Reset: lastIndex_ = random(kResetN); break;
  }
  hasDrawn_ = true;
}

const char* FocusCard::categoryName() const {
  switch (category_) {
    case CardCategory::Focus: return "focus";
    case CardCategory::Break: return "break";
    case CardCategory::Reset: return "reset";
    default:                  return "?";
  }
}

const char* FocusCard::cardText() const {
  if (!hasDrawn_) return "";
  switch (category_) {
    case CardCategory::Focus: return kFocus[lastIndex_];
    case CardCategory::Break: return kBreak[lastIndex_];
    case CardCategory::Reset: return kReset[lastIndex_];
    default:                  return "";
  }
}

}  // namespace pc
