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
    case ProdMode::Countdown: return kCountdownMs;
    case ProdMode::Pomodoro:  return inBreak_ ? kBreakMs : kFocusMs;
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

void Productivity::secondaryAction(std::uint32_t nowMs) {
  nowMs_ = nowMs;
  if (status_ == ProdStatus::Ready) {
    // Nichts laeuft: Halten wechselt den Modus - so bleibt BtnA/BtnC reine
    // Screen-Navigation und Halten kollidiert nie mit einem Reset.
    mode_ = static_cast<ProdMode>((static_cast<int>(mode_) + 1) % 3);
    resetTimer();
    session_ = 1;
    event_ = ProdEvent::ModeChanged;
    return;
  }
  resetTimer();
  session_ = 1;
  event_ = ProdEvent::Reset;
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
  switch (status_) {
    case ProdStatus::Ready:   return "B: start   hold: mode";
    case ProdStatus::Running: return "B: pause   hold: reset";
    case ProdStatus::Paused:  return "B: resume  hold: reset";
    case ProdStatus::Done:    return "B: ok      hold: reset";
    default:                  return "";
  }
}

std::uint32_t Productivity::displaySeconds() const {
  std::uint32_t ms = elapsedMs();
  const std::uint32_t t = targetMs();
  if (t != 0) ms = t - ms;
  return ms / 1000UL;
}

}  // namespace pc
