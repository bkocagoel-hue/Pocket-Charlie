#pragma once
// ============================================================================
//  EightBall - Magic 8-Ball (Sprint 7, Nougat: vierte Pocketindex-Mini-App)
//
//  Eine einzige Antwort-Poule (keine Kategorien wie FocusCard) - B zieht
//  eine zufaellige, absichtlich alberne/"dank" Antwort. A/C bewusst ohne
//  eigene Funktion (wie Beatbox) - eine Frage stellt man nur auf eine Art.
//  local-first, kein Netz-/Bridge-Bezug.
// ============================================================================

#include <cstdint>

namespace pc {

class EightBall {
 public:
  void begin();

  void ask();  // BtnB: neue Antwort

  bool hasAsked() const { return hasAsked_; }
  const char* answerText() const;  // aktuelle Antwort, oder "" wenn noch keine

 private:
  int lastIndex_ = 0;
  bool hasAsked_ = false;
};

}  // namespace pc
