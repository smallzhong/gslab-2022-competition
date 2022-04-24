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

EXTERN_C VOID AsmHookHandler();
EXTERN_C VOID myint3();

typedef struct _v3
{
	float x, y, z;
}v3, * pv3;

typedef struct _v4
{
	DWORD r, g, b, a;
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

float getScreenZoom()
{
	static float fRes = 0;
	if (fRes != 0) return fRes;

	// 获取窗口当前显示的监视器
	HWND hWnd = GetDesktopWindow();//根据需要可以替换成自己程序的句柄 
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// 获取监视器逻辑宽度与高度
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	GetMonitorInfo(hMonitor, &miex);
	int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
	int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

	// 获取监视器物理宽度与高度
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;
	int cyPhysical = dm.dmPelsHeight;

	//缩放比例计算
	double horzScale = ((double)cxPhysical / (double)cxLogical);
	double vertScale = ((double)cyPhysical / (double)cyLogical);

	fRes = (float)horzScale;
	//返回缩放比
	A("fres = %f", fRes);
	return fRes;
}

EXTERN_C VOID HookHandler(PGuestContext context)
{
	static bool flag = false; if (!flag) { flag = true; A("inline hook成功！"); }

	float  xgap = 0.064935, ygap = 0.118343; // 同一格子内部距离
	float xbet = 0.012987, ybet = 0.023669; // 两个相邻格子之间的距离

	float trans = getScreenZoom() / 1.25;
	xgap *= trans, ygap *= trans, xbet *= trans, ybet *= trans; // 进行缩放

	float origin_a1_x = -0.9305;
	float origin_a1_y = 0.906692;
#define eps 1e-2
	if (fabs(getScreenZoom() - 1.00) < eps) origin_a1_y = 0.906692; // 缩放100%
	else if (fabs(getScreenZoom() - 1.25) < eps) origin_a1_y = 0.882840; // 缩放125%
	else if (fabs(getScreenZoom() - 1.50) < eps) origin_a1_y = 0.858773; // 缩放150%
	else if (fabs(getScreenZoom() - 1.75) < eps) origin_a1_y = 0.834448; // 缩放175%

	if (fabs(getScreenZoom() - 1.00) < eps) origin_a1_x = -0.9472; // 缩放100%
	else if (fabs(getScreenZoom() - 1.25) < eps) origin_a1_x = -0.9305;  // 缩放125%
	else if (fabs(getScreenZoom() - 1.50) < eps) origin_a1_x = -0.9236;  // 缩放150%
	else if (fabs(getScreenZoom() - 1.75) < eps) origin_a1_x = -0.9139;  // 缩放175%

	PInfo info = (PInfo)(context->mRcx);

	static int ct = 0;
	ct++;

	static int wrong_ct = 0;
	bool wrong_flag = false;

	if (info->a1.x < -1.0 || info->a1.x > 1.0 ||
		info->a2.x < -1.0 || info->a2.x > 1.0 ||
		info->a3.x < -1.0 || info->a3.x > 1.0 ||
		info->a4.x < -1.0 || info->a4.x > 1.0 ||

		info->a1.y < -1.0 || info->a1.y > 1.0 ||
		info->a2.y < -1.0 || info->a2.y > 1.0 ||
		info->a3.y < -1.0 || info->a3.y > 1.0 ||
		info->a4.y < -1.0 || info->a4.y > 1.0
		)
	{
		wrong_flag = true;
		wrong_ct++;
	}

	if (wrong_flag)
	{
		if (wrong_ct <= 6)
		{
			float a1x = origin_a1_x;
			float a1y = origin_a1_y - (wrong_ct - 1) * (ygap + ybet);
			info->a1.x = a1x, info->a1.y = a1y;
			info->a2.x = a1x + xgap, info->a2.y = a1y;
			info->a3.x = a1x, info->a3.y = a1y - ygap;
			info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
		}
		else if (wrong_ct <= 9)
		{
			float a1x = origin_a1_x + (wrong_ct - 6) * (xgap + xbet);
			float a1y = origin_a1_y - (4 - 1) * (ygap + ybet);
			info->a1.x = a1x, info->a1.y = a1y;
			info->a2.x = a1x + xgap, info->a2.y = a1y;
			info->a3.x = a1x, info->a3.y = a1y - ygap;
			info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
		}
		else if (wrong_ct == 10)
		{
			float a1x = origin_a1_x + (7 - 6) * (xgap + xbet);
			float a1y = origin_a1_y - (2 - 1) * (ygap + ybet);
			info->a1.x = a1x, info->a1.y = a1y;
			info->a2.x = a1x + xgap, info->a2.y = a1y;
			info->a3.x = a1x, info->a3.y = a1y - ygap;
			info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
		}
		else if (wrong_ct == 11)
		{
			float a1x = origin_a1_x + (8 - 6) * (xgap + xbet);
			float a1y = origin_a1_y - (3 - 1) * (ygap + ybet);
			info->a1.x = a1x, info->a1.y = a1y;
			info->a2.x = a1x + xgap, info->a2.y = a1y;
			info->a3.x = a1x, info->a3.y = a1y - ygap;
			info->a4.x = a1x + xgap, info->a4.y = a1y - ygap;
			wrong_ct = 0;
		}
	}

	if (ct == 42) ct = 0;

	static int TT = 0;
	static Info last_info = { 0 };

	bool aaa = false;
	if (TT < 42)
	{
		if (aaa) return;


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
		//memcpy(&last_info, info, sizeof(Info)); // 备份
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

	*(PULONG64)&bufcode[6] = (ULONG64)AsmHookHandler;

	DWORD old_protect = 0;
	VirtualProtect(t, 0x1000, PAGE_EXECUTE_READWRITE, &old_protect);
	A("改变内存属性成功");

	A("AsmHookHandler = %llx\r\n", AsmHookHandler);
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
		//HANDLE t = ::CreateThread(NULL, 0, mythread, NULL, 0, NULL);
		A("注入成功，测试。1");
		A("PULONG64 p_g_origin = 0x%llx; // 填入", &g_origin);
		A("ULONG64 AsmHookHandler = 0x%llx; // 填入", AsmHookHandler);
		break;
	}
	break;
	case DLL_THREAD_ATTACH:
	{
		A("注入成功，测试。2");
		break;
	}
	break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

