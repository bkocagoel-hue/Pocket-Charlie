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

// Sprint 4 (E4B): Microcopy fuer die freigeschalteten Emotionen.
constexpr const char* kCurious[]  = {"hmm?", "oh?", "neat."};
constexpr const char* kConfused[] = {"huh?", "eh?", "wait..."};
constexpr const char* kExcited[]  = {"yay!", "nice!", "online!"};
constexpr const char* kSad[]      = {"oh.", "hm.", "aw."};
constexpr const char* kWakeUp[]   = {"...", "morning", "mhh"};

constexpr int kGreetN    = 5;
constexpr int kGrumbleN  = 4;
constexpr int kIdleN     = 5;
constexpr int kIdleHighN = 5;
constexpr int kIdleLowN  = 5;
constexpr int kCuriousN  = 3;
constexpr int kConfusedN = 3;
constexpr int kExcitedN  = 3;
constexpr int kSadN      = 3;
constexpr int kWakeUpN   = 3;

}  // namespace phrases
}  // namespace pc
