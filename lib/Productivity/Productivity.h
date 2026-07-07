#pragma once
// ============================================================================
//  Productivity - lokale Fokus-Werkzeuge (Sprint 5, Fudge)
//
//  Drei Modi in einem Screen: Stopwatch, Countdown, Pomodoro. Reine
//  millis()-Zustandslogik ohne delay() - Zeit laeuft unabhaengig von der
//  Bildrate und blockiert nie. Bedienung: BtnB kurz = start/pause/weiter,
//  BtnB halten = reset/stop (bzw. Moduswechsel, wenn nichts laeuft).
//
//  Ereignisse (Start/Pause/Done/...) werden als Einmal-Flags gesammelt und
//  von der App per takeEvent() abgeholt -> Emotion/Sound/Microcopy bleiben
//  Sache der App, dieses Modul kennt weder Persona noch Display.
//  local-first: keinerlei Netz-/WLAN-Abhaengigkeit.
// ============================================================================

#include <cstddef>
#include <cstdint>

namespace pc {

enum class ProdMode : std::uint8_t { Stopwatch = 0, Countdown, Pomodoro };
enum class ProdStatus : std::uint8_t { Ready = 0, Running, Paused, Done };

// Einmal-Ereignisse fuer die App (per takeEvent() abholen, danach None).
enum class ProdEvent : std::uint8_t {
  None = 0,
  Started,
  Paused,
  Resumed,
  Reset,
  ModeChanged,
  CountdownDone,
  FocusDone,  // Pomodoro: Fokus fertig -> Break startet automatisch
  BreakDone,  // Pomodoro: Break fertig -> Durchgang geschafft (Session +1)
};

class Productivity {
 public:
  void begin();

  // Jede Loop-Runde aufrufen: schreibt Zeit fort, erkennt Timer-Ende.
  void update(std::uint32_t nowMs);

  // BtnB kurz: start / pause / weiter (bzw. Done bestaetigen -> bereit).
  void primaryAction(std::uint32_t nowMs);
  // BtnB halten: Ready -> naechster Modus; sonst reset/stop.
  void secondaryAction(std::uint32_t nowMs);

  ProdMode mode() const { return mode_; }
  ProdStatus status() const { return status_; }
  std::uint16_t session() const { return session_; }

  const char* modeName() const;                     // "stopwatch"/"timer"/...
  void timeText(char* out, std::size_t n) const;    // "MM:SS" bzw. "H:MM:SS"
  void subText(char* out, std::size_t n) const;     // Status (+Phase/Session)
  const char* actionHint() const;                   // fuer die NavBar
  std::uint32_t displaySeconds() const;             // fuer Redraw-Erkennung

  ProdEvent takeEvent() {
    const ProdEvent e = event_;
    event_ = ProdEvent::None;
    return e;
  }

 private:
  std::uint32_t elapsedMs() const;
  std::uint32_t targetMs() const;  // 0 = Stoppuhr (kein Ziel)
  void resetTimer();

  ProdMode mode_ = ProdMode::Stopwatch;
  ProdStatus status_ = ProdStatus::Ready;
  bool inBreak_ = false;       // Pomodoro: aktuelle Phase
  std::uint16_t session_ = 1;  // Pomodoro: Durchgang (zaehlt nach Break hoch)
  std::uint32_t accumMs_ = 0;     // angesammelte Laufzeit (ohne aktuellen Lauf)
  std::uint32_t runStartMs_ = 0;  // Beginn des aktuellen Laufs
  std::uint32_t nowMs_ = 0;       // letzter update()-Zeitpunkt (fuer Anzeige)
  ProdEvent event_ = ProdEvent::None;
};

}  // namespace pc
