// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <process.h>
#include <vector>

#define DLL_EXPORT __declspec(dllexport)

using namespace std;

extern "C" void DLL_EXPORT __stdcall ReplaceStr(DWORD pid, const char* search, const char* replace)
{
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (process)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        MEMORY_BASIC_INFORMATION info;
        const size_t searchLength = sizeof(search);
        const size_t replaceLength = sizeof(replace);
		vector<char> chunk;
		char* p = 0;
		while (p < si.lpMaximumApplicationAddress)
		{
			if (VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info) && (info.State == MEM_COMMIT && info.AllocationProtect == PAGE_READWRITE))
			{
				p = (char*)info.BaseAddress;
				chunk.resize(info.RegionSize);
				SIZE_T bytesRead;
				if (ReadProcessMemory(process, p, &chunk[0], info.RegionSize, &bytesRead))
				{
					for (size_t i = 0; i < bytesRead; ++i)
					{
						if (memcmp(search, &chunk[i], searchLength) == 0)
						{
							char* ref = p + i;
							for (int j = 0; j < replaceLength; j++) {
								ref[j] = replace[j];
							}
							ref[replaceLength] = 0;
						}
					}
				}
			}
			p += info.RegionSize;
		}
    }
}

const char SEARCHSTR[] = "Original";
const char REPLACESTR[] = "Replaced";

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{	
	DWORD pid = _getpid();
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

    case DLL_THREAD_ATTACH:
		ReplaceStr(pid, SEARCHSTR, REPLACESTR);
		break;

    case DLL_THREAD_DETACH:	

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

