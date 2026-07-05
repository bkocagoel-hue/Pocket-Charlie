#pragma once
// ============================================================================
//  Emotion - Charlies lokale Grundstimmungen (Sprint 2, Vorbereitung)
//
//  Nur das Modell (Enum) - noch KEINE Logik, KEIN Rendering. Ableitung der
//  Emotionen siehe docs/PERSONALITY.md. Neutral steht bewusst zuerst
//  (Standard-/Ausgangszustand).
// ============================================================================

namespace pc {

enum class Emotion {
  Neutral,     // Grundzustand
  Happy,       // Freude (Boot-Gruss, Touch)
  Tired,       // muede (lange Inaktivitaet)
  Thoughtful,  // nachdenklich (BtnB-Hold)
  Annoyed,     // genervt (Piesacken)
};

}  // namespace pc
