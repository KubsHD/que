#include <game_app.h>

void App_Main(GraphicsAPI_Type apiType) {
	DebugOutput debugOutput;  // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
	XR_TUT_LOG("Que MAIN");
	GameApp app(apiType);


	app.Run();
}


#if defined(XR_OS_WINDOWS)

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	App_Main(VULKAN);
}

#endif

#if defined(XR_OS_ANDROID)
extern "C" {
	void android_main(struct android_app* app) {
		// Allow interaction with JNI and the JVM on this thread.
		// https://developer.android.com/training/articles/perf-jni#threads
		JNIEnv* env;
		app->activity->vm->AttachCurrentThread(&env, nullptr);

		// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_KHR_loader_init
		// Load xrInitializeLoaderKHR() function pointer. On Android, the loader must be initialized with variables from android_app *.
		// Without this, there's is no loader and thus our function calls to OpenXR would fail.
		XrInstance m_xrInstance = XR_NULL_HANDLE;  // Dummy XrInstance variable for OPENXR_CHECK macro.
		PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR = nullptr;
		OPENXR_CHECK(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR), "Failed to get InstanceProcAddr for xrInitializeLoaderKHR.");
		if (!xrInitializeLoaderKHR) {
			return;
		}

		// Fill out an XrLoaderInitInfoAndroidKHR structure and initialize the loader for Android.
		XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid{ XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
		loaderInitializeInfoAndroid.applicationVM = app->activity->vm;
		loaderInitializeInfoAndroid.applicationContext = app->activity->clazz;
		OPENXR_CHECK(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid), "Failed to initialize Loader for Android.");

		app->userData = &App::androidAppState;
		app->onAppCmd = &App::AndroidAppHandleCmd;

		App::androidApp = app;

		App_Main(VULKAN);
	}
}
#endif