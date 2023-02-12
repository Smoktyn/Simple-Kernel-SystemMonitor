#include <iostream>
#include <Windows.h>
#include "../SysMon/Common.h"
HANDLE hConsole;

void DisplayTime(const LARGE_INTEGER& time) {
	SYSTEMTIME st;
	::FileTimeToSystemTime((FILETIME*)&time, &st);
	printf("%02d:%02d:%02d.%03d: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void DisplayInfo(BYTE* buffer, DWORD size) {
	auto count = size;
	while (count > 0) {
		auto header = (ItemHeader*)buffer;
		switch (header->Type) {
		case ItemType::ProcessExit:
		{
			SetConsoleTextAttribute(hConsole, 12);
			DisplayTime(header->Time);
			auto info = (ProcessExitInfo*)buffer;
			printf("[!] Process");
			SetConsoleTextAttribute(hConsole, 14);
			printf(" %d ", info->ProcessId);
			SetConsoleTextAttribute(hConsole, 12);
			printf("Exited\n\n");
			SetConsoleTextAttribute(hConsole, 7);
			break;
		}

		case ItemType::ProcessCreate:
		{
			SetConsoleTextAttribute(hConsole, 11);
			DisplayTime(header->Time);
			auto info = (ProcessCreateInfo*)buffer;
			std::wstring commandline((WCHAR*)(buffer + info->CommandLineOffset), info->CommandLineLength);
			std::wstring imageName((WCHAR*)(buffer + info->ImageNameOffset), info->ImageNameLenght);
			printf("[+] Process");
			SetConsoleTextAttribute(hConsole, 14);
			printf(" %d ", info->ProcessId);
			SetConsoleTextAttribute(hConsole, 11);
			printf("Created.\n");
			printf("[+] Command line -> %ws\n", commandline.c_str());
			printf("[+] Image name -> %ws\n\n", imageName.c_str());
			SetConsoleTextAttribute(hConsole, 7);
			break;
		}
		default:
			break;
		}
		buffer += header->Size;
		count -= header->Size;
	}

}
int main() {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	auto hHandle = CreateFileW(L"\\\\.\\sysmon", GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hHandle == INVALID_HANDLE_VALUE) {
		std::cout << "Create File Error\n";
		return 1;
	}
	BYTE buffer[1 << 16];
	while (true) {
		DWORD bytes;
		if (!::ReadFile(hHandle, buffer, sizeof(buffer), &bytes, nullptr)) {
			return 1;
		}
		if (bytes != 0) {
			DisplayInfo(buffer, bytes);
		}
		::Sleep(200);
	}
	return 0;
}
