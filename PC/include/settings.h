#pragma once

#include <stdbool.h>

#include "keys.hpp"

enum Analogue {
	mouse,
	joystick1,
	joystick2,
};

struct Settings {
	int port;
	int throttle;
	enum Analogue circlePad;
	enum Analogue cStick;
	enum Analogue touch;
	int mouseSpeed;
	struct KeyMapping A, B, X, Y, L, R, ZL, ZR, Left, Right, Up, Down, Start, Select, Tap;
};

extern struct Settings settings;
extern struct Settings defaultSettings;

bool readSettings(void);
