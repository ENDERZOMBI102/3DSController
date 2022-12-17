add_executable(
	WinExec
	src/general.cpp
	include/general.h
	src/joystick.cpp
	include/joystick.hpp
	src/keyboard.cpp
	include/keyboard.hpp
	src/keys.cpp
	include/keys.hpp
	src/settings.cpp
	include/settings.h
	src/wireless.cpp
	include/wireless.hpp
	src/winmain.cpp)
include_directories( include )

target_link_libraries( WinExec PRIVATE ViGEmClient )
target_include_directories( WinExec PRIVATE ViGEmClient )
target_compile_options( WinExec PRIVATE /std:c++20 )