add_executable(
	WinExec
	src/general.c
	include/general.h
	src/joystick.c
	include/joystick.h
	src/keyboard.c
	include/keyboard.h
	src/keys.c
	include/keys.h
	src/main.c
	src/settings.c
	include/settings.h
	src/wireless.c
	include/wireless.h
)
include_directories( include )

target_link_libraries( WinExec PRIVATE ViGEmClient )
target_include_directories( WinExec PRIVATE ViGEmClient )