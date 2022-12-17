#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "keys.hpp"
#include "wireless.hpp"

#include "settings.h"

struct Settings settings;

struct Settings defaultSettings = {
	.port       = 8889,
	.throttle   = 20,
	.circlePad  = joystick1,
	.cStick     = joystick2,
	.touch      = mouse,
	.mouseSpeed = 4,
	.A          = { 1, {'A'       } },
	.B          = { 1, {'B'       } },
	.X          = { 1, {'X'       } },
	.Y          = { 1, {'Y'       } },
	.L          = { 1, {'L'       } },
	.R          = { 1, {'R'       } },
	.ZL         = { 1, {'Q'       } },
	.ZR         = { 1, {'W'       } },
	.Left       = { 1, {VK_LEFT   } },
	.Right      = { 1, {VK_RIGHT  } },
	.Up         = { 1, {VK_UP     } },
	.Down       = { 1, {VK_DOWN   } },
	.Start      = { 1, {VK_RETURN } },
	.Select     = { 1, {VK_BACK   } },
	.Tap        = { 1, {'T'       } },
};

static bool getSetting( char *name, char *src, char *dest ) {
	char *start = strstr(src, name);
	
	if ( start ) {
		start += strlen(name);
		
		char *end = start + strlen(start);
		if ( strstr( start, "\n" ) - 1 < end )
			end = strstr( start, "\n" ) - 1;
		size_t size = (size_t)end - (size_t)start;
		
		strncpy(dest, start, size);
		dest[size] = '\0';
		
		return true;
	}
	
	return false;
}

static struct KeyMapping getButton( char* string ) {
	struct KeyMapping k = { 1, { 0} };
	
	k.useJoypad = 0;
	// "SPECIAL" KEYS
	if      ( strcmp( string, "SPACE"       ) == 0 ) k.virtualKey = VK_SPACE;
	else if ( strcmp( string, "CLICK"       ) == 0 ) k.virtualKey = VK_LBUTTON;
	else if ( strcmp( string, "RIGHT CLICK" ) == 0 ) k.virtualKey = VK_RBUTTON;
	else if ( strcmp( string, "ENTER"       ) == 0 ) k.virtualKey = VK_RETURN;
	else if ( strcmp( string, "BACKSPACE"   ) == 0 ) k.virtualKey = VK_BACK;
	else if ( strcmp( string, "SHIFT"       ) == 0 ) k.virtualKey = VK_SHIFT;
	else if ( strcmp( string, "TAB"         ) == 0 ) k.virtualKey = VK_TAB;
	else if ( strcmp( string, "LEFT"        ) == 0 ) k.virtualKey = VK_LEFT;
	else if ( strcmp( string, "RIGHT"       ) == 0 ) k.virtualKey = VK_RIGHT;
	else if ( strcmp( string, "UP"          ) == 0 ) k.virtualKey = VK_UP;
	else if ( strcmp( string, "DOWN"        ) == 0 ) k.virtualKey = VK_DOWN;
	else if ( strcmp( string, "PAGE UP"     ) == 0 ) k.virtualKey = VK_PRIOR;
	else if ( strcmp( string, "PAGE DOWN"   ) == 0 ) k.virtualKey = VK_NEXT;
	else if ( strcmp( string, "META"        ) == 0 ) k.virtualKey = VK_LWIN;
	else if ( strcmp( string, "NONE"        ) == 0 ) k.virtualKey = 0;
	// JOYPAD STUFF
	else if ( strcmp( string, "JOY1"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 0; }
	else if ( strcmp( string, "JOY2"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 1; }
	else if ( strcmp( string, "JOY3"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 2; }
	else if ( strcmp( string, "JOY4"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 3; }
	else if ( strcmp( string, "JOY5"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 4; }
	else if ( strcmp( string, "JOY6"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 5; }
	else if ( strcmp( string, "JOY7"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 6; }
	else if ( strcmp( string, "JOY8"  ) == 0 ) { k.useJoypad = 1; k.joypadButton = 1 << 7; }
	else if ( strcmp( string, "JOY9"  ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 0; }
	else if ( strcmp( string, "JOY10" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 1; }
	else if ( strcmp( string, "JOY11" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 2; }
	else if ( strcmp( string, "JOY12" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 3; }
	else if ( strcmp( string, "JOY13" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 4; }
	else if ( strcmp( string, "JOY14" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 5; }
	else if ( strcmp( string, "JOY15" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 6; }
	else if ( strcmp( string, "JOY16" ) == 0 ) { k.useJoypad = 2; k.joypadButton = 1 << 7; }
	// LETTERS AND NUMBERS ( somehow )
	else k.virtualKey = (int) string[0];
	
	return k;
}

bool readSettings( void ) {
	FILE *f;
	size_t len;
	char *data = NULL;
	
	memcpy(  &settings, &defaultSettings, sizeof( struct Settings ) );
	
	f = fopen( "config.ini", "rb" );
	if(! f )
		return false;
	
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	rewind( f );

	data = malloc( len);
	if(! data ) {
		fclose(f);
		return false;
	}
	
	fread( data, 1, len, f );
	
	char setting[64] = { '\0' };
	
	if ( getSetting( "Port: ", data, setting ) )
		sscanf( setting, "%d", &settings.port );
	
	if ( getSetting( "Throttle: ", data, setting ) )
		sscanf( setting, "%d", &settings.throttle );
	
	if ( getSetting( "Circle Pad: ", data, setting ) ) {
		if ( strcmp( setting, "MOUSE" ) == 0 ) settings.circlePad = mouse;
		else if ( strcmp( setting, "JOYSTICK1" ) == 0 ) settings.circlePad = joystick1;
		else if ( strcmp( setting, "JOYSTICK2" ) == 0 ) settings.circlePad = joystick2;
	}
	
	if ( getSetting( "C Stick: ", data, setting ) ) {
		if ( strcmp( setting, "MOUSE" ) == 0 ) settings.cStick = mouse;
		else if ( strcmp( setting, "JOYSTICK1" ) == 0 ) settings.cStick = joystick1;
		else if ( strcmp( setting, "JOYSTICK2" ) == 0 ) settings.cStick = joystick2;
	}
	
	if ( getSetting( "Touch: ", data, setting ) ) {
		if ( strcmp( setting, "MOUSE" ) == 0 ) settings.touch = mouse;
		else if ( strcmp( setting, "JOYSTICK1" ) == 0 ) settings.touch = joystick1;
		else if ( strcmp( setting, "JOYSTICK2" ) == 0 ) settings.touch = joystick2;
	}
	
	if ( getSetting( "Mouse Speed: ", data, setting ) )
		sscanf(setting, "%d", &settings.mouseSpeed);
	
	if ( getSetting( "A: "     , data, setting ) ) settings.A      = getButton( setting);
	if ( getSetting( "B: "     , data, setting ) ) settings.B      = getButton( setting);
	if ( getSetting( "X: "     , data, setting ) ) settings.X      = getButton( setting);
	if ( getSetting( "Y: "     , data, setting ) ) settings.Y      = getButton( setting);
	if ( getSetting( "L: "     , data, setting ) ) settings.L      = getButton( setting);
	if ( getSetting( "R: "     , data, setting ) ) settings.R      = getButton( setting);
	if ( getSetting( "ZL: "    , data, setting ) ) settings.ZL     = getButton( setting);
	if ( getSetting( "ZR: "    , data, setting ) ) settings.ZR     = getButton( setting);
	if ( getSetting( "Left: "  , data, setting ) ) settings.Left   = getButton( setting);
	if ( getSetting( "Right: " , data, setting ) ) settings.Right  = getButton( setting);
	if ( getSetting( "Up: "    , data, setting ) ) settings.Up     = getButton( setting);
	if ( getSetting( "Down: "  , data, setting ) ) settings.Down   = getButton( setting);
	if ( getSetting( "Start: " , data, setting ) ) settings.Start  = getButton( setting);
	if ( getSetting( "Select: ", data, setting ) ) settings.Select = getButton( setting);
	if ( getSetting( "Tap: "   , data, setting ) ) settings.Tap    = getButton( setting);
	
	fclose(f);
	
	return true;
}
