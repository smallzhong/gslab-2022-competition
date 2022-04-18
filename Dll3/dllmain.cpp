// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <math.h>

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
	float x, y, z;
}v3, * pv3;

typedef struct _v4
{
	float r, g, b, a;
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
	static bool flag = false; if (!flag) { flag = true; A("inline hook成功！"); }

	float ygap = 0.118343, xgap = 0.064935; // 同一格子内部距离
	float ybet = 0.023669, xbet = 0.012987; // 两个相邻格子之间的距离

	PInfo info = (PInfo)(context->mRcx);

#define ORIGIN_A1_X -0.9
#define ORIGIN_A1_Y 0.882840

	static int ct = 0;
	ct++;
	if (ct <= 6)
	{
		float a1x = ORIGIN_A1_X;
		float a1y = ORIGIN_A1_Y - (ct - 1) * (ygap + ybet);
		info->a1.x = a1x, info->a1.y = a1y;
		info->a2.x = a1x + xgap, info->a2.y = a1y;
		info->a3.x = a1x, info->a3.y = a1y - ygap;
		info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
	}
	else if (ct <= 9)
	{
		float a1x = ORIGIN_A1_X + (ct - 6) * (xgap + xbet);
		float a1y = ORIGIN_A1_Y - (4 - 1) * (ygap + ybet);
		info->a1.x = a1x, info->a1.y = a1y;
		info->a2.x = a1x + xgap, info->a2.y = a1y;
		info->a3.x = a1x, info->a3.y = a1y - ygap;
		info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
	}
	else if (ct == 10)
	{
		float a1x = ORIGIN_A1_X + (7 - 6) * (xgap + xbet);
		float a1y = ORIGIN_A1_Y - (2 - 1) * (ygap + ybet);
		info->a1.x = a1x, info->a1.y = a1y;
		info->a2.x = a1x + xgap, info->a2.y = a1y;
		info->a3.x = a1x, info->a3.y = a1y - ygap;
		info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
	}
	else if (ct == 11)
	{
		float a1x = ORIGIN_A1_X + (8 - 6) * (xgap + xbet);
		float a1y = ORIGIN_A1_Y - (3- 1) * (ygap + ybet);
		info->a1.x = a1x, info->a1.y = a1y;
		info->a2.x = a1x + xgap, info->a2.y = a1y;
		info->a3.x = a1x, info->a3.y = a1y - ygap;
		info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
	}

	if (ct == 42) ct = 0;

	static int TT = 0;
	static Info last_info = { 0 };
	if (TT < 42)
	{
		TT++;
		if (TT == 12) A("");
		A("a1.x = %f a1.y = %f a2.x = %f a2.y = %f a3.x = %f a3.y = %f a4.x = %f a4.y = %f",
			info->a1.x, info->a1.y,
			info->a2.x, info->a2.y,
			info->a3.x, info->a3.y,
			info->a4.x, info->a4.y);
		/*A("左上角坐标：%f %f，上下间距：%f 左右间距：%f\n", info->a1.x, info->a1.y, info->a1.y - info->a3.y, info->a2.x - info->a1.x);
		if (TT != 1)
		{
			A("左右间隔：%f 上下间隔：%f", info->a1.x - last_info.a1.x,
				last_info.a4.y - info->a2.y);
		}*/
		memcpy(&last_info, info, sizeof(Info)); // 备份
	}
}

__declspec(dllexport) DWORD WINAPI mythread(LPVOID lpParameter)
{
	A("in mythread\n");
	PULONG64 saddr = (PULONG64)0x140008308;
	while (*saddr == 0); // 等待shellcode被申请成功
	A("saddr = %llx\n", *saddr);

	// 在shellcode+30e的地方下hook
	PUCHAR t = (PUCHAR)((*saddr) + 0x30e);
	g_origin = (ULONG64)((*saddr) + 0x31c); // 需要跳回的地方

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
	A("memcpy完成");

	return 0;
}



__declspec(dllexport) BOOL APIENTRY DllMain(HMODULE hModule,
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

