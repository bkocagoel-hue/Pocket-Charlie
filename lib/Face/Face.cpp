#include "Face.h"

#include <Arduino.h>
#include <math.h>

#include "PcConfig.h"

namespace pc {
namespace {

// ---------------------------------------------------------------------------
//  Gesichts-Layout / Geometrie + Visual-Tuning (alle Werte leicht anpassbar)
//  Anordnung (mittig, klar getrennt): Augen -> Schnurrbart -> Mund.
//
//  Visual identity tuning:
//   - Schnurrbart: kStacheHalfW / kStacheEndUp / kStacheDip / kStacheCurlR / kStacheY
//   - Augen: kEyeOpenPupil (ab wann Pupille) / kEyeClosedLineH (geschlossene Linie)
//   - Farben (Auge/Pupille/Gesicht): siehe PcConfig.h (Charlie-Theme)
// ---------------------------------------------------------------------------
constexpr std::int16_t kEyeW       = 60;
constexpr std::int16_t kEyeH       = 76;
constexpr std::int16_t kEyeCornerR = 18;
constexpr std::int16_t kEyeGap     = 116;  // Abstand der Augenmittelpunkte
constexpr std::int16_t kEyeCenterY = 100;
constexpr std::int16_t kPupilR     = 13;
constexpr float        kEyeOpenPupil    = 0.40f;  // ab dieser Oeffnung: Pupille sichtbar
constexpr float        kEyeClosedThresh = 0.12f;  // darunter: explizite geschlossene Linie
constexpr std::int16_t kEyeClosedLineH  = 6;      // Hoehe der geschlossenen Lid-Linie

// Schnurrbart: aktuell DEAKTIVIERT (auf kleinem Display noch nicht ueberzeugend).
// Architektur/Hilfsfunktion bleiben erhalten -> spaeter als echtes Pixel-Sprite.
// Zum Reaktivieren: kEnableMoustache = true.
constexpr bool kEnableMoustache = false;
// Schnurrbart-Geometrie (genutzt, wenn kEnableMoustache = true):
constexpr std::int16_t kStacheY     = 152;  // vertikale Mitte
constexpr std::int16_t kStacheGap   = 4;    // halbe Mittel-Luecke (zwei Haelften)
constexpr std::int16_t kStacheHalfW = 30;   // Armlaenge je Haelfte
constexpr std::int16_t kStacheThick = 7;    // Strichdicke
constexpr std::int16_t kStacheEndUp = 12;   // Anhebung der Enden (Schwung)
constexpr std::int16_t kStacheDip   = 4;    // Absenkung in der Mitte (Philtrum)
constexpr std::int16_t kStacheCurlR = 5;    // Radius der eingerollten Enden (Curl)

// Mund (datengetrieben: Kurve -1..+1)
constexpr std::int16_t kMouthY     = 168;
constexpr std::int16_t kMouthHalfW = 32;
constexpr float        kMouthAmp   = 11.0f;
constexpr std::int16_t kMouthCols  = 22;
constexpr std::int16_t kMouthThick = 5;

// --- Blick (Gaze) ---
constexpr std::int16_t kMaxGazeX   = 16;
constexpr std::int16_t kMaxGazeY   = 9;
constexpr float        kGazeSmooth = 0.18f;

// --- Blinzeln ---
constexpr std::uint32_t kBlinkCloseMs  = 90;
constexpr std::uint32_t kBlinkOpenMs   = 130;
constexpr std::uint32_t kBlinkMinGapMs = 2500;
constexpr std::uint32_t kBlinkMaxGapMs = 6000;

// --- Blickwechsel (Idle) ---
constexpr std::uint32_t kGazeMinGapMs = 1400;
constexpr std::uint32_t kGazeMaxGapMs = 3800;
constexpr int           kGazeCenterPercent = 35;

// --- Atmen ---
constexpr float kBreathAmpPx = 2.5f;
constexpr float kBreathHz    = 0.31f;
constexpr float kTwoPi       = 6.28318530718f;

// --- Emotions-Stil (datengetrieben, weich interpolierbar) ---
constexpr float kStyleLerp = 0.14f;

enum GazeMode : std::uint8_t { GazeIdle = 0, GazeUp = 1, GazeSide = 2 };

struct EmotionStyle {
  float eyeOpen;     // Basis-Augenoeffnung 0..1
  float mouthCurve;  // -1 (Frown) .. 0 (flach) .. +1 (Laecheln)
  float mustache;    // -1 (haengt) .. 0 .. +1 (angehoben)
  float blinkMul;    // Blinzel-Intervall-Faktor (groesser = langsamer)
  std::uint8_t gaze;
};

// Emotion -> Stil. Kern-Emotionen deutlich unterscheidbar; die vorbereiteten
// Emotionen nutzen v1-Naeherungen (TODO: eigene Feinanimationen).
EmotionStyle styleFor(Emotion e) {
  switch (e) {
    case Emotion::Happy:      return {0.95f,  0.90f,  1.0f, 1.0f, GazeIdle};
    case Emotion::Tired:      return {0.45f, -0.20f, -1.0f, 1.9f, GazeIdle};
    case Emotion::Thoughtful: return {1.00f,  0.00f,  0.0f, 1.2f, GazeUp};
    case Emotion::Annoyed:    return {0.62f, -0.55f,  0.6f, 1.0f, GazeSide};
    case Emotion::Curious:    return {1.00f,  0.25f,  0.3f, 0.9f, GazeSide};
    case Emotion::Sad:        return {0.70f, -0.80f, -1.0f, 1.3f, GazeIdle};
    case Emotion::Sleeping:   return {0.05f,  0.00f, -1.0f, 3.0f, GazeIdle};
    case Emotion::WakingUp:   return {0.55f,  0.10f, -0.5f, 1.5f, GazeIdle};
    case Emotion::Excited:    return {1.00f,  1.00f,  1.0f, 0.8f, GazeIdle};
    case Emotion::Confused:   return {0.95f, -0.10f,  0.2f, 1.1f, GazeSide};
    case Emotion::Neutral:
    default:                  return {1.00f,  0.00f,  0.0f, 1.0f, GazeIdle};
  }
}

inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
inline std::int16_t iround(float v) {
  return static_cast<std::int16_t>(v < 0 ? v - 0.5f : v + 0.5f);
}

}  // namespace

void Face::begin(std::int16_t screenW, std::int16_t screenH) {
  screenW_ = screenW;
  screenH_ = screenH;
  randomSeed(micros());

  canvas_.setColorDepth(16);
  canvas_.setPsram(true);
  ready_ = (canvas_.createSprite(screenW_, screenH_) != nullptr);
  if (!ready_) {
    Serial.println("[Face] FEHLER: Canvas konnte nicht angelegt werden (RAM?).");
  }

  const std::uint32_t now = millis();
  scheduleNextBlink(now);
  scheduleNextGaze(now);

  const EmotionStyle s = styleFor(Emotion::Neutral);
  sEyeOpen_ = s.eyeOpen;
  sMouthCurve_ = s.mouthCurve;
  sMustache_ = s.mustache;
  sBlinkMul_ = s.blinkMul;
  gazeMode_ = s.gaze;
}

void Face::scheduleNextBlink(std::uint32_t nowMs) {
  const std::uint32_t gap = static_cast<std::uint32_t>(
      random(kBlinkMinGapMs, kBlinkMaxGapMs) * sBlinkMul_);
  nextBlinkAt_ = nowMs + gap;
}

void Face::scheduleNextGaze(std::uint32_t nowMs) {
  nextGazeChangeAt_ = nowMs + random(kGazeMinGapMs, kGazeMaxGapMs);
  if (random(100) < kGazeCenterPercent) {
    gazeTargetX_ = 0.0f;
    gazeTargetY_ = 0.0f;
  } else {
    gazeTargetX_ = static_cast<float>(random(-kMaxGazeX, kMaxGazeX + 1));
    gazeTargetY_ = static_cast<float>(random(-kMaxGazeY, kMaxGazeY + 1));
  }
}

void Face::blinkNow() {
  if (!blinking_) {
    blinking_ = true;
    blinkStartedAt_ = millis();
  }
}

void Face::setEmotion(Emotion e) {
  emotion_ = e;  // Ziel-Emotion; der Stil wird in update() sanft nachgezogen.
}

void Face::lookAt(std::int16_t x, std::int16_t y) {
  const float cx = screenW_ * 0.5f;
  const float cy = screenH_ * 0.5f;
  gazeTargetX_ = clampf((x - cx) / cx * kMaxGazeX, -kMaxGazeX, kMaxGazeX);
  gazeTargetY_ = clampf((y - cy) / cy * kMaxGazeY, -kMaxGazeY, kMaxGazeY);
  nextGazeChangeAt_ = millis() + 1500;
}

void Face::update(std::uint32_t nowMs) {
  // --- Emotions-Stil weich nachziehen (sanfte Uebergaenge) ---
  const EmotionStyle tgt = styleFor(emotion_);
  sEyeOpen_    += (tgt.eyeOpen    - sEyeOpen_)    * kStyleLerp;
  sMouthCurve_ += (tgt.mouthCurve - sMouthCurve_) * kStyleLerp;
  sMustache_   += (tgt.mustache   - sMustache_)   * kStyleLerp;
  sBlinkMul_   += (tgt.blinkMul   - sBlinkMul_)   * kStyleLerp;
  gazeMode_ = tgt.gaze;

  // --- Blinzeln (im Schlaf ausgesetzt: Augen bleiben geschlossen) ---
  if (emotion_ == Emotion::Sleeping) {
    blinking_ = false;
    eyeOpen_ = 1.0f;  // der Sleeping-Stil haelt die Augen zu (kein Blinzeln)
  } else if (blinking_) {
    const std::uint32_t t = nowMs - blinkStartedAt_;
    if (t < kBlinkCloseMs) {
      eyeOpen_ = 1.0f - static_cast<float>(t) / kBlinkCloseMs;
    } else if (t < kBlinkCloseMs + kBlinkOpenMs) {
      eyeOpen_ = static_cast<float>(t - kBlinkCloseMs) / kBlinkOpenMs;
    } else {
      eyeOpen_ = 1.0f;
      blinking_ = false;
      scheduleNextBlink(nowMs);
    }
  } else {
    eyeOpen_ = 1.0f;
    if (nowMs >= nextBlinkAt_) blinkNow();
  }

  // --- Blickrichtung: emotionsabhaengig ---
  if (gazeMode_ == GazeUp) {
    gazeTargetX_ = -6.0f;
    gazeTargetY_ = -static_cast<float>(kMaxGazeY);
  } else if (gazeMode_ == GazeSide) {
    gazeTargetX_ = static_cast<float>(kMaxGazeX);
    gazeTargetY_ = 2.0f;
  } else if (nowMs >= nextGazeChangeAt_) {
    scheduleNextGaze(nowMs);
  }
  gazeX_ += (gazeTargetX_ - gazeX_) * kGazeSmooth;
  gazeY_ += (gazeTargetY_ - gazeY_) * kGazeSmooth;

  // --- Atmen ---
  bobY_ = sinf(static_cast<float>(nowMs) * 0.001f * kBreathHz * kTwoPi) *
          kBreathAmpPx;
}

void Face::drawEye(std::int16_t cx, std::int16_t cy, float openAmount,
                   std::int16_t gazeX, std::int16_t gazeY) {
  // Ansatz "Augenlid": der volle Augapfel wird gezeichnet und von OBEN durch ein
  // Lid (Hintergrundfarbe) verdeckt. So schliesst das Auge sichtbar von oben und
  // endet geschlossen als klare Linie - statt einfach zu schrumpfen.
  const std::int16_t top = static_cast<std::int16_t>(cy - kEyeH / 2);

  // Fast geschlossen -> klar lesbares, geschlossenes Lid als Linie zeichnen.
  if (openAmount < kEyeClosedThresh) {
    canvas_.fillRoundRect(cx - kEyeW / 2, cy - kEyeClosedLineH / 2, kEyeW,
                          kEyeClosedLineH, kEyeClosedLineH / 2, config::kColorEye);
    return;
  }

  // Voller Augapfel/Iris = warmes Violett.
  canvas_.fillRoundRect(cx - kEyeW / 2, top, kEyeW, kEyeH, kEyeCornerR,
                        config::kColorEye);

  // Oberlid-Hoehe (verdeckt oben) vorab bestimmen.
  std::int16_t lidH = iround(kEyeH * (1.0f - openAmount));
  const std::int16_t maxLid = static_cast<std::int16_t>(kEyeH - kEyeClosedLineH);
  if (lidH > maxLid) lidH = maxLid;

  // Weisse Pupille in den SICHTBAREN (unteren) Bereich setzen, damit sie auch bei
  // reduzierter Augenoeffnung (z. B. Annoyed/Tired) klar sichtbar bleibt.
  if (openAmount > kEyeOpenPupil) {
    const std::int16_t px = static_cast<std::int16_t>(cx + gazeX);
    std::int16_t py = static_cast<std::int16_t>(cy + lidH / 2 + gazeY);
    const std::int16_t visTop = static_cast<std::int16_t>(top + lidH);
    const std::int16_t visBot = static_cast<std::int16_t>(cy + kEyeH / 2);
    if (py < visTop + kPupilR) py = static_cast<std::int16_t>(visTop + kPupilR);
    if (py > visBot - kPupilR) py = static_cast<std::int16_t>(visBot - kPupilR);
    canvas_.fillCircle(px, py, kPupilR, config::kColorPupil);
  }

  // Oberlid: verdeckt (1 - openAmount) der Hoehe von oben (nach der Pupille,
  // damit es sie beim Schliessen sauber ueberdeckt).
  if (lidH > 0) {
    canvas_.fillRect(cx - kEyeW / 2 - 1, top - 1,
                     static_cast<std::int16_t>(kEyeW + 2),
                     static_cast<std::int16_t>(lidH + 1),
                     config::kColorBackground);
  }
}

void Face::drawMouth(std::int16_t cx, std::int16_t baseY, float curve) {
  // Parabel entlang der Breite: curve>0 = Laecheln (Enden hoch), <0 = Frown.
  const float amp = clampf(curve, -1.0f, 1.0f) * kMouthAmp;
  for (std::int16_t i = 0; i < kMouthCols; ++i) {
    const float t =
        (static_cast<float>(i) / (kMouthCols - 1)) * 2.0f - 1.0f;  // -1..+1
    const std::int16_t x = static_cast<std::int16_t>(cx + t * kMouthHalfW);
    const std::int16_t y = static_cast<std::int16_t>(baseY - amp * (t * t));
    canvas_.fillRect(x - 2, y - kMouthThick / 2, 4, kMouthThick,
                     config::kColorFace);
  }
}

void Face::drawMustache(std::int16_t cx, std::int16_t baseY, float lift) {
  // Geschwungener Curly-Handlebar: zwei gespiegelte Haelften, in der Mitte leicht
  // tiefer (Philtrum), zu den Enden nach oben geschwungen, mit kleiner eingerollter
  // Curl-Andeutung an den Spitzen. lift>0 hebt (Happy), lift<0 senkt (Tired).
  const float baseFY = baseY - lift * 2.0f;
  const int kSteps = 12;
  for (std::int16_t side = -1; side <= 1; side += 2) {
    std::int16_t tipX = cx;
    std::int16_t tipY = static_cast<std::int16_t>(baseFY);
    for (int i = 0; i <= kSteps; ++i) {
      const float s = static_cast<float>(i) / kSteps;  // 0=Mitte .. 1=Spitze
      const std::int16_t x =
          static_cast<std::int16_t>(cx + side * (kStacheGap + s * kStacheHalfW));
      const std::int16_t y = static_cast<std::int16_t>(
          baseFY + kStacheDip * (1.0f - s) - kStacheEndUp * (s * s));
      canvas_.fillCircle(x, y, kStacheThick / 2, config::kColorFace);
      tipX = x;
      tipY = y;
    }
    // Eingerolltes Ende: kleiner Ring nach oben-aussen (Curl-Andeutung).
    const std::int16_t curlCx =
        static_cast<std::int16_t>(tipX + side * kStacheCurlR);
    const std::int16_t curlCy = static_cast<std::int16_t>(tipY - kStacheCurlR);
    canvas_.fillCircle(curlCx, curlCy, kStacheCurlR, config::kColorFace);
    canvas_.fillCircle(curlCx, curlCy,
                       static_cast<std::int16_t>(kStacheCurlR - 2),
                       config::kColorBackground);
  }
}

void Face::render() {
  if (!ready_) return;

  canvas_.fillScreen(config::kColorBackground);

  const std::int16_t cx = screenW_ / 2;
  const std::int16_t bob = iround(bobY_);
  const std::int16_t eyeY = static_cast<std::int16_t>(kEyeCenterY + bob);
  const std::int16_t gx = iround(gazeX_);
  const std::int16_t gy = iround(gazeY_);

  const float eye = clampf(eyeOpen_ * sEyeOpen_, 0.0f, 1.0f);
  drawEye(cx - kEyeGap / 2, eyeY, eye, gx, gy);
  drawEye(cx + kEyeGap / 2, eyeY, eye, gx, gy);

  if (kEnableMoustache) {
    drawMustache(cx, static_cast<std::int16_t>(kStacheY + bob), sMustache_);
  }
  drawMouth(cx, static_cast<std::int16_t>(kMouthY + bob), sMouthCurve_);

  // Dezente Zustands-Symbole (verschwinden automatisch beim Emotionswechsel).
  if (emotion_ == Emotion::Sleeping) {
    // Animiertes "zZz" oben rechts: z -> zZ -> zZz (steigt nach oben-rechts).
    const std::uint32_t tnow = millis();
    const int count = 1 + static_cast<int>((tnow / 650) % 3);  // 1..3 Zeichen
    const char seq[3] = {'z', 'Z', 'z'};
    canvas_.setTextColor(config::kColorFace);
    canvas_.setTextDatum(bottom_left);
    canvas_.setTextSize(2);
    for (int i = 0; i < count; ++i) {
      const char c[2] = {seq[i], '\0'};
      canvas_.drawString(c, screenW_ - 58 + i * 13,
                         static_cast<std::int16_t>(44 - i * 11));
    }
  } else if (emotion_ == Emotion::Thoughtful) {
    canvas_.setTextColor(config::kColorEye);  // violetter Denk-Akzent
    canvas_.setTextDatum(top_left);
    canvas_.setTextSize(3);
    canvas_.drawString("?", screenW_ - 44, 26);
  }

  // TODO (Sprint 2): seltene Idle-Easter-Eggs hier einklinken (nur Platzhalter).

  canvas_.pushSprite(&M5.Display, 0, 0);
}

}  // namespace pc
