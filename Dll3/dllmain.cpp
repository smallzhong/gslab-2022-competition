// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

EXTERN_C ULONG64 g_origin = 0;

void DbgPrint(const TCHAR* format, ...)
{

	TCHAR buf[4096];

	va_list args;
	va_start(args, format);
	int len = _vstprintf(buf, format, args);
	va_end(args);


	OutputDebugString(buf);
}
void DbgPrint(const char* format, ...)
{
	//OutputDebugStringA("[zyc]:");
	char buf[4096], * p = buf;
	va_list args;
	va_start(args, format);
	p += _vsnprintf(p, sizeof buf - 1, format, args);
	va_end(args);
	OutputDebugStringA(buf);
}

#define A(format, ...) DbgPrint ("[zyc]:"##format, __VA_ARGS__)

EXTERN_C VOID AsmVmxExitHandler();
EXTERN_C VOID myint3();

typedef struct _v3
{
	float x,y,z;
}v3, * pv3;

typedef struct _v4
{
	float r,g,b,a;
}v4, * pv4;

typedef struct _Info
{
	v3 a1;
	v4 b1;	

	v3 a2;
	v4 b2;	

	v3 a3;
	v4 b3;	

	v3 a4;
	v4 b4;
} Info, * PInfo;

EXTERN_C VOID VmxExitHandler(PGuestContext context)
{
	static bool flag = false; if (!flag) { flag = true; DbgPrint("inline hook成功！"); }

	PInfo info = (PInfo)(context->mRcx);
	
	if (info->a1.x < 0) info->a1.x = -info->a1.x;
	if (info->a2.x < 0) info->a2.x = -info->a2.x;
	if (info->a3.x < 0) info->a3.x = -info->a3.x;
	if (info->a4.x < 0) info->a4.x = -info->a4.x;

	if (info->a1.y < 0) info->a1.y = -info->a1.y;
	if (info->a2.y < 0) info->a2.y = -info->a2.y;
	if (info->a3.y < 0) info->a3.y = -info->a3.y;
	if (info->a4.y < 0) info->a4.y = -info->a4.y;
}

DWORD WINAPI mythread(LPVOID lpParameter)
{
	A("in mythread\n");
	PULONG64 saddr = (PULONG64)0x140008308;
	while (*saddr == 0); // 等待shellcode被申请成功
	A("saddr = %llx\n", *saddr);

	// 在shellcode+30e的地方下hook
	PUCHAR t = (PUCHAR)((*saddr) + 0x30e);
	g_origin = (ULONG64)((*saddr) + 0x322); // 需要跳回的地方

	char bufcode[] = {
					0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00,
	};

	//*(PULONG64)&bufcode[6] = (ULONG64)VmxExitHandler;
	*(PULONG64)&bufcode[6] = (ULONG64)AsmVmxExitHandler;
	//*(PULONG64)&bufcode[7] = (ULONG64)VmxExitHandler;

	DWORD old_protect = 0;
	VirtualProtect(t, 0x1000, PAGE_EXECUTE_READWRITE, &old_protect);
	A("改变内存属性成功");

	A("AsmVmxExitHandler = %llx\r\n", AsmVmxExitHandler);
	memcpy(t, bufcode, sizeof bufcode);
	A("第二次成功！");

	return 0;
}



BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE t = ::CreateThread(NULL, 0, mythread, NULL, 0, NULL);
		break;
	}
	break;
	case DLL_THREAD_ATTACH:
	{
	}
	break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

