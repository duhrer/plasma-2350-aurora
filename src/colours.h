typedef struct {
    int r;
    int g;
    int b;
} TARGET_COLOUR;

TARGET_COLOUR TC_RED    = { .r = 255, .g = 0,   .b = 0};

TARGET_COLOUR TC_ORANGE = { .r = 255, .g = 165, .b = 0};
TARGET_COLOUR TC_LIGHTER_ORANGE = { .r = 255, .g = 213, .b = 128};

TARGET_COLOUR TC_YELLOW = { .r = 255, .g = 255, .b = 0};

TARGET_COLOUR TC_GREEN  = { .r = 0,   .g = 255, .b = 0};
TARGET_COLOUR TC_LIGHTER_GREEN = { .r = 100, .g = 2554, .b = 0};

TARGET_COLOUR TC_BLUE   = { .r = 0,   .g = 0,   .b = 255};
TARGET_COLOUR TC_LIGHTER_BLUE = { .r = 64, .g = 64, .b = 255};
TARGET_COLOUR TC_LIGHTEST_BLUE = { .r = 127, .g = 127, .b = 255};

TARGET_COLOUR TC_INDIGO = { .r = 200, .g = 0,   .b = 255};
TARGET_COLOUR TC_VIOLET = { .r = 255, .g = 0,   .b = 255};

TARGET_COLOUR TC_WHITE = { .r = 255, .g = 255, .b = 255 };

TARGET_COLOUR COLOUR_WHEEL[6] = { TC_RED, TC_ORANGE, TC_YELLOW, TC_GREEN, TC_BLUE, TC_VIOLET};
