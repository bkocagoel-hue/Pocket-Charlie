#pragma once
// ============================================================================
//  Emotion - Charlies lokale Grundstimmungen
//
//  Nur das Modell (Enum). Die sichtbare Darstellung (Parameter/Animation) liegt
//  im Face-Modul (datengetrieben, siehe Face.cpp: styleFor()). Ableitung siehe
//  docs/PERSONALITY.md. Neutral steht bewusst zuerst (Ausgangszustand).
// ============================================================================

namespace pc {

enum class Emotion {
  // Kern-Emotionen (Sprint 2, sichtbar umgesetzt)
  Neutral,     // Grundzustand
  Happy,       // Freude (Boot-Gruss, Touch)
  Tired,       // muede (lange Inaktivitaet)
  Thoughtful,  // nachdenklich (BtnB)
  Annoyed,     // genervt (schnelles Dauer-Tippen)

  // Vorbereitet (v1: Naeherung im Face-Stil; eigene Feinanimation ist TODO)
  Curious,
  Sad,
  Sleeping,
  WakingUp,
  Excited,
  Confused,
};

}  // namespace pc
