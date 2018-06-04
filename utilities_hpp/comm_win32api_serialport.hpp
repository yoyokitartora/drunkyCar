#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<vector>
#include<thread>

class SerialPort
{
public:
	SerialPort(std::string name, int baudrate, int bytesize, int stopbits, int paritymode) {
		open(name, baudrate, bytesize, stopbits, paritymode);
		start();
	}
	SerialPort() {};
	bool open(std::string name, int baudrate, int bytesize, int stopbits, int paritymode) {
		portname = name;
		hserialport = CreateFileA(portname.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hserialport == INVALID_HANDLE_VALUE) {
			std::cout << "Open serialport " << portname << " failed." << std::endl;
			return false;
		}
		if (PurgeComm(hserialport, PURGE_TXCLEAR | PURGE_RXCLEAR) == FALSE) {
			std::cout << "Clear serialport " << portname << " error: " << GetLastError() << std::endl;
			return false;
		}
		return setup(baudrate, bytesize, stopbits, paritymode);
	}
	/* e.g. auto setup_success = setup(921600, 8, ONESTOPBIT, NOPARITY); */
	bool setup(int baudrate, int bytesize, int stopbits, int paritymode) {
		DCB dcb;
		memset(&dcb, 0, sizeof(dcb));
		GetCommState(hserialport, &dcb);
		dcb.BaudRate = baudrate;
		dcb.ByteSize = bytesize;
		dcb.StopBits = stopbits;
		dcb.fParity = paritymode ? TRUE : FALSE;
		dcb.Parity = paritymode;
		if (SetCommState(hserialport, &dcb) == FALSE) {
			std::cout << "Set serialport " << portname << " error: " << GetLastError() << std::endl;
			return false;
		}
		return true;
	}
	bool start() {
		if (is_opened() == false) return false;
		listener_busy = true;
		std::thread listener(&SerialPort::thread_listen, this, this);
		listener.detach();
	}
	void setCommCallback(void(*handler_function)(byte* bytes, size_t size)) {
		handler = handler_function;
	}
	void stop() {
		listener_busy = false;
	}
	bool operator << (std::vector<unsigned char> bytes) {
		return write(bytes.data(), bytes.size());
	}
	bool operator << (std::string str) {
		return write(str.data(), str.size());
	}
	std::string readline(char stop_char = '\n') {
		// 施工中
	}
	SerialPort& operator >> (std::vector<unsigned char> bytes) {
		DWORD dwErrorFlags, dwRead;
		byte buffer[1024];
		COMSTAT ComStat;
		ClearCommError(hserialport, &dwErrorFlags, &ComStat);
		if (ComStat.cbInQue == 0) return *this;
		ReadFile(hserialport, buffer, ComStat.cbInQue, &dwRead, NULL);
		bytes.reserve(dwRead);
		for (size_t i = 0; i < dwRead; i++)	bytes[i] = buffer[i];
		return *this;
	}
	SerialPort& operator >> (std::string) {
		// 施工中
		return *this;
	}
	bool write(const void *data, size_t size) {
		DWORD dwWriteBytes = 0;
		if (WriteFile(hserialport, data, size, &dwWriteBytes, NULL) == FALSE) {
			std::cout << "Write serialport " << portname << " error: " << GetLastError() << std::endl;
			return false;
		}
		return true;
	}
	bool is_opened() {
		return hserialport != INVALID_HANDLE_VALUE;
	}
	bool close() {
		listener_busy = false;
		Sleep(500);
		return is_opened() ? CloseHandle(hserialport) == TRUE : true;
	}
	~SerialPort() { close(); }
private:
	HANDLE hserialport = INVALID_HANDLE_VALUE;
	void(*handler)(byte* bytes, size_t size) = NULL;
	std::string portname;
	bool listener_busy = false;
	void thread_listen(void *instance) {
		std::cout << "Serialport " << portname << ": listener started." << std::endl;
		for (DWORD dwErrorFlags, dwRead; listener_busy && hserialport != INVALID_HANDLE_VALUE;) {
			byte buffer[1024];
			COMSTAT ComStat;
			ClearCommError(hserialport, &dwErrorFlags, &ComStat);
			if (ComStat.cbInQue == 0) { Sleep(1); continue; }
			ReadFile(hserialport, buffer, ComStat.cbInQue, &dwRead, NULL);
			if (handler) handler(buffer, dwRead);
		}
		std::cout << "Serialport " << portname << ": listener stopped." << std::endl;
	}
};
