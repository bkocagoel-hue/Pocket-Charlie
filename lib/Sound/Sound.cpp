#include "Sound.h"

#include <Arduino.h>
#include <M5Unified.h>
#include <cmath>

namespace pc {
namespace {
constexpr std::uint8_t kVolume = 72;  // moderat (0..255) - dezent, nicht laut

// Sprint 7 (Beatbox, Mini-App 3): kurze, prozedural erzeugte Drum-PCM-
// Samples statt reiner tone()-Beeps. M5Unified stellt echte PCM-Wiedergabe
// (playRaw) und 8 Misch-Kanaele bereit, die bis hierhin ungenutzt waren.
// Einmalig bei begin() synthetisiert (kein Neuberechnen pro Hit), danach
// nur noch abgespielt - so bleibt jeder Hit sofort/non-blocking wie tone().
constexpr std::uint32_t kBeatSampleRate = 11025;
constexpr std::size_t kKickLen  = kBeatSampleRate * 150 / 1000;
constexpr std::size_t kSnareLen = kBeatSampleRate * 130 / 1000;
constexpr std::size_t kHihatLen = kBeatSampleRate * 70  / 1000;
constexpr std::size_t kClapLen  = kBeatSampleRate * 120 / 1000;
constexpr float kTwoPi = 6.28318530718f;

std::int16_t kickBuf[kKickLen];
std::int16_t snareBuf[kSnareLen];
std::int16_t hihatBuf[kHihatLen];
std::int16_t clapBuf[kClapLen];
bool beatSamplesReady = false;

// Kleiner, deterministischer PRNG fuer die Rauschanteile (Snare/Hihat/
// Clap) - echter Zufall ist hier nicht noetig, ein Drum-Sample klingt bei
// jedem Hit bewusst gleich (wie bei einer echten Drum-Machine), kein
// <random> fuer ein paar Rauschsamples.
std::uint32_t noiseState = 0x9E3779B9u;
float nextNoise() {
  noiseState ^= noiseState << 13;
  noiseState ^= noiseState >> 17;
  noiseState ^= noiseState << 5;
  return (static_cast<float>(noiseState) / 4294967295.0f) * 2.0f - 1.0f;  // -1..1
}

void synthKick() {
  float phase = 0.0f;
  for (std::size_t i = 0; i < kKickLen; ++i) {
    const float t = static_cast<float>(i) / kBeatSampleRate;
    const float freq = 45.0f + 140.0f * std::exp(-18.0f * t);  // Sweep 185->45Hz
    const float amp = std::exp(-14.0f * t);
    phase += kTwoPi * freq / kBeatSampleRate;
    kickBuf[i] = static_cast<std::int16_t>(amp * std::sin(phase) * 26000.0f);
  }
}

void synthSnare() {
  float phase = 0.0f;
  for (std::size_t i = 0; i < kSnareLen; ++i) {
    const float t = static_cast<float>(i) / kBeatSampleRate;
    const float ampTone  = std::exp(-16.0f * t);
    const float ampNoise = std::exp(-10.0f * t);
    phase += kTwoPi * 190.0f / kBeatSampleRate;
    const float s = ampTone * std::sin(phase) * 0.45f + ampNoise * nextNoise() * 0.55f;
    snareBuf[i] = static_cast<std::int16_t>(s * 24000.0f);
  }
}

void synthHihat() {
  float prevNoise = 0.0f;
  for (std::size_t i = 0; i < kHihatLen; ++i) {
    const float t = static_cast<float>(i) / kBeatSampleRate;
    const float amp = std::exp(-45.0f * t);
    const float n = nextNoise();
    const float highPassed = n - prevNoise;  // einfacher Differenzfilter -> mehr Hoehen
    prevNoise = n;
    hihatBuf[i] = static_cast<std::int16_t>(amp * highPassed * 16000.0f);
  }
}

void synthClap() {
  for (std::size_t i = 0; i < kClapLen; ++i) {
    const float t = static_cast<float>(i) / kBeatSampleRate;
    // Drei kurze Rausch-Buerste statt eines einzelnen Decays - klingt eher
    // nach "Klatschen" als nach reinem weissen Rauschen.
    const float burstPhase = std::fmod(t, 0.03f) / 0.03f;
    const float burstAmp = std::exp(-30.0f * burstPhase);
    const float envelope = std::exp(-9.0f * t);
    clapBuf[i] = static_cast<std::int16_t>(nextNoise() * burstAmp * envelope * 20000.0f);
  }
}

void ensureBeatSamples() {
  if (beatSamplesReady) return;
  synthKick();
  synthSnare();
  synthHihat();
  synthClap();
  beatSamplesReady = true;
}
}  // namespace

void Sound::begin() {
  available_ = M5.Speaker.isEnabled();
  ensureBeatSamples();  // billig (~5000 Samples), unabhaengig von available_
  if (available_) {
    M5.Speaker.setVolume(kVolume);
    Serial.println("[Sound] Speaker bereit (dezent).");
  } else {
    Serial.println("[Sound] kein Speaker verfuegbar -> stumm, alles laeuft.");
  }
}

void Sound::tone(float freqHz, std::uint32_t durMs) {
  if (!enabled_ || !available_) return;
  M5.Speaker.tone(freqHz, durMs);  // non-blocking, kurzer Einzelton
}

void Sound::playRaw(int channel, const std::int16_t* pcmData, std::size_t sampleCount) {
  if (!enabled_ || !available_) return;
  // stop_current_sound=true: ein erneuter Hit auf denselben Kanal setzt
  // sofort neu ein, statt den vorherigen Hit erst ausklingen zu lassen -
  // fuehlt sich bei schnellem Antippen direkter an.
  M5.Speaker.playRaw(pcmData, sampleCount, kBeatSampleRate, /*stereo=*/false,
                      /*repeat=*/1, channel, /*stop_current_sound=*/true);
}

void Sound::playTimerDone() { tone(880.0f, 160); }   // A5, freundlich
void Sound::playFocusDone() { tone(988.0f, 160); }   // H5, "Pause verdient"
void Sound::playBreakDone() { tone(1319.0f, 200); }  // E6, kleiner Abschluss

void Sound::playPocketOpen()   { tone(740.0f, 90); }    // F#5, kurzer Blip
void Sound::playPocketClose()  { tone(392.0f, 90); }    // G4, tiefer = "zu"
void Sound::playPocketMove()   { tone(520.0f, 40); }    // C5, sehr kurzes Tick
void Sound::playPocketSelect() { tone(1046.0f, 130); }  // C6, warmer Confirm

void Sound::playAdjustTick() { tone(660.0f, 50); }  // E5, neutrales In-Screen-Tick

void Sound::playDiceRoll() { tone(300.0f, 70); }  // D4, kurzer "Thud"

void Sound::playCardDraw() { tone(880.0f, 60); }  // A5, kurzer "Flip"

// Eigener Kanal je Drum-Sound (0..3) - so schneidet ein Hihat-Hit einen
// gerade ausklingenden Kick nicht ab, auch wenn schnell zwischen Sounds
// gewechselt wird.
void Sound::playBeatboxKick()  { playRaw(0, kickBuf,  kKickLen); }
void Sound::playBeatboxSnare() { playRaw(1, snareBuf, kSnareLen); }
void Sound::playBeatboxHihat() { playRaw(2, hihatBuf, kHihatLen); }
void Sound::playBeatboxClap()  { playRaw(3, clapBuf,  kClapLen); }

}  // namespace pc
