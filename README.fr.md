# TimeSafe

*Une boîte scellée qui ne s'ouvre qu'à une date choisie — une capsule temporelle
physique impossible à ouvrir avant l'heure — et qui fait aussi coffre à mot de
passe / code PIN.*

![platform](https://img.shields.io/badge/platform-ESP32-blue)
![tests](https://img.shields.io/badge/tests%20natifs-69%20OK-brightgreen)
![license](https://img.shields.io/badge/license-MIT-green)

🇬🇧 [English version](README.md)

<!-- DEMO : ajouter assets/demo.gif (compte à rebours → ouverture) une fois la boîte montée -->

## Le concept

TimeSafe est une boîte ESP32 scellée, imprimée en 3D, avec écran tactile. On
règle une date cible et elle devient une **capsule temporelle** : le verrou ne
s'ouvre pas avant ce moment — et sans pile ni issue de secours, elle est
réellement inviolable avant l'heure. On peut aussi (ou à la place) définir un
mot de passe ou un PIN : c'est alors un **coffre** classique. Aucun secours par
conception.

## Fonctionnalités

- **Verrou temporel** à deux sources de temps de confiance (HTTPS épinglé + RTC DS3231)
- Coffre **mot de passe ou PIN 6 chiffres**, avec temporisation exponentielle après erreurs
- **5 thèmes d'occasion** (cadeau, souvenir, fête, enfant, coffre), gardés après reboot
- **Config WiFi à l'écran** — sans ordinateur
- **Survit à la coupure** — date de capsule, hash et thème persistés en NVS
- Bâti sur un **cœur pur et testable sur PC** : 69 tests unitaires, sans matériel

## Comment ça marche

**Architecture.** Un cœur logique pur (`lib/`) derrière des interfaces ; les
adaptateurs matériels (`src/hw`) et l'UI LVGL (`src/ui`) s'y branchent. Le cœur
n'a aucune dépendance Arduino, d'où 69 tests natifs sur PC. Schéma complet :
[docs/architecture.md](docs/architecture.md).

**Temps de confiance (anti-triche).** La boîte lit l'heure d'une réponse
**HTTPS épinglée** (qui cale l'horloge système) et d'un **RTC DS3231**. Elle
ouvre si *l'une* des sources plausibles a atteint la date (règle OU / maximum),
et reste **verrouillée** si aucune n'est fiable (fail-closed).

**Sécurité.** Les mots de passe sont stockés en **hash salé
PBKDF2-HMAC-SHA256**, jamais en clair. Les erreurs déclenchent une
**temporisation exponentielle** persistée en NVS. Aucune porte dérobée.

<!-- MATÉRIEL : ajouter BOM + câblage (ou lien vers hardware/BUILD.md) une fois la boîte montée -->

## Compiler & lancer

Nécessite [PlatformIO](https://platformio.org/).

```bash
# Tests unitaires sur PC (aucun matériel)
pio test -e native

# Configurer le WiFi, puis compiler & flasher
cp src/secrets.h.example src/secrets.h   # puis éditer avec ton réseau 2.4 GHz
pio run -e esp32dev -t upload
```

Adapte le câblage à ta carte dans [`include/board_config.h`](include/board_config.h).

## Avertissement

Projet hobby, pas un produit de sécurité certifié — **à voir comme une capsule
temporelle ludique, pas comme un vrai coffre-fort.** Comme le verrou ouvre dès
qu'*une* source atteint la date, une source qui déraille vers l'avant pourrait
ouvrir en avance. Sans secours, un mot de passe oublié = boîte perdue.

## Licence

Code sous [licence MIT](LICENSE).

## Crédits

Construit avec [LVGL](https://lvgl.io/), [LovyanGFX](https://github.com/lovyan03/LovyanGFX),
[RTClib](https://github.com/adafruit/RTClib) et [ESP32Servo](https://github.com/madhephaestus/ESP32Servo).
