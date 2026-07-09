#pragma once
// ============================================================================
//  Productivity - lokale Fokus-Werkzeuge (Sprint 5, Fudge)
//
//  Drei Modi in einem Screen: Stopwatch, Countdown, Pomodoro. Reine
//  millis()-Zustandslogik ohne delay() - Zeit laeuft unabhaengig von der
//  Bildrate und blockiert nie. Bedienung: BtnB kurz = start/pause/weiter.
//  BtnB HALTEN ist seit Sprint 7 global dem Pocketindex vorbehalten (siehe
//  App::handleButtons()) - Moduswechsel lebt deshalb auf A/C im Ready-
//  Zustand (cycleModePrev()/cycleModeNext()), nicht mehr auf BtnB-Halten.
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

  // Sprint 7, Fix (Screen-eigene Tasten statt globaler Navigation): A/C
  // bekommen je Zustand/Modus eine eigene, direkte Bedeutung.
  //
  // Ready (nichts laeuft): A/C waehlen das Werkzeug (Stopwatch/Timer/
  // Pomodoro) - ersetzt das fruehere "BtnB halten" bei Ready.
  void cycleModePrev();
  void cycleModeNext();
  // Running/Paused/Done: Stopwatch: A = unbedingter Reset, C = Lap-Marke
  // (zaehlt nur, keine Historie). Countdown/Pomodoro: A = Ziel-Minute
  // runter, C = rauf (wirkt auf die aktive Phase, Pomodoro: Fokus oder
  // Break je nach inBreak_; geklemmt auf 1..90 Minuten).
  void resetStopwatch(std::uint32_t nowMs);
  void markLap();
  std::uint16_t lapCount() const { return lapCount_; }
  void adjustTarget(std::int32_t deltaMs);

  ProdMode mode() const { return mode_; }
  ProdStatus status() const { return status_; }
  std::uint16_t session() const { return session_; }

  const char* modeName() const;                     // "stopwatch"/"timer"/...
  void timeText(char* out, std::size_t n) const;    // "MM:SS" bzw. "H:MM:SS"
  void subText(char* out, std::size_t n) const;     // Status (+Phase/Session)
  const char* actionHint() const;                   // fuer die NavBar (BtnB)
  const char* aHint() const;                        // fuer die NavBar (BtnA)
  const char* cHint() const;                        // fuer die NavBar (BtnC)
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

  // Sprint 7, Fix: per A/C verstellbare Ziele (Default = die bisherigen
  // Konstanten, siehe begin()/Productivity.cpp). Bleiben ueber Reset/
  // Modus-Wechsel hinweg bestehen - nur begin() (Neustart) setzt sie
  // zurueck, ein einfacher Reset soll eine bewusst laenger/kuerzer
  // gestellte Zeit nicht wegwerfen.
  std::uint32_t countdownTargetMs_ = 0;
  std::uint32_t focusTargetMs_ = 0;
  std::uint32_t breakTargetMs_ = 0;

  // Sprint 7, Fix: Stopwatch-Lap-Zaehler (nur Zaehlung, keine Historie -
  // bewusst einfach gehalten).
  std::uint16_t lapCount_ = 0;
};

}  // namespace pc
