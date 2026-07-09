#include "Beatbox.h"

namespace pc {

void Beatbox::begin() {
  lastKit_ = DrumKit::Kick;
  hasHit_ = false;
}

void Beatbox::hit(DrumKit kit) {
  lastKit_ = kit;
  hasHit_ = true;
}

const char* Beatbox::kitName(DrumKit kit) {
  switch (kit) {
    case DrumKit::Kick:  return "kick";
    case DrumKit::Snare: return "snare";
    case DrumKit::Hihat: return "hihat";
    case DrumKit::Clap:  return "clap";
    default:             return "?";
  }
}

}  // namespace pc
