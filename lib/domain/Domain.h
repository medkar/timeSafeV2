#pragma once
#include <cstdint>

namespace tsafe {

// États possibles décidés par LockPolicy (spec §6).
enum class PolicyState {
    Setup,        // non armée : menu de configuration
    Alert,        // anomalie de temps : verrouillé
    WaitingSync,  // date requise mais temps non fiable : verrouillé
    Countdown,    // date requise, pas encore atteinte : verrouillé
    AskPassword,  // conditions de date OK, mot de passe requis
    Unlock        // toutes conditions remplies : ouvrir
};

// Configuration armée de la boîte (partie utile à la décision pure).
struct BoxConfig {
    bool armed = false;
    bool hasDate = false;
    int64_t openDate = 0;   // epoch secondes UTC
    bool hasPassword = false;
};

// Résultat de l'autorité de temps (spec §5).
struct TimeStatus {
    bool trusted = false;
    int64_t effectiveNow = 0;  // valide seulement si trusted == true (max des sources plausibles)
    bool anomaly = false;
};

// État persistant des tentatives de mot de passe (spec §9).
struct AttemptState {
    int failedCount = 0;
    int64_t lockedUntil = 0;   // epoch secondes ; 0 = pas de verrouillage temporisé
};

// Entrée de LockPolicy::decide.
struct PolicyInput {
    BoxConfig config;
    TimeStatus time;
    bool passwordSatisfied = false;  // un mot de passe correct a été saisi durant cette session
    AttemptState attempts;
    int64_t now = 0;                 // meilleur temps courant disponible (pour backoff / rebours)
};

// Résultat de LockPolicy::decide.
struct PolicyResult {
    PolicyState state = PolicyState::Setup;
    bool lockedOut = false;        // AskPassword : dans la fenêtre de temporisation
    int64_t retryAt = 0;           // epoch s : fin de la temporisation
    int64_t remainingSeconds = 0;  // Countdown : secondes avant openDate
};

} // namespace tsafe
