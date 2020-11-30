#include <stdio.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <functional>

#define DLL_IMPORT __declspec(dllimport)
#define STR "Original"
#define ARGS DWORD pid, const char* data, const char* replacement

const char FUNCNAME[] = "ReplaceStr";
const char SEARCHSTR[] = "Original";
const char REPLACESTR[] = "Replaced";
const char DLLPATH[] = "Dll.dll";

typedef HMODULE(WINAPI* LPLoadLibrary)(LPCSTR);
typedef HMODULE(WINAPI* LPGetProcAddress)(HMODULE, LPCSTR);
typedef void __stdcall TReplaceData(ARGS);
extern "C" void DLL_IMPORT __stdcall ReplaceStr(ARGS);


int InjectDynamically(DWORD pid);
int InjectStatically(DWORD pid);
void InjectIntoProcess(DWORD PID);

int main()
{
	const char str1[] = STR;
	DWORD pid = GetCurrentProcessId();	
	printf("%u\n", pid);
	printf("Choose a way to inject DLL\n 1. static injection\n 2. dynamic injection\n 3. inject in process\n");

	char a = getchar();

	switch (a)
	{
	case '1':
		InjectStatically(pid);
		printf("static injection\n");
		break;
	case '2':
		InjectDynamically(pid);
		printf("dynamic injection\n");
		break;
	case '3':
		DWORD iPid;
		printf("Enter pid\n");
		scanf_s("%d", &iPid);
		InjectIntoProcess(iPid);		
		break;
	default:
		("incorrect number");
		break;
	}
	printf("String \"%s\" where replaced with \"%s\"\n", STR, str1);	

	return 0;
}

int makeCall(TReplaceData func, DWORD pid) {
	if (func != NULL)
	{
		func(pid, SEARCHSTR, REPLACESTR);
		return 1;
	}
	else
	{
		puts("DLL not found.");
		return 0;
	}
}

int InjectDynamically(DWORD pid)
{
	HMODULE hDll;
	if ((hDll = LoadLibraryA(DLLPATH)) == NULL) return 1; 
	auto* func = (TReplaceData*)GetProcAddress(hDll, FUNCNAME);
	FreeLibrary(hDll);
	return makeCall(func, pid);
}

int InjectStatically(DWORD pid)
{
	TReplaceData* func = (TReplaceData*)ReplaceStr;
	return makeCall(func, pid);
}

void InjectIntoProcess(DWORD pid) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (process)
	{
		HMODULE hKernel = LoadLibraryA("kernel32.dll");
		if (hKernel)
		{
			auto loadLibrary = (LPLoadLibrary)GetProcAddress(hKernel, "LoadLibraryA");
			LPVOID remoteString = VirtualAllocEx(process, NULL, strlen(DLLPATH) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (remoteString)
			{
				WriteProcessMemory(process, remoteString, DLLPATH, strlen(DLLPATH) + 1, NULL);
				HANDLE hThread = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)loadLibrary, remoteString, NULL, NULL);

				if (hThread == NULL) {
					printf("Error creating remote thread.\n");
				}
				else {
					WaitForSingleObject(hThread, INFINITE);
					printf("DLL were injected!\n");
					CloseHandle(hThread);
				}
			}
			else
			{
				printf("Error allocating memory for remote string.\n");
			}
		}
		else
		{
			printf("Error loading kernel32.dll.\n");
		}
			FreeLibrary(hKernel);
			CloseHandle(process);
	}	
}