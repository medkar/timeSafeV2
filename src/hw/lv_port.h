#pragma once

// Initialise l'écran (LovyanGFX) + LVGL (flush + lecture tactile + tick).
void lvport_init();

// À appeler en boucle : traite les timers/rendus LVGL.
void lvport_loop();
