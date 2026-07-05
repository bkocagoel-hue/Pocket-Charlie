#include "Persona.h"

namespace pc {

void Persona::begin() {
  current_ = Emotion::Neutral;
}

void Persona::update(std::uint32_t /*nowMs*/) {
  // Sprint 2, Backlog-Item 3: noch keine Trigger -> Emotion bleibt Neutral.
  // Ab spaeteren Items kommen hier die Regeln (Inaktivitaet, Touch, BtnB-Hold).
}

}  // namespace pc
