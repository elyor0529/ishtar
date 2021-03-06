// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#pragma comment(lib, "mscoree.lib")


// Load and execute managed binary in default AppDomain.
//
// @param arg -- String in format <assemblyPath>;<typeName>;<methodName>;<argument>
// @return ExecuteInDefaultAppDomain exit code.
__declspec(dllexport) HRESULT LoadManagedCode(_In_ LPCTSTR arg)
{
	OutputDebugStringA("[Ishtar][InjectDLL] Loading Ishtar...");
	HRESULT result;
	
	ICLRMetaHost *metaHost = NULL;
	result = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metaHost);
	
	if (FAILED(result))
	{
		OutputDebugStringA("[Ishtar][InjectDLL] Error - Could not get CLR instane.");
		return result;
	}

	OutputDebugStringA("[Ishtar][InjectDLL] Success - CLR instance created.");

	OutputDebugStringA("[Ishtar][InjectDLL] Enumerating loaded runtimes...");
	HANDLE process = GetCurrentProcess();
	IEnumUnknown *runtimes = NULL;
	result = metaHost->EnumerateLoadedRuntimes(process, &runtimes);
	if (FAILED(result))
	{
		OutputDebugStringA("[Ishtar][InjectDLL] Error - Could not enumerate runtimes.");
		return result;
	}

	OutputDebugStringA("[Ishtar][InjectDLL] Success - Runtimes enumerated.");

	ICLRRuntimeInfo *info = NULL;
	ULONG fetched = 0;
	while (S_OK == (result = runtimes->Next(1, (IUnknown **)&info, &fetched)) && 1 == fetched){
		DWORD size = 256;
		LPWSTR buf = new WCHAR[256];
		result = info->GetVersionString(buf, &size);
				
		if (FAILED(result))
		{
			OutputDebugStringA("[Ishtar][InjectDLL] Error - Could not get CLR version.");
			return result;
		}

		std::wstring prefix = L"[Ishtar][InjectDLL] Runtime found. Version: ";
		std::wstring version(buf);
		std::wstring message = prefix + version;

		OutputDebugStringW(message.c_str());
		delete[] buf;

		ICLRRuntimeHost *clrRuntimeHost = NULL;
		result = info->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&clrRuntimeHost));
		if (FAILED(result))
		{
			OutputDebugStringA("[Ishtar][InjectDLL] Error - Could not get CLRRuntimeHost interface.");
			return result;
		}

		std::wstring params(arg);

		std::wstring assemblyPath = params.substr(0, params.find(L";"));
		params.erase(0, params.find(L";") + 1);

		std::wstring typeName = params.substr(0, params.find(L";"));
		params.erase(0, params.find(L";") + 1);
		
		std::wstring methodName = params.substr(0, params.find(L";"));
		params.erase(0, params.find(L";") + 1);

		std::wstring argument = params.length() > 0 ? params : L"";

		DWORD retVal;
		
		result = clrRuntimeHost->ExecuteInDefaultAppDomain(assemblyPath.c_str(), typeName.c_str(), 
														   methodName.c_str(), argument.c_str(), &retVal);
		if (FAILED(result))
		{
			OutputDebugStringA("[Ishtar][InjectDLL] Error - Could not execute code in default app domain.");
			return result;
		}

		clrRuntimeHost->Release();
	}
	
	// Cleanup and exit.
	// Manged code should be injected already.
	info->Release();
	runtimes->Release();
	metaHost->Release();

	return result;
}


// Library entry point.
// Do not run any code here as it might deadlock.
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

