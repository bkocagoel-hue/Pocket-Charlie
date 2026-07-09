#include "Productivity.h"

#include <cstdio>

namespace pc {
namespace {
// Standard-Dauern. Fuer Hardware-Tests kann PC_PROD_TEST_TIMINGS als
// Build-Flag gesetzt werden (kurze Zeiten); der normale Standard bleibt
// Countdown 5 min, Pomodoro 25/5 min.
#ifdef PC_PROD_TEST_TIMINGS
constexpr std::uint32_t kCountdownMs = 15UL * 1000UL;
constexpr std::uint32_t kFocusMs     = 20UL * 1000UL;
constexpr std::uint32_t kBreakMs     = 10UL * 1000UL;
#else
constexpr std::uint32_t kCountdownMs = 5UL * 60UL * 1000UL;
constexpr std::uint32_t kFocusMs     = 25UL * 60UL * 1000UL;
constexpr std::uint32_t kBreakMs     = 5UL * 60UL * 1000UL;
#endif
}  // namespace

void Productivity::begin() {
  mode_ = ProdMode::Stopwatch;
  session_ = 1;
  lapCount_ = 0;
  countdownTargetMs_ = kCountdownMs;
  focusTargetMs_ = kFocusMs;
  breakTargetMs_ = kBreakMs;
  resetTimer();
}

void Productivity::resetTimer() {
  status_ = ProdStatus::Ready;
  accumMs_ = 0;
  runStartMs_ = 0;
  inBreak_ = false;
}

std::uint32_t Productivity::targetMs() const {
  switch (mode_) {
    case ProdMode::Countdown: return countdownTargetMs_;
    case ProdMode::Pomodoro:  return inBreak_ ? breakTargetMs_ : focusTargetMs_;
    default:                  return 0;  // Stoppuhr zaehlt frei hoch
  }
}

std::uint32_t Productivity::elapsedMs() const {
  std::uint32_t e = accumMs_;
  if (status_ == ProdStatus::Running) e += nowMs_ - runStartMs_;
  const std::uint32_t t = targetMs();
  if (t != 0 && e > t) e = t;  // nie ueber das Ziel hinaus / nie negativ
  return e;
}

void Productivity::update(std::uint32_t nowMs) {
  nowMs_ = nowMs;
  if (status_ != ProdStatus::Running) return;
  const std::uint32_t t = targetMs();
  if (t == 0 || elapsedMs() < t) return;  // Stoppuhr frei / Ziel nicht erreicht

  if (mode_ == ProdMode::Countdown) {
    accumMs_ = t;  // exakt bei 00:00 einfrieren (nie negativ)
    status_ = ProdStatus::Done;
    event_ = ProdEvent::CountdownDone;
  } else if (mode_ == ProdMode::Pomodoro) {
    if (!inBreak_) {
      // Fokus geschafft -> Break startet automatisch (ruhig, kein Nerven).
      inBreak_ = true;
      accumMs_ = 0;
      runStartMs_ = nowMs;
      event_ = ProdEvent::FocusDone;
    } else {
      // Break vorbei -> Durchgang geschafft; naechster Fokus bewusst per BtnB.
      ++session_;
      inBreak_ = false;
      accumMs_ = 0;
      status_ = ProdStatus::Done;
      event_ = ProdEvent::BreakDone;
    }
  }
}

void Productivity::primaryAction(std::uint32_t nowMs) {
  nowMs_ = nowMs;
  switch (status_) {
    case ProdStatus::Ready:
      runStartMs_ = nowMs;
      status_ = ProdStatus::Running;
      event_ = ProdEvent::Started;
      break;
    case ProdStatus::Running:
      accumMs_ = elapsedMs();  // Zeit einfrieren
      status_ = ProdStatus::Paused;
      event_ = ProdEvent::Paused;
      break;
    case ProdStatus::Paused:
      runStartMs_ = nowMs;  // ab hier laeuft es weiter (accum bleibt)
      status_ = ProdStatus::Running;
      event_ = ProdEvent::Resumed;
      break;
    case ProdStatus::Done:
      resetTimer();  // bestaetigen -> bereit fuer die naechste Runde
      break;
  }
}

void Productivity::cycleModePrev() {
  // Sprint 7, Fix: nur im Ready-Zustand sinnvoll (nichts laeuft, das
  // Werkzeug wird noch gewaehlt) - waehrend Running/Paused/Done haben A/C
  // ihre eigene, direkte Bedeutung (siehe resetStopwatch()/markLap()/
  // adjustTarget()), ein Moduswechsel waere dort verwirrend/verlustreich.
  if (status_ != ProdStatus::Ready) return;
  mode_ = static_cast<ProdMode>((static_cast<int>(mode_) + 2) % 3);  // -1 mod 3
  resetTimer();
  session_ = 1;
  event_ = ProdEvent::ModeChanged;
}

void Productivity::cycleModeNext() {
  if (status_ != ProdStatus::Ready) return;
  mode_ = static_cast<ProdMode>((static_cast<int>(mode_) + 1) % 3);
  resetTimer();
  session_ = 1;
  event_ = ProdEvent::ModeChanged;
}

void Productivity::resetStopwatch(std::uint32_t nowMs) {
  // Sprint 7, Fix: unbedingter, eindeutiger Reset fuer BtnA im Stopwatch-
  // Modus - anders als secondaryAction() oben NIE ein Moduswechsel, egal
  // in welchem Status. So stimmt der Footer-Hinweis "A: reset" immer.
  nowMs_ = nowMs;
  resetTimer();
  session_ = 1;
  event_ = ProdEvent::Reset;
}

void Productivity::markLap() {
  ++lapCount_;  // bewusst nur ein Zaehler, keine Lap-Historie (Sprint 7, Fix)
}

void Productivity::adjustTarget(std::int32_t deltaMs) {
  std::uint32_t* target = nullptr;
  if (mode_ == ProdMode::Countdown) {
    target = &countdownTargetMs_;
  } else if (mode_ == ProdMode::Pomodoro) {
    target = inBreak_ ? &breakTargetMs_ : &focusTargetMs_;
  } else {
    return;  // Stoppuhr hat kein Ziel
  }
  constexpr std::int64_t kMinMs = 60UL * 1000UL;       // 1 Minute
  constexpr std::int64_t kMaxMs = 90UL * 60UL * 1000UL;  // 90 Minuten
  std::int64_t next = static_cast<std::int64_t>(*target) + deltaMs;
  if (next < kMinMs) next = kMinMs;
  if (next > kMaxMs) next = kMaxMs;
  *target = static_cast<std::uint32_t>(next);
}

const char* Productivity::modeName() const {
  switch (mode_) {
    case ProdMode::Stopwatch: return "stopwatch";
    case ProdMode::Countdown: return "timer";
    case ProdMode::Pomodoro:  return "pomodoro";
    default:                  return "?";
  }
}

void Productivity::timeText(char* out, std::size_t n) const {
  // Stoppuhr zeigt die verstrichene Zeit, Countdown/Pomodoro die Restzeit.
  std::uint32_t ms = elapsedMs();
  const std::uint32_t t = targetMs();
  if (t != 0) ms = t - ms;
  const std::uint32_t sec = ms / 1000UL;
  if (sec >= 3600UL) {
    std::snprintf(out, n, "%u:%02u:%02u", static_cast<unsigned>(sec / 3600UL),
                  static_cast<unsigned>((sec / 60UL) % 60UL),
                  static_cast<unsigned>(sec % 60UL));
  } else {
    std::snprintf(out, n, "%02u:%02u", static_cast<unsigned>(sec / 60UL),
                  static_cast<unsigned>(sec % 60UL));
  }
}

void Productivity::subText(char* out, std::size_t n) const {
  const char* status = status_ == ProdStatus::Running ? "running"
                     : status_ == ProdStatus::Paused  ? "paused"
                     : status_ == ProdStatus::Done    ? "done"
                                                      : "ready";
  if (mode_ == ProdMode::Pomodoro) {
    // Laufend zeigt die Phase, sonst den Status - immer mit Session-Zaehler.
    const char* label = (status_ == ProdStatus::Running)
                            ? (inBreak_ ? "break" : "focus")
                            : status;
    std::snprintf(out, n, "%s s%u", label, static_cast<unsigned>(session_));
  } else {
    std::snprintf(out, n, "%s", status);
  }
}

const char* Productivity::actionHint() const {
  // Sprint 7, Fix: "hold: mode"/"hold: reset" entfernt - BtnB-Halten ist
  // jetzt global dem Pocketindex vorbehalten (siehe App::handleButtons()),
  // ein Hinweis darauf hier waere irrefuehrend (Halten tut auf diesem
  // Screen nichts Eigenes mehr, siehe cycleModePrev()/cycleModeNext()).
  switch (status_) {
    case ProdStatus::Ready:   return "B: start";
    case ProdStatus::Running: return "B: pause";
    case ProdStatus::Paused:  return "B: resume";
    case ProdStatus::Done:    return "B: ok";
    default:                  return "";
  }
}

const char* Productivity::aHint() const {
  if (status_ == ProdStatus::Ready) return "A: prev tool";
  return (mode_ == ProdMode::Stopwatch) ? "A: reset" : "A: -1m";
}

const char* Productivity::cHint() const {
  if (status_ == ProdStatus::Ready) return "C: next tool";
  return (mode_ == ProdMode::Stopwatch) ? "C: lap" : "C: +1m";
}

std::uint32_t Productivity::displaySeconds() const {
  std::uint32_t ms = elapsedMs();
  const std::uint32_t t = targetMs();
  if (t != 0) ms = t - ms;
  return ms / 1000UL;
}

}  // namespace pc
