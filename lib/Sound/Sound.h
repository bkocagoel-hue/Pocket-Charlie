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

  // Sprint 7 (Pocketindex - Rolodex Notebook): kurze UI-Blips, dieselbe
  // Disziplin wie oben - ein einzelner, kurzer tone()-Aufruf, kein Loop,
  // respektiert enabled()/available_ ueber dieselbe private tone().
  void playPocketOpen();    // Pocketindex geoeffnet
  void playPocketClose();   // Pocketindex geschlossen
  void playPocketMove();    // Kartenwechsel (A/C) - bewusst sehr kurz, da
                            // haeufig ausgeloest
  void playPocketSelect();  // Karte per B gewaehlt

  // Sprint 7, Fix (Screen-eigene A/C-Funktionen): kurzes, neutrales Tick
  // fuer direkte In-Screen-Aktionen (Minute +/-, Lap-Marke) - unterscheidet
  // sich bewusst von den Pocketindex-Toenen oben, damit "im Menue" und
  // "in der App" auch akustisch unterscheidbar bleiben.
  void playAdjustTick();

  // Sprint 7: kurzer "Wuerfel/Muenze"-Ton fuers Dice-Ergebnis (Mini-App 1).
  void playDiceRoll();

 private:
  void tone(float freqHz, std::uint32_t durMs);

  bool enabled_ = true;
  bool available_ = false;
};

}  // namespace pc
