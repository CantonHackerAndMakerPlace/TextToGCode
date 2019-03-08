#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#ifndef SERIALPORT_H
#define SERIALPORT_H

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 255

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

class SerialPort
{
private:
	HANDLE handler;
	bool connected;
	COMSTAT status;
	DWORD errors;
public:
	SerialPort(const char*);
	~SerialPort();

	int readSerialPort(char *buffer, unsigned int buf_size);
	bool writeSerialPort(const char *buffer, unsigned int buf_size);
	bool isConnected();

	static void GetPortNames(std::vector<std::string>& ports) {

		TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
		DWORD test;
		bool gotPort = 0; // in case the port is not found

		for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
		{
			char buffer[22];
			sprintf(buffer, "COM%d", i);

			test = QueryDosDevice(buffer, (LPSTR)lpTargetPath, 5000);

			// Test the return value and error if any
			if (test != 0) //QueryDosDevice returns zero if it didn't find an object
			{
				char buffer2[22];
				sprintf(buffer2, "\\\\.\\COM%d", i);
				ports.push_back(buffer2);
			}

			if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
				continue;
			}
		}
	}
};

#endif // SERIALPORT_H