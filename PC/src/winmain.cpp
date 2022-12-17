//
// Created by ENDERZOMBI102 on 17/12/2022.
//

#include <windows.h>
#include <iostream>

// The ViGEm API
#include <ViGEm/Client.h>
// Link against SetupAPI
#pragma comment( lib, "setupapi.lib" )

#include "general.h"
#include "settings.h"
#include "wireless.hpp"
#include "keyboard.hpp"
#include "keys.hpp"


int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow ) {
	std::cout << "3DS Controller Server " << VERSION << std::endl;

	double widthMultiplier = GetSystemMetrics( SM_CXSCREEN ) / 320.0; // screen width / n3ds screens to get aspect ratio
	double heightMultiplier = GetSystemMetrics( SM_CYSCREEN ) / 240.0; // screen height / n3ds screens to get aspect ratio

	if(! readSettings() )
		std::cout << "Couldn't read settings file, using default key bindings." << std::endl;

	initNetwork();

	std::cout << "Port: " << settings.port << "\nRunning on: " << hostName <<"Your local IP(s):" << std::endl;
	printIPs();

	printf("\n");


	const auto client = vigem_alloc();

	if ( client == nullptr ) {
		std::cerr << "Uh, not enough memory to do that?!" << std::endl;
		return -1;
	}

	const auto retval = vigem_connect(client);

	if (! VIGEM_SUCCESS(retval) ) {
		std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
		return -1;
	}

	//
	// Allocate handle to identify new pad
	//
	const auto pad = vigem_target_x360_alloc();

	//
	// Add client to the bus, this equals a plug-in event
	//
	const auto pir = vigem_target_add(client, pad);

	//
	// Error handling
	//
	if (! VIGEM_SUCCESS( pir ) ) {
		std::cerr << "Target plugin failed with error code: 0x" << std::hex << pir << std::endl;
		return -1;
	}
	Packet pPacket{};
	XUSB_REPORT report{};

	startListening();
	while ( true ) {
		memset( &pPacket, 0, sizeof(Packet) );

		while ( receiveBuffer( sizeof(Packet) ) <= 0 ) {
			// Waiting
			Sleep( settings.throttle );
		}

		keyboardActive = pPacket.keyboardActive;

		switch( pPacket.command ) {
			case CONNECT:
				lastKeys = 0;
				currentKeys = 0;
				circlePad.x = 0;
				circlePad.y = 0;
				lastTouch.x = 0;
				lastTouch.y = 0;
				currentTouch.x = 0;
				currentTouch.y = 0;
				cStick.x = 0;
				cStick.y = 0;

				pPacket.command = CONNECT;
				printf("3DS Connected!\n");

				Sleep(50);
				sendBuffer(1);

				Sleep(50);
				sendBuffer(1);

				Sleep(50);
				sendBuffer(1);
				break;

			case KEYS:
				lastKeys = currentKeys;
				if ( currentKeys & KEY_TOUCH )
					lastTouch = currentTouch;

				memcpy( &currentKeys , &pPacket.keysPacket.keys     , 4 );
				memcpy( &circlePad   , &pPacket.keysPacket.circlePad, 4 );
				memcpy( &currentTouch, &pPacket.keysPacket.touch    , 4 );
				memcpy( &cStick      , &pPacket.keysPacket.cStick   , 4 );

				handleKey( KEY_A     , settings.A      );
				handleKey( KEY_B     , settings.B      );
				handleKey( KEY_SELECT, settings.Select );
				handleKey( KEY_START , settings.Start  );
				handleKey( KEY_DRIGHT, settings.Right  );
				handleKey( KEY_DLEFT , settings.Left   );
				handleKey( KEY_DUP   , settings.Up     );
				handleKey( KEY_DDOWN , settings.Down   );
				handleKey( KEY_R     , settings.R      );
				handleKey( KEY_L     , settings.L      );
				handleKey( KEY_ZR    , settings.ZR     );
				handleKey( KEY_ZL    , settings.ZL     );
				handleKey( KEY_X     , settings.X      );
				handleKey( KEY_Y     , settings.Y      );

				if( newpress( KEY_TOUCH ) ) {
					lastTouch.x = currentTouch.x;
					lastTouch.y = currentTouch.y;
				}

				if( ( currentKeys & KEY_TOUCH ) ) {
					if(keyboardActive) {
						if ( newpress( KEY_TOUCH ) ) {
							char letter = currentKeyboardKey();
							if(letter) {
								simulateKeyNewpress(letter);
								simulateKeyRelease(letter);
							}
						}
					} else if( settings.touch == mouse) {
						if ( settings.mouseSpeed ) {
							POINT p;
							GetCursorPos(&p);
							SetCursorPos(p.x + (currentTouch.x - lastTouch.x) * settings.mouseSpeed, p.y + ( currentTouch.y - lastTouch.y) * settings.mouseSpeed);
						} else
							SetCursorPos((int)((double)currentTouch.x * widthMultiplier), (int)((double)currentTouch.y * heightMultiplier));
					} else if( settings.touch == joystick1) {
						report.sThumbLX = currentTouch.x * 128;
						report.sThumbLY = currentTouch.y * 128;
					} else if( settings.touch == joystick2) {
						report.sThumbRX = currentTouch.x * 128;
						report.sThumbRY = currentTouch.y * 128;
					} else
						handleKey( KEY_TOUCH, settings.Tap );
				}

				if( settings.circlePad == mouse) {
					if( abs( pPacket.keysPacket.circlePad.x ) < settings.mouseSpeed * 3 )
						pPacket.keysPacket.circlePad.x = 0;
					if( abs( pPacket.keysPacket.circlePad.y ) < settings.mouseSpeed * 3 )
						pPacket.keysPacket.circlePad.y = 0;

					POINT p;
					GetCursorPos(&p);
					SetCursorPos(p.x + ( pPacket.keysPacket.circlePad.x * settings.mouseSpeed ) / 32, p.y - ( pPacket.keysPacket.circlePad.y * settings.mouseSpeed ) / 32 );
				} else if( settings.circlePad == joystick1) {
					report.sThumbLX = ( pPacket.keysPacket.circlePad.x + 128) * 128;
					report.sThumbLY = ( 128 - pPacket.keysPacket.circlePad.y) * 128;
				} else if( settings.circlePad == joystick2) {
					report.sThumbRX = ( pPacket.keysPacket.circlePad.x + 128) * 128;
					report.sThumbRY = ( 128 - pPacket.keysPacket.circlePad.y) * 128;
				}

				if( settings.cStick == mouse) {
					if( abs( pPacket.keysPacket.cStick.x ) < settings.mouseSpeed * 3 )
						pPacket.keysPacket.cStick.x = 0;
					if( abs( pPacket.keysPacket.cStick.y ) < settings.mouseSpeed * 3 )
						pPacket.keysPacket.cStick.y = 0;

					POINT p;
					GetCursorPos(&p);
					SetCursorPos(p.x + ( pPacket.keysPacket.cStick.x * settings.mouseSpeed ) / 32, p.y - ( pPacket.keysPacket.cStick.y * settings.mouseSpeed ) / 32 );
				} else if( settings.cStick == joystick1) {
					report.sThumbLX = ( pPacket.keysPacket.cStick.x + 128) * 128;
					report.sThumbLY = ( 128 - pPacket.keysPacket.cStick.y) * 128;
				} else if( settings.cStick == joystick2) {
					report.sThumbRX = ( pPacket.keysPacket.cStick.x + 128) * 128;
					report.sThumbRY = ( 128 - pPacket.keysPacket.cStick.y) * 128;
				}
				break;
			case SCREENSHOT:
				break;
		}

		vigem_target_x360_update( client, pad, report );
	}

	//
	// We're done with this pad, free resources (this disconnects the virtual device)
	//
	vigem_target_remove( client, pad );
	vigem_target_free( pad );
}
