#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Adafruit_NeoPixel.hpp"

// Copied from pico-examples
#include "nec_receive_library/nec_receive.c"

#include "colours.h"
#include "common_defaults.h"

// 1 status LED for many units, 3 for units with a neopixel status light.
#ifndef STATUS_LEDS
#define STATUS_LEDS 1
#endif

#if STATUS_LEDS == 1
#define STATUS_LED_PIN 25
#elif STATUS_LEDS == 3
#define STATUS_LED_R_PIN 16
#define STATUS_LED_G_PIN 17
#define STATUS_LED_B_PIN 18
#endif

bool ledIsLit = true;

Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(TOTAL_NEOPIXELS, NEOPIXELS_PIN, NEO_RGB + NEO_KHZ800);

// Common "state" variables

bool keep_running = true;
int ticks = 0;
int rx_sm;

// Used for "off" and "on" toggle
bool isLit = false;

// TODO: This should be a more defined structure
int colourMode = 0; // 0: Solid, 1: Colour Cycling 2: Aurora

TARGET_COLOUR current_colour = TC_WHITE;

int brightness_pct = 25;

int continuous_colour_change_index = 0;


// End "state" variables

void setup () {
    // NeoPixel Init
    neopixels.begin();
    neopixels.clear();
    neopixels.show();
  
    stdio_init_all();

    // LED Init
    #if STATUS_LEDS == 1
    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
    #elif STATUS_LEDS == 3
    gpio_init(STATUS_LED_R_PIN);
    gpio_set_dir(STATUS_LED_R_PIN, GPIO_OUT);

    gpio_init(STATUS_LED_G_PIN);
    gpio_set_dir(STATUS_LED_G_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_G_PIN, 1);

    gpio_init(STATUS_LED_B_PIN);
    gpio_set_dir(STATUS_LED_B_PIN, GPIO_OUT);
    #endif

    // IR Init
    rx_sm = nec_rx_init(pio0, IR_RX_GPIO_PIN);

    if (rx_sm == -1) {
        // TODO: Log this or flash lights if we often have errors like this.
    }
}

int next_index (int current_index, int direction, int numPixels) {
    int raw_index = current_index + direction;
    return (raw_index + numPixels) % numPixels;
}

/*

    This code supports most of the IR signals used by the 'practical series ii'
    remote that controls various cheap party lights:

    00:45 - On button (at last used brightness)
    00:46 - White + Dimmed brightness
    00:47 - Off button
    00:44 - 'Stepped' colour cycling
    00:40 - Turn on timer?
    00:43 - Add one hour to timer
    00:07 - Continuous transitions between colours
    00:15 - Increase brightness
    00:09 - Decrease brightness
    00:16 - Red
    00:19 - Green
    00:0d - Blue
    00:0c - Orange
    00:18 - Light Green
    00:5e - Light Blue
    00:08 - Blueish Purple
    00:1c - Light Orange
    00:5a - Lightest Blue
    00:42 - Reddish Purple
    00:52 - Yellow
    00:4a - White + Full Brightness

*/

uint8_t rx_address, rx_data, previous_rx_data;

void poll_infrared () {
    // Adapted from pico-examples
    while (!pio_sm_is_rx_fifo_empty(pio0, rx_sm)) {
        uint32_t rx_frame = pio_sm_get(pio0, rx_sm);

        if (nec_decode_frame(rx_frame, &rx_address, &rx_data)) {
            // Brief flicker to give the impression that something was received.
            neopixels.clear();
            neopixels.show();

            #if STATUS_LEDS == 3
            // Flicker the onboard LED since we don't have onboard neopixels for this unit.
            gpio_put(STATUS_LED_R_PIN, 0);
            #endif

            // Non-continuous controls
            if (rx_data != previous_rx_data) {               
                if (isLit) {
                        switch(rx_data) {
                        // 00:46 - White + Dimmed brightness
                        case 0x46:
                            colourMode = 0;
                            brightness_pct = MIN_BRIGHTNESS_PCT;
                            current_colour = TC_WHITE;
                            break;
                        // 00:47 - Off button
                        case 0x47:
                            isLit = false;
                            break;
                        // 00:44 - 'Stepped' colour cycling
                        case 0x44:
                            colourMode = 1;
                            break;
                        // 00:40 - Turn on timer (ignored)
                        // 00:43 - Add one hour to timer (ignored)
                        case 0x40:
                        case 0x43:
                            break;
                        // 00:07 - Continuous transitions between colours
                        case 0x07:
                            colourMode = 2;
                            break;
                        // 00:15 - Increase brightness
                        case 0x15:
                            if ((brightness_pct + BRIGHTNESS_DELTA) <= MAX_BRIGHTNESS_PCT) {
                              brightness_pct += BRIGHTNESS_DELTA;
                            }
                            break;
                        // 00:09 - Decrease brightness
                        case 0x09:
                            if ((brightness_pct - BRIGHTNESS_DELTA) >= MIN_BRIGHTNESS_PCT) {
                              brightness_pct -= BRIGHTNESS_DELTA;
                            }
                            break;
                        // 00:16 - Red
                        case 0x16:
                            colourMode = 0;
                            current_colour = TC_RED;
                            break;
                        // 00:19 - Green
                        case 0x19:
                            colourMode = 0;
                            current_colour = TC_GREEN;
                            break;
                        // 00:0d - Blue
                        case 0x0d:
                            colourMode = 0;
                            current_colour = TC_BLUE;
                            break;
                        // 00:0c - Orange
                        case 0x0c:
                            colourMode = 0;
                            current_colour = TC_ORANGE;
                            break;
                        // 00:18 - Light Green
                        case 0x18:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_GREEN;
                            break;
                        // 00:5e - Light Blue
                        case 0x5e:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_BLUE;
                            break;
                        // 00:08 - Blueish Purple
                        case 0x08:
                            colourMode = 0;
                            current_colour = TC_INDIGO;
                            break;
                        // 00:1c - Light Orange
                        case 0x1c:
                            colourMode = 0;
                            current_colour = TC_LIGHTER_ORANGE;
                            break;
                        // 00:5a - Lightest Blue
                        case 0x5a:
                            colourMode = 0;
                            current_colour = TC_LIGHTEST_BLUE;
                            break;
                        // 00:42 - Reddish Purple
                        case 0x42:
                            colourMode = 0;
                            current_colour = TC_VIOLET;
                            break;
                        // 00:52 - Yellow
                        case 0x52:
                            colourMode = 0;
                            current_colour = TC_YELLOW;
                            break;
                        // 00:4a - White + Full Brightness
                        case 0x4a:
                            colourMode = 0;
                            current_colour = TC_WHITE;
                            brightness_pct = MAX_BRIGHTNESS_PCT;
                            break;
                        default:
                            break;
                    }
                }
                // 00:45 - On button (at last used brightness)
                else if (rx_data == 0x45) {
                    isLit = true;
                }

            }

            previous_rx_data = rx_data;
        }
        // Controls that can also be held down, doesn't seem to work.
        else {
            if (isLit) {
                switch(rx_data) {
                    // 00:15 - Increase brightness
                    case 0x15:
                        if ((brightness_pct + BRIGHTNESS_DELTA) <= MAX_BRIGHTNESS_PCT) {
                            brightness_pct += BRIGHTNESS_DELTA;
                        }
                        break;
                    // 00:09 - Decrease brightness
                    case 0x09:
                        if ((brightness_pct - BRIGHTNESS_DELTA) >= MIN_BRIGHTNESS_PCT) {
                            brightness_pct -= BRIGHTNESS_DELTA;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

// TODO: Make the colour more interesting, i.e. add pulsing or waves.
void redraw_with_current_colour () {
    float brightness_multiplier = brightness_pct / 100.0;
    neopixels.fill(neopixels.Color(current_colour.r * brightness_multiplier, current_colour.g * brightness_multiplier, current_colour.b * brightness_multiplier));
    neopixels.show();
}

typedef struct Orbit {
    int cx;
    int cy;

    int orbit_radius;
    int orbit_angle;
    int orbit_angle_delta;

    int planet_radius;
    uint32_t colour;
} Orbit;

static Orbit redOrbit = {
    .cx = 0,
    .cy = 0,
    .orbit_radius = 15,
    .orbit_angle = 0,
    .orbit_angle_delta = 8,

    .planet_radius = 15,
    .colour = (uint32_t) (255 << 16)
};

// redOrbit.colour = neopixels.Color(255, 0, 0);

static Orbit greenOrbit = {
    .cx = 20,
    .cy = 0,
    .orbit_radius = 15,
    .orbit_angle = 180,
    .orbit_angle_delta = 10,

    .planet_radius = 15,
    .colour = (uint32_t) (255 << 8)
};

// greenOrbit.colour = neopixels.Color(0, 255, 0);

static Orbit blueOrbit = {
    .cx = 10,
    .cy = 20,
    .orbit_radius = 10,
    .orbit_angle = 270,
    .orbit_angle_delta = 12,

    .planet_radius = 15,
    .colour = (uint32_t) 255
};

// blueOrbit.colour = neopixels.Color(0, 0, 255);


void update_aurora() {
    redOrbit.orbit_angle = (redOrbit.orbit_angle + redOrbit.orbit_angle_delta) % 360;
    greenOrbit.orbit_angle = (greenOrbit.orbit_angle + greenOrbit.orbit_angle_delta) % 360;
    blueOrbit.orbit_angle = (blueOrbit.orbit_angle + blueOrbit.orbit_angle_delta) % 360;
}

void add_orbit_to_grid (Orbit orbit, uint32_t *grid) {
    double planetRadians = orbit.orbit_angle * (M_PI / 180);

    int planetCX = orbit.cx + (cos(planetRadians) * orbit.orbit_radius);
    int planetCY = orbit.cy + (sin(planetRadians) * orbit.orbit_radius);

    for (int row = 0; row < 20; row++) {
        for (int column = 0; column < 20; column++) {
            int gridIndex = (row * 20) + column;
            int distance;

            if (row == planetCY) {
                distance = fabs(planetCX - column);
            }
            else if (column == planetCX) {
                distance = fabs(planetCY - row);
            }
            else {
                // Triangulate the distance
                distance = sqrt(
                    pow((planetCX - column) ,2) +
                    pow((planetCY - row) ,2)
                );
            }

            if (distance <= orbit.planet_radius) {
                int energyPercentage;
                if (distance == 0) {
                    energyPercentage = 100;
                }
                else {
                    energyPercentage = 100 * (orbit.planet_radius - distance)/orbit.planet_radius;
                }

                uint8_t currentGridRedLevel = (grid[gridIndex] >> 16) & 0xff;
                uint8_t currentGridGreenLevel = (grid[gridIndex] >> 8) & 0xff;
                uint8_t currentGridBlueLevel = grid[gridIndex] & 0xff;

                uint8_t orbitRedLevel = (orbit.colour >> 16) & 0xff;
                uint8_t orbitGreenLevel = (orbit.colour >> 8) & 0xff;
                uint8_t orbitBlueLevel = orbit.colour & 0xff;

                uint8_t newRedLevel = (uint8_t)((int)(currentGridRedLevel + (orbitRedLevel * energyPercentage)/100) % 256);
                uint8_t newGreenLevel = (uint8_t)((int)(currentGridGreenLevel + (orbitGreenLevel * energyPercentage)/100) % 256);
                uint8_t newBlueLevel = (uint8_t)((int)(currentGridBlueLevel + (orbitBlueLevel * energyPercentage)/100) % 256);

                uint32_t newColour = ((newRedLevel & 0xff) << 16) | ((newGreenLevel & 0xff) << 8) | (newBlueLevel & 0xff);

                grid[gridIndex] = newColour;
            }
        }
    }
}

void redraw_aurora() {
    uint32_t colourGrid[400] = { 0 };

    add_orbit_to_grid(redOrbit,   colourGrid);
    add_orbit_to_grid(greenOrbit, colourGrid);
    add_orbit_to_grid(blueOrbit,  colourGrid);

    for (int index = 0; index < 400; index++) {
        neopixels.setPixelColor(index, colourGrid[index]);
    }

    neopixels.show();
}

int main() {
    setup();

    while (keep_running) {
        // Blink our status LED
        if ((ticks % LED_BLINK_TICKS) == 0) {
            ledIsLit = !ledIsLit;

            // The unit we use for "1" uses "high" to indicate whether something
            // is lit. The unit we use for "3" uses low.
            #if STATUS_LEDS == 1
            gpio_put(STATUS_LED_PIN, ledIsLit);
            #elif STATUS_LEDS == 3
            // The "R" light indicates that a signal was received, we clear it after .5 seconds.
            gpio_put(STATUS_LED_R_PIN, 1);
            gpio_put(STATUS_LED_B_PIN, !ledIsLit);
            #endif
        }

        // Changes coming from infrared are tracked first.
        if (rx_sm != -1) {
            poll_infrared();
        }

        // Some modes can also "dirty" the display periodically

        if (colourMode == 1) {
            if ((ticks % TICKS_PER_CONTINUOUS_COLOUR_CHANGE) == 0) {
                continuous_colour_change_index = (continuous_colour_change_index + 1) % 6;
                current_colour = COLOUR_WHEEL[continuous_colour_change_index];
            }
        }
        else if (colourMode == 2) {
            if ((ticks % TICKS_PER_AURORA_UPDATE) == 0) {
                update_aurora();
            }
        }

        if (isLit) {
            // "Continuous" colour switching, i.e. the full-on aurora.
            if (colourMode == 2) {
                redraw_aurora();
            }
            else {
                redraw_with_current_colour();
            }
        }
        else {
            // Turn off all pixels.
            neopixels.clear();
            neopixels.show();
        }

        ticks++;
        sleep_ms(MS_PER_TICK);
    }
}