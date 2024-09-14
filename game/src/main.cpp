#include "pch.h"

#include <game_app.h>
#include <common/DebugOutput.h>

void App_Main(GraphicsAPI_Type apiType);

#if defined(XR_OS_WINDOWS)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <Shlwapi.h>
#include "../../projects/Game/config.generated.h"

#include <client/crash_report_database.h>
#include <client/settings.h>
#include <client/crashpad_client.h>
#include <client/crashpad_info.h>

#if defined(LIVEPP_ENABLED)
#include "LivePP/API/x64/LPP_API_x64_CPP.h"
#endif

std::unique_ptr<crashpad::CrashReportDatabase> database;

bool start_crash_handler()
{
	using namespace crashpad;

	std::map<std::string, std::string> annotations;
	std::vector<std::string> arguments;
	bool rc;

	annotations["format"] = "minidump";
	//arguments.push_back("--no-rate-limit");

	base::FilePath db(L"crashdb");
	base::FilePath handler(L"crashpad_handler.exe");

	database = crashpad::CrashReportDatabase::Initialize(db);

	/* Enable automated uploads. */
	database->GetSettings()->SetUploadsEnabled(true);

	std::string url = "https://submit.backtrace.io/kubs/2671612688bf66d376ef962cfea4bb86cee9d7c1c16c68ef6b4b532a042ec387/minidump";

	return CrashpadClient{}.StartHandler(
		handler, db, db, url, annotations, arguments, false, false, {});
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if (_DEBUG && LIVEPP_ENABLED)
	lpp::LppDefaultAgent lppAgent = lpp::LppCreateDefaultAgent(nullptr, LIVEPP_PATH);

	if (lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr, nullptr);
	}

#endif

#if defined(_WIN32) && defined(_DEBUG)

	std::wstring commandLineStr(GetCommandLineW());

	std::wstring arg = L"--shader_compile";
	if (commandLineStr.find(arg) != std::wstring::npos)
	{
		STARTUPINFO si{};
		PROCESS_INFORMATION pi{};

		OutputDebugString("Compiling shaders...");
		if (!CreateProcess(NULL, ".\\shader\\shader_compile.bat", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			OutputDebugString("ERROR COMPILING SHADERS");
		}
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		OutputDebugString(" Done!\n");
	}
#endif

#if defined(WIN32) && !defined(_DEBUG)
	auto crashpadok = start_crash_handler();
#endif

	App_Main(VULKAN);

#if (_DEBUG && LIVEPP_ENABLED)
	if (lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		lpp::LppDestroyDefaultAgent(&lppAgent);
	}
#endif
}

#endif

void App_Main(GraphicsAPI_Type apiType)
{

	DebugOutput debugOutput; // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
	LOG_INFO("Que MAIN");
	GameApp app(apiType);
	app.Run();
}


#if defined(XR_OS_ANDROID)
#include <fmod_android.h>
extern "C"
{
	void android_main(struct android_app *app)
	{
		// Allow interaction with JNI and the JVM on this thread.
		// https://developer.android.com/training/articles/perf-jni#threads
		JNIEnv *env;
		app->activity->vm->AttachCurrentThread(&env, nullptr);

		FMOD_Android_JNI_Init(app->activity->vm, app->activity->clazz);

		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_loader_init
		// Load xrInitializeLoaderKHR() function pointer. On Android, the loader must be initialized with variables from android_app *.
		// Without this, there's is no loader and thus our function calls to OpenXR would fail.
		XrInstance m_xrInstance = XR_NULL_HANDLE; // Dummy XrInstance variable for OPENXR_CHECK macro.
		PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
		OPENXR_CHECK(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction *)&xrInitializeLoaderKHR), "Failed to get InstanceProcAddr for xrInitializeLoaderKHR.");
		if (!xrInitializeLoaderKHR)
		{
			return;
		}

		// Fill out an XrLoaderInitInfoAndroidKHR structure and initialize the loader for Android.
		XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
		loaderInitializeInfoAndroid.applicationVM = app->activity->vm;
		loaderInitializeInfoAndroid.applicationContext = app->activity->clazz;
		OPENXR_CHECK(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&loaderInitializeInfoAndroid), "Failed to initialize Loader for Android.");

		app->userData = &App::androidAppState;
		app->onAppCmd = &App::AndroidAppHandleCmd;

		App::androidApp = app;

		App_Main(VULKAN);

		FMOD_Android_JNI_Close();
	}
}
#endif