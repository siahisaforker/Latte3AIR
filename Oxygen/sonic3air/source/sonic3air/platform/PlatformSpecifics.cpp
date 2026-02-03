/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2025 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "sonic3air/pch.h"
#include "sonic3air/platform/PlatformSpecifics.h"

#if defined(PLATFORM_VITA)
	#include <vitasdk.h>
	#include <vitaGL.h>
	#include "sonic3air/platform/vita/trophies.h"
#endif

#if defined(PLATFORM_WIIU)
#include <sys/stat.h>
#include <cstring>
#if defined(__has_include)
#if __has_include(<padscore/kpad.h>)
#include <padscore/kpad.h>
#define WIIU_INIT_KPAD 1
#endif
#endif
#endif


namespace
{
#if defined(PLATFORM_VITA)
	int init_msg_dialog(const char* msg)
	{
		SceMsgDialogUserMessageParam msg_param;
		memset(&msg_param, 0, sizeof(msg_param));
		msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
		msg_param.msg = (SceChar8*)msg;

		SceMsgDialogParam param;
		sceMsgDialogParamInit(&param);
		_sceCommonDialogSetMagicNumber(&param.commonParam);
		param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
		param.userMsgParam = &msg_param;

		return sceMsgDialogInit(&param);
	}

	void warning(const char* msg)
	{
		init_msg_dialog(msg);

		while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED)
		{
			vglSwapBuffers(GL_TRUE);
		}
		sceMsgDialogTerm();
	}

	void fatal_error(const char* msg)
	{
		vglInit(0);
		warning(msg);
		sceKernelExitProcess(0);
		while (1);
	}

	int file_exists(const char* path)
	{
		SceIoStat stat;
		return sceIoGetstat(path, &stat) >= 0;
	}
#endif
}


void PlatformSpecifics::platformStartup()
{
#if defined(PLATFORM_VITA)
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);

	// Check for libshacccg.suprx existence
	if (!file_exists("ur0:/data/libshacccg.suprx") && !file_exists("ur0:/data/external/libshacccg.suprx"))
		fatal_error("Error: libshacccg.suprx is not installed.");

	vglInitExtended(0, 960, 544, 12 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);

	// Initing trophy system
	SceIoStat st;
	int r = trophies_init();
	if (r < 0 && sceIoGetstat("ux0:data/sonic3air/trophies.chk", &st) < 0)
	{
		FILE* f = fopen("ux0:data/sonic3air/trophies.chk", "w");
		fclose(f);
		warning("This game features unlockable trophies but NoTrpDrm is not installed. If you want to be able to unlock trophies, please install it.");
	}

	changeWorkingDirectory(L"ux0:/data/sonic3air");
#endif

#if defined(PLATFORM_WIIU)
#if defined(WIIU_INIT_KPAD)
	KPADInit();
#endif
	// Try common SD mount points and set working directory to the S3AIR folder if present
	{
		const char* candidates[] = {"sd:/S3AIR", "/vol/storage_sd/S3AIR", "/vol/storage_mlc01/S3AIR", "/vol/storage_usb01/S3AIR", "/S3AIR"};
		for (const char* p : candidates)
		{
			struct stat st;
			if (p && stat(p, &st) == 0 && (st.st_mode & S_IFDIR))
			{
				// Convert to wide string and change working directory
				std::wstring wpath;
				const size_t len = std::strlen(p);
				wpath.reserve(len);
				for (size_t i = 0; i < len; ++i) wpath.push_back((wchar_t)p[i]);
				changeWorkingDirectory(wpath);
				break;
			}
		}
	}

	// Verify required ROM is present in working directory
	{
		const char* requiredRom = "Sonic_Knuckles_wSonic3.bin";
		struct stat st;
		if (stat(requiredRom, &st) != 0)
		{
			// ROM not found — fail fast and instruct user where to place it
			std::fprintf(stderr, "Error: required ROM '%s' not found in working directory.\nPlease place it in the S3AIR folder on the SD card.\n", requiredRom);
			std::fflush(stderr);
			exit(1);
		}
	}
#endif
}
