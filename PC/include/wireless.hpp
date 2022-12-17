#pragma once

#ifndef WINVER
	#define WINVER 0x0500
#endif

#include <windows.h>
#include <winsock.h>

#include <stddef.h>

#define SCREENSHOT_CHUNK 4000

#define IP INADDR_ANY

enum NET_COMMANDS {
	CONNECT,
	KEYS,
	SCREENSHOT,
};

// It is deliberately set up to have an anonymous struct as well as a named struct for convenience, not a mistake!
struct Packet {
	NET_COMMANDS command;
	unsigned char keyboardActive;

	union {
		// CONNECT
		struct ConnectPacket { } connectPacket;
		
		// KEYS
		struct KeysPacket {
			unsigned int keys;
			struct { short x; short y; } circlePad;
			struct { unsigned short x; unsigned short y; } touch;
			struct { short x; short y; } cStick;
		} keysPacket;
		
		// SCREENSHOT
		struct ScreenshotPacket {
			unsigned short offset;
			unsigned char data[SCREENSHOT_CHUNK];
		} screenshotPacket;
	};
};

extern SOCKET listener;
extern SOCKET client;

extern struct sockaddr_in client_in;

extern int sockaddr_in_sizePtr;

extern struct Packet buffer;
extern char hostName[80];

void initNetwork(void);
void printIPs(void);
void startListening(void);
void sendBuffer(int length);
int receiveBuffer(int length);

void sendScreenshot(void);
