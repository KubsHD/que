#pragma once

#include "pch.h"

#include <common/GraphicsAPI_Vulkan.h>
#include <common/OpenXRDebugUtils.h>
#include <common/xr_linear_algebra.h>

#include <core/asset.h>
#include <core/ecs.h>
#include <core/xr/xr_input.h>



class OpenXRPlatform {
public:
	OpenXRPlatform();
	~OpenXRPlatform() = default;

	void init();
	void destroy();

	void poll();

	void render();

	std::shared_ptr<XrInput> input;

	struct RenderLayerInfo;
	struct FrameRenderInfo;

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


	void set_render_callback(std::function<void(FrameRenderInfo)> callback) {
		m_render_callback = callback;
	}
private:
	void create_reference_space();
	void destroy_reference_space();

	void get_properties();
	void get_view_configuration_views();

	void create_swapchains();
	void destroy_swapchains();

	void create_instance();
	void destroy_instance();

	void create_debug();
	void destroy_debug();

	void create_session();
	void destroy_session();

	bool render_layer(RenderLayerInfo& info);

	void poll_events();
	void calculate_frame_stats();
	void get_requirements();
	void init_gfx_device_for_xr();
protected:

	XrDebugUtilsMessengerEXT m_debugMessenger = XR_NULL_HANDLE;

	GraphicsAPI_Type m_apiType = VULKAN;

	std::vector<const char*> m_activeAPILayers = {};
	std::vector<const char*> m_activeInstanceExtensions = {};
	std::vector<std::string> m_apiLayers = {};
	std::vector<std::string> m_instanceExtensions = {};

	// session stuff
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

	XrSpace m_localSpace = XR_NULL_HANDLE;

	std::function<void(FrameRenderInfo)> m_render_callback;

	XrGraphicsBindingVulkanKHR m_binding;

public:
	struct RenderLayerInfo {
		XrTime predictedDisplayTime;
		std::vector<XrCompositionLayerBaseHeader*> layers;
		XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
	};

	RenderLayerInfo renderLayerInfo;
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
#endif
private:
	void poll_system_events();
};
