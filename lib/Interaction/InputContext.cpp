#include "InputContext.h"

namespace pc {
namespace {
// Klassifikations-Schwellen (leicht tunbar).
constexpr std::uint32_t kDoubleTapMs   = 400;   // max. Abstand fuer Doppel-Tap
constexpr std::uint32_t kRapidWindowMs = 1600;  // Fenster fuer schnelles Tippen
constexpr std::uint16_t kRapidTaps     = 4;     // Taps im Fenster -> RapidTap
}  // namespace

void InputContext::begin() {
  singleTap_ = doubleTap_ = rapidTap_ = false;
  btnA_ = btnB_ = btnC_ = pwr_ = false;
  lastTapMs_ = 0;
  rapidWindowStart_ = 0;
  lastActivityMs_ = 0;
  idleMs_ = 0;
  rapidCount_ = 0;
}

void InputContext::update(std::uint32_t nowMs, const Input& input) {
  singleTap_ = doubleTap_ = rapidTap_ = false;
  btnA_ = input.btnAPressed();
  btnB_ = input.btnBPressed();
  btnC_ = input.btnCPressed();
  pwr_  = input.btnPwrPressed();

  if (input.wasPressed()) {
    singleTap_ = true;

    // Doppel-Tap: zweiter Tap kurz nach dem ersten.
    if (lastTapMs_ != 0 && nowMs - lastTapMs_ <= kDoubleTapMs) {
      doubleTap_ = true;
    }
    lastTapMs_ = nowMs;

    // Rapid-Tap: genug Taps innerhalb des Fensters.
    if (nowMs - rapidWindowStart_ > kRapidWindowMs) {
      rapidWindowStart_ = nowMs;
      rapidCount_ = 1;
    } else {
      ++rapidCount_;
    }
    if (rapidCount_ >= kRapidTaps) {
      rapidTap_ = true;
      rapidCount_ = 0;
    }
  }

  if (singleTap_ || btnA_ || btnB_ || btnC_ || pwr_) lastActivityMs_ = nowMs;
  idleMs_ = nowMs - lastActivityMs_;
}

}  // namespace pc
