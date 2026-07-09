#pragma once
// ============================================================================
//  Beatbox - kleine Drum-Machine (Sprint 7, Nougat: dritte Pocketindex-
//  Mini-App)
//
//  Vier Sounds: Kick/Snare/Hihat/Clap, als 2x2-Touch-Raster direkt auf dem
//  Screen (siehe Display::showBeatboxGrid()/beatboxZoneAt() und
//  App::handleInput()) - kein Umweg mehr ueber A/C-Zyklen wie bei
//  Dice/FocusCard, ein Tap auf eine Zone loest den Hit direkt aus. Beatbox
//  selbst kennt nur "welcher Sound wurde zuletzt getroffen" (fuer optionales
//  Tap-Feedback); die eigentliche PCM-Wiedergabe lebt weiterhin in Sound
//  (Sound::playBeatbox*), von App.cpp ausgeloest.
// ============================================================================

#include <cstdint>

namespace pc {

enum class DrumKit : std::uint8_t { Kick = 0, Snare, Hihat, Clap };

class Beatbox {
 public:
  void begin();

  void hit(DrumKit kit);  // Touch-Zone getroffen

  DrumKit lastKit() const { return lastKit_; }
  bool hasHit() const { return hasHit_; }

  // "kick" / "snare" / "hihat" / "clap" - statisch, da alle vier Namen
  // gleichzeitig als Zonen-Label gezeichnet werden (kein "aktueller" Sound
  // mehr wie noch bei Dice/FocusCard).
  static const char* kitName(DrumKit kit);

 private:
  DrumKit lastKit_ = DrumKit::Kick;
  bool hasHit_ = false;
};

}  // namespace pc
