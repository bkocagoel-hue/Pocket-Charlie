#pragma once
// ============================================================================
//  Phrases - kurze, dezente lokale Microcopy fuer Charlie (Sprint 3)
//
//  Ton: ruhig, freundlich, minimalistisch, leicht frech - nie nervig. Sehr kurze
//  Texte, damit sie auf dem CoreS3 gut lesbar bleiben. Reine Daten; die Auswahl
//  (random) macht der Aufrufer.
// ============================================================================

namespace pc {
namespace phrases {

constexpr const char* kGreet[]   = {"hi", "hey", "ok", ":)", "yo"};
constexpr const char* kGrumble[] = {"hey!", "stop", "ugh", "bro..."};
constexpr const char* kIdle[]     = {"hmm", "...", "tap?", "sup?", "donut?"};
constexpr const char* kIdleHigh[] = {"yay", ":)", "hi!", "fun", "donut!"};
constexpr const char* kIdleLow[]  = {"meh", "ugh", "tired", "hmph", "..."};

constexpr int kGreetN    = 5;
constexpr int kGrumbleN  = 4;
constexpr int kIdleN     = 5;
constexpr int kIdleHighN = 5;
constexpr int kIdleLowN  = 5;

}  // namespace phrases
}  // namespace pc
