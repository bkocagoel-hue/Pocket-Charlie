#include "EightBall.h"

#include <Arduino.h>

namespace pc {
namespace {
// Bewusst kein klassisches "Signs point to yes" - eigene, alberne/"dank"
// Antworten statt der bekannten Mattel-Formulierungen. Gemischt aus
// Ja/Nein/Chaos, damit sich die Antwort nie vorhersehbar anfuehlt.
constexpr const char* kAnswers[] = {
    "Obviously. Duh.",
    "100%. No cap.",
    "Send it.",
    "Yes - chaos approves.",
    "The vibes say yes.",
    "Manifesting a yes.",
    "Big nope energy.",
    "Absolutely not, bestie.",
    "Hard pass.",
    "Nah, fam.",
    "The universe said lol no.",
    "404: answer not found.",
    "Ask again, but louder.",
    "Reboot and try again.",
    "Between yes and cursed.",
    "Buffering... forever.",
    "Consult your local raccoon.",
    "The answer is: tacos.",
    "Charlie shrugs internally.",
    "Ask the WiFi router.",
};
constexpr int kAnswerN = sizeof(kAnswers) / sizeof(kAnswers[0]);
}  // namespace

void EightBall::begin() {
  lastIndex_ = 0;
  hasAsked_ = false;
}

void EightBall::ask() {
  lastIndex_ = random(kAnswerN);
  hasAsked_ = true;
}

const char* EightBall::answerText() const {
  if (!hasAsked_) return "";
  return kAnswers[lastIndex_];
}

}  // namespace pc
