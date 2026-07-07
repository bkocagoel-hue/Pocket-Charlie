#pragma once
// ============================================================================
//  Sound - dezentes Audio-Feedback (Sprint 5, Fudge)
//
//  Ein kurzer, freundlicher Ton je Timer-Ereignis - keine Melodien, keine
//  Loops, kein Dauerpiepen. M5.Speaker.tone() spielt non-blocking im
//  Hintergrund, die UI friert nie ein. Defensiv: ohne verfuegbaren Speaker
//  bleibt alles stumm und Charlie laeuft normal weiter.
//  Abschaltbar (enabled-Flag) - der Settings-Screen haengt sich hier ein.
// ============================================================================

#include <cstdint>

namespace pc {

class Sound {
 public:
  void begin();  // prueft Speaker-Verfuegbarkeit, setzt moderate Lautstaerke

  void setEnabled(bool on) { enabled_ = on; }
  bool enabled() const { return enabled_; }

  void playTimerDone();  // Countdown fertig
  void playFocusDone();  // Pomodoro: Fokus fertig (Break beginnt)
  void playBreakDone();  // Pomodoro: Durchgang geschafft

 private:
  void tone(float freqHz, std::uint32_t durMs);

  bool enabled_ = true;
  bool available_ = false;
};

}  // namespace pc
