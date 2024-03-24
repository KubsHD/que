#pragma once

#include <common/DebugOutput.h>
#include <common/GraphicsAPI_Vulkan.h>
#include <common/OpenXRDebugUtils.h>

#include <memory>
#include <common/xr_linear_algebra.h>

#include <math.h>
#include <core/asset.h>

#include <core/ecs.h>

#include <functional>
#include <core/input.h>


class App {
public:
	App(GraphicsAPI_Type apiType);
	~App() = default;
	void Run();

	std::function<void()> onInit;
	std::function<void()> onUpdate;
	std::function<void()> onRender;
	std::function<void()> onExit;

	std::unique_ptr<Scene> m_current_scene;
	std::shared_ptr<Input> input;

	struct RenderLayerInfo;
	struct FrameRenderInfo;

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void render(FrameRenderInfo& info) = 0;
	virtual void destroy() = 0;
	struct SwapchainInfo {
		XrSwapchain swapchain = XR_NULL_HANDLE;
		int64_t swapchainFormat = 0;
		std::vector<VkImageView> imageViews;
	};


	struct FrameRenderInfo {
		SwapchainInfo* colorSwapchainInfo;
		SwapchainInfo* depthSwapchainInfo;

		uint32_t colorImageIndex;
		uint32_t depthImageIndex;

		int width;
		int height;

		XrView view;
	};
protected:

	size_t renderCuboidIndex = 0;

	struct CameraConstants {
		XrMatrix4x4f viewProj;
		XrMatrix4x4f modelViewProj;
		XrMatrix4x4f model;
		XrVector4f color;
		XrVector4f pad1;
		XrVector4f pad2;
		XrVector4f pad3;
	};
private:
	void create_reference_space();
	void destroy_reference_space();


	void get_properties();
	void get_system();
	void get_enviroment_blend_modes();
	void get_view_configuration_views();

	void create_swapchains();
	void destroy_swapchains();

	void create_instance();
	void destroy_instance();

	void create_debug();
	void destroy_debug();

	void create_session();
	void destroy_session();

	void render_frame();
	bool render_layer(RenderLayerInfo& info);
	

	void poll_events();
protected:

	std::shared_ptr<Asset> m_asset_manager;

	XrInstance m_xrInstance{};

	XrDebugUtilsMessengerEXT m_debugMessenger = XR_NULL_HANDLE;

	XrFormFactor m_xrFormFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	XrSystemId m_xrSystemId = XR_NULL_SYSTEM_ID;
	XrSystemProperties m_xrSystemProperties = { XR_TYPE_SYSTEM_PROPERTIES };

	GraphicsAPI_Type m_apiType = VULKAN;

	std::vector<const char*> m_activeAPILayers = {};
	std::vector<const char*> m_activeInstanceExtensions = {};
	std::vector<std::string> m_apiLayers = {};
	std::vector<std::string> m_instanceExtensions = {};

	// session stuff
	std::unique_ptr<GraphicsAPI_Vulkan> m_graphicsAPI = nullptr;
	XrSession m_session = XR_NULL_HANDLE;

	XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;

	bool m_applicationRunning = true;
	bool m_sessionRunning = false;

	// swapchain stuff
	std::vector<XrViewConfigurationType> m_applicationViewConfigurations = { XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO };
	std::vector<XrViewConfigurationType> m_viewConfigurations;
	XrViewConfigurationType m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
	std::vector<XrViewConfigurationView> m_viewConfigurationViews;


	std::vector<SwapchainInfo> m_colorSwapchainInfos = {};
	std::vector<SwapchainInfo> m_depthSwapchainInfos = {};


	std::vector<XrEnvironmentBlendMode> m_applicationEnvironmentBlendModes = { XR_ENVIRONMENT_BLEND_MODE_OPAQUE, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE };
	std::vector<XrEnvironmentBlendMode> m_environmentBlendModes = {};
	XrEnvironmentBlendMode m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;

	XrSpace m_localSpace = XR_NULL_HANDLE;
	
	public:
	struct RenderLayerInfo {
		XrTime predictedDisplayTime;
		std::vector<XrCompositionLayerBaseHeader*> layers;
		XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
	};




#if defined(__ANDROID__)
public:
	// Stored pointer to the android_app structure from android_main().
	static android_app* androidApp;

	// Custom data structure that is used by PollSystemEvents().
	// Modified from https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/d6b6d7a10bdcf8d4fe806b4f415fde3dd5726878/src/tests/hello_xr/main.cpp#L133C1-L189C2
	struct AndroidAppState {
		ANativeWindow* nativeWindow = nullptr;
		bool resumed = false;
	};
	static AndroidAppState androidAppState;

	// Processes the next command from the Android OS. It updates AndroidAppState.
	static void AndroidAppHandleCmd(struct android_app* app, int32_t cmd);

private:
	void poll_system_events();
#else
	void poll_system_events();
#endif
};
