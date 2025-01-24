#include "pch.h"

#include <common/DebugOutput.h>

void App_Main();

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

#include <codecvt>
#include <asset/file/shader_bundle.h>
#include <asset/resource_compiler.h>
#include <asset/asset_manager.h>
#include <NGFX_Injection.h>
#include <core/profiler.h>

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
#if (_DEBUG && LIVEPP_ENABLED && 0)
	lpp::LppDefaultAgent lppAgent = lpp::LppCreateDefaultAgent(nullptr, LIVEPP_PATH);

	if (lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr, nullptr);
	}

#endif

#if defined(XR_OS_WINDOWS) && defined(_DEBUG)

	std::wstring commandLineStr(GetCommandLineW());

	// inject ngfx
	auto arg = L"-ngfx";
	if (commandLineStr.find(arg) != std::wstring::npos)
	{
		QUE_PROFILE_SECTION("NGFX Injection");

			uint32_t numInstallations = 0;
		auto result = NGFX_Injection_EnumerateInstallations(&numInstallations, nullptr);
		if (numInstallations == 0 || NGFX_INJECTION_RESULT_OK != result)
		{
			std::wstringstream stream;
			stream << L"Could not find any Nsight Graphics installations to inject: " << result << "\n";
			stream << L"Please install Nsight Graphics to enable programmatic injection.";
		}

		std::vector<NGFX_Injection_InstallationInfo> installations(numInstallations);
		result = NGFX_Injection_EnumerateInstallations(&numInstallations, installations.data());
		if (numInstallations == 0 || NGFX_INJECTION_RESULT_OK != result)
		{
			std::wstringstream stream;
			stream << L"Could not find any Nsight Graphics installations to inject: " << result << "\n";
			stream << L"Please install Nsight Graphics to enable programmatic injection.";
		}

		// 2) We have at least one Nsight Graphics installation, find which
		// activities are available using the latest installation.
		const NGFX_Injection_InstallationInfo& installation = installations.back();

		// 3) Retrieve the count of activities so we can initialize our activity data to the correct size
		uint32_t numActivities = 0;
		result = NGFX_Injection_EnumerateActivities(&installation, &numActivities, nullptr);
		if (numActivities == 0 || NGFX_INJECTION_RESULT_OK != result)
		{
			std::wstringstream stream;
			stream << L"Could not find any activities in Nsight Graphics installation: " << result << "\n";
			stream << L"Please install Nsight Graphics to enable programmatic injection.";
		}

		// With the count of activities available, query their description
		std::vector<NGFX_Injection_Activity> activities(numActivities);
		result = NGFX_Injection_EnumerateActivities(&installation, &numActivities, activities.data());
		if (NGFX_INJECTION_RESULT_OK != result)
		{
			std::wstringstream stream;
			stream << L"NGFX_Injection_EnumerateActivities failed with" << result;
		}

		// 4) We have valid activities. From here, we choose an activity.
		// In this sample, we use "Frame Debugger" activity
		const NGFX_Injection_Activity* pActivityToInject = nullptr;
		for (const NGFX_Injection_Activity& activity : activities)
		{
			if (activity.type == NGFX_INJECTION_ACTIVITY_FRAME_DEBUGGER)
			{
				pActivityToInject = &activity;
				break;
			}
		}

		if (!pActivityToInject) {
			std::wstringstream stream;
			stream << L"Frame Debugger activity is not available" << result;
		}

		// 5) With the activity identified, Inject into the process, setup for the
		// Frame Debugger activity
		result = NGFX_Injection_InjectToProcess(&installation, pActivityToInject);
		if (NGFX_INJECTION_RESULT_OK != result)
		{
			std::wstringstream stream;
			stream << L"NGFX_Injection_InjectToProcess failed with" << result;
		}
	}

#endif

#if defined(XR_OS_WINDOWS) && !defined(_DEBUG)
	auto crashpadok = start_crash_handler();
#endif

	DebugOutput debugOutput; // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
	AssetManager::PreInit();

#if _DEBUG

	if (commandLineStr.find(L"-rc"))
		ResourceCompiler::Compile(AssetManager::get_asset_dir(), ".cache");
#else
	if (!fs::exists("data"))
		ResourceCompiler::Compile("..\\..\\..\\..\\..\\game\\data", "data");
#endif

	App_Main();

#if (_DEBUG && LIVEPP_ENABLED && 0)
	if (lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		lpp::LppDestroyDefaultAgent(&lppAgent);
	}
#endif
}

#endif


#include <game.h>
#include <editor.h>

void App_Main()
{
	LOG_INFO("Que MAIN");


	// check if game is run with -ed flag
	bool run_editor = false;
	for (int i = 0; i < __argc; i++)
	{
		if (strcmp(__argv[i], "-ed") == 0)
		{
			run_editor = true;
			break;
		}
	}

	if (run_editor)
	{
		Editor editor;
		editor.run();
	}
	else
	{
		Game game;
		game.run();
	}

	return;
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

		App_Main();

		FMOD_Android_JNI_Close();
	}
}
#endif