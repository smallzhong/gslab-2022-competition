﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
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

	float ygap = 0.118343, xgap = 0.064935;
	PInfo info = (PInfo)(context->mRcx);

	static int ct = 0;
	ct++;
	if (ct == 1)
	{
		float a1x = -0.7, a1y = 0.8;
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
		/*A("a1.x = %f a1.y = %f a2.x = %f a2.y = %f a3.x = %f a3.y = %f a4.x = %f a4.y = %f",
			info->a1.x, info->a1.y,
			info->a2.x, info->a2.y,
			info->a3.x, info->a3.y,
			info->a4.x, info->a4.y);*/
		A("左上角坐标：%f %f，上下间距：%f 左右间距：%f\n", info->a1.x, info->a1.y, info->a1.y - info->a3.y, info->a2.x - info->a1.x);
		if (TT != 1)
		{
			A("左右间隔：%f 上下间隔：%f", info->a1.x - last_info.a1.x,
				last_info.a4.y - info->a2.y);
		}
		memcpy(&last_info, info, sizeof(Info)); // 备份
	}

	/*
		if (fabs(info->a1.x) > 1) info->a1.x -= (int)info->a1.x;
	if (fabs(info->a2.x) > 1) info->a2.x -= (int)info->a2.x;
	if (fabs(info->a3.x) > 1) info->a3.x -= (int)info->a3.x;
	if (fabs(info->a4.x) > 1) info->a4.x -= (int)info->a4.x;

	if (fabs(info->a1.y) > 1) info->a1.y -= (int)info->a1.y;
	if (fabs(info->a2.y) > 1) info->a2.y -= (int)info->a2.y;
	if (fabs(info->a3.y) > 1) info->a3.y -= (int)info->a3.y;
	if (fabs(info->a4.y) > 1) info->a4.y -= (int)info->a4.y;*/


		/*if (info->a1.x < 0) info->a1.x = -info->a1.x;
	if (info->a2.x < 0) info->a2.x = -info->a2.x;
	if (info->a3.x < 0) info->a3.x = -info->a3.x;
	if (info->a4.x < 0) info->a4.x = -info->a4.x;

	if (info->a1.y < 0) info->a1.y = -info->a1.y;
	if (info->a2.y < 0) info->a2.y = -info->a2.y;
	if (info->a3.y < 0) info->a3.y = -info->a3.y;
	if (info->a4.y < 0) info->a4.y = -info->a4.y;*/
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

