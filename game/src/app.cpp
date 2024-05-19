#include "pch.h"

#include "app.h"
#include <core/profiler.h>
#include <common/DebugOutput.h>



#if defined(__ANDROID__)
android_app* App::androidApp = nullptr;
App::AndroidAppState App::androidAppState = {};
#endif


App::App(GraphicsAPI_Type apiType) : m_apiType(apiType)
{

}

void App::Run()
{
#if defined(__ANDROID__)
	m_asset_manager = std::make_shared<Asset>(androidApp->activity->assetManager);

#else
	m_asset_manager = std::make_shared<Asset>();
#endif

	m_asset_manager->Instance;

	{
		QUE_PROFILE_SECTION("OpenXr Init");

		create_instance();
		create_debug();
		get_properties();
		get_system();

		get_view_configuration_views();
		get_enviroment_blend_modes();

		input = std::make_shared<Input>(m_xrInstance, &m_session);
		input->create_action_set();
		input->suggest_bindings();

		create_session();
		create_swapchains();
		create_reference_space();

		input->create_action_poses();
		input->attach_action_set();
	}

	init();

	m_start_time = std::chrono::high_resolution_clock::now();

	while (m_applicationRunning)
	{
		poll_system_events();
		poll_events();
		if (m_sessionRunning)
		{
			calculate_frame_stats();
			update(m_delta_time);
			render_frame();
		}

		QUE_PROFILE_FRAME();
	}

	destroy();

	destroy_reference_space();
	destroy_swapchains();
	destroy_session();
	destroy_debug();
	destroy_instance();
}




void App::create_reference_space()
{
	// Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
	XrReferenceSpaceCreateInfo referenceSpaceCI{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	referenceSpaceCI.poseInReferenceSpace = { {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
	OPENXR_CHECK(xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_localSpace), "Failed to create ReferenceSpace.");
}

void App::destroy_reference_space()
{
	OPENXR_CHECK(xrDestroySpace(m_localSpace), "Failed to destroy ReferenceSpace.");
}

bool App::render_layer(RenderLayerInfo& info)
{
	QUE_PROFILE;

	// Locate the views from the view configuration within the (reference) space at the display time.
	std::vector<XrView> views(m_viewConfigurationViews.size(), { XR_TYPE_VIEW });

	XrViewState viewState{ XR_TYPE_VIEW_STATE };  // Will contain information on whether the position and/or orientation is valid and/or tracked.
	XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType = m_viewConfiguration;
	viewLocateInfo.displayTime = info.predictedDisplayTime;
	viewLocateInfo.space = m_localSpace;
	uint32_t viewCount = 0;
	XrResult result = xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data());
	if (result != XR_SUCCESS) {
		XR_TUT_LOG("Failed to locate Views.");
		return false;
	}

	// Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
	info.layerProjectionViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });

	// Per view in the view configuration:
	for (uint32_t i = 0; i < viewCount; i++) {
		SwapchainInfo& colorSwapchainInfo = m_colorSwapchainInfos[i];
		SwapchainInfo& depthSwapchainInfo = m_depthSwapchainInfos[i];

		// Acquire and wait for an image from the swapchains.
		// Get the image index of an image in the swapchains.
		// The timeout is infinite.
		uint32_t colorImageIndex = 0;
		uint32_t depthImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
		OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

		XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		waitInfo.timeout = XR_INFINITE_DURATION;
		OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
		OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

		// Get the width and height and construct the viewport and scissors.
		const uint32_t& width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		const uint32_t& height = m_viewConfigurationViews[i].recommendedImageRectHeight;


		// Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
		// This also associates the swapchain image with this layer projection view.
		info.layerProjectionViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		info.layerProjectionViews[i].pose = views[i].pose;
		info.layerProjectionViews[i].fov = views[i].fov;
		info.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;
		info.layerProjectionViews[i].subImage.imageRect.offset.x = 0;
		info.layerProjectionViews[i].subImage.imageRect.offset.y = 0;
		info.layerProjectionViews[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
		info.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
		info.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.

		// create FrameRenderInfo inline and call render
		FrameRenderInfo frameRenderInfo;
		frameRenderInfo.colorSwapchainInfo = &colorSwapchainInfo;
		frameRenderInfo.depthSwapchainInfo = &depthSwapchainInfo;
		frameRenderInfo.width = width;
		frameRenderInfo.height = height;
		frameRenderInfo.colorImageIndex = colorImageIndex;
		frameRenderInfo.depthImageIndex = depthImageIndex;
		frameRenderInfo.view = views[i];

		render(frameRenderInfo);


		// Give the swapchain image back to OpenXR, allowing the compositor to use the image.
		XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
		OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");
	}

	// Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
	info.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	info.layerProjection.space = m_localSpace;
	info.layerProjection.viewCount = static_cast<uint32_t>(info.layerProjectionViews.size());
	info.layerProjection.views = info.layerProjectionViews.data();

	return true;
}


void App::get_enviroment_blend_modes()
{
	uint32_t environmentBlendModeCount = 0;
	OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_xrSystemId, m_viewConfiguration, 0, &environmentBlendModeCount, nullptr), "Failed to enumerate EnvironmentBlend Modes.");
	m_environmentBlendModes.resize(environmentBlendModeCount);
	OPENXR_CHECK(xrEnumerateEnvironmentBlendModes(m_xrInstance, m_xrSystemId, m_viewConfiguration, environmentBlendModeCount, &environmentBlendModeCount, m_environmentBlendModes.data()), "Failed to enumerate EnvironmentBlend Modes.");

	// Pick the first application supported blend mode supported by the hardware.
	for (const XrEnvironmentBlendMode& environmentBlendMode : m_applicationEnvironmentBlendModes) {
		if (std::find(m_environmentBlendModes.begin(), m_environmentBlendModes.end(), environmentBlendMode) != m_environmentBlendModes.end()) {
			m_environmentBlendMode = environmentBlendMode;
			break;
		}
	}
	if (m_environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM) {
		XR_TUT_LOG_ERROR("Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.");
		m_environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	}
}

void App::get_view_configuration_views()
{
	// Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
	uint32_t viewConfigurationCount = 0;
	OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_xrSystemId, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
	m_viewConfigurations.resize(viewConfigurationCount);
	OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_xrSystemId, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

	for (const XrViewConfigurationType& viewConfig : m_applicationViewConfigurations)
	{
		if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfig) != m_viewConfigurations.end())
		{
			m_viewConfiguration = viewConfig;
			break;
		}
	}

	if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM) {
		std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
		m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	}

	// Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
	uint32_t viewConfigurationViewCount = 0;
	OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_xrSystemId, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
	m_viewConfigurationViews.resize(viewConfigurationViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_xrSystemId, m_viewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");
}

void App::create_swapchains()
{
	uint32_t formatCount = 0;
	OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr), "Failed to enumerate Swapchain Formats");
	std::vector<int64_t> formats(formatCount);
	OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data()), "Failed to enumerate Swapchain Formats");
	if (m_graphicsAPI->SelectDepthSwapchainFormat(formats) == 0) {
		std::cerr << "Failed to find depth format for Swapchain." << std::endl;
		DEBUG_BREAK;
	}

	m_colorSwapchainInfos.resize(m_viewConfigurationViews.size());
	m_depthSwapchainInfos.resize(m_viewConfigurationViews.size());

	for (size_t i = 0; i < m_viewConfigurationViews.size(); i++) {
		SwapchainInfo& colorSwapchainInfo = m_colorSwapchainInfos[i];
		SwapchainInfo& depthSwapchainInfo = m_depthSwapchainInfos[i];

		// Color.
		XrSwapchainCreateInfo swapchainCI{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainCI.createFlags = 0;
		swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
		swapchainCI.format = m_graphicsAPI->SelectColorSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
		swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
		swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
		swapchainCI.faceCount = 1;
		swapchainCI.arraySize = 1;
		swapchainCI.mipCount = 1;
		OPENXR_CHECK(xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain), "Failed to create Color Swapchain");
		colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

		// Depth.
		swapchainCI.createFlags = 0;
		swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
		swapchainCI.format = m_graphicsAPI->SelectDepthSwapchainFormat(formats);                // Use GraphicsAPI to select the first compatible format.
		swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
		swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
		swapchainCI.faceCount = 1;
		swapchainCI.arraySize = 1;
		swapchainCI.mipCount = 1;
		OPENXR_CHECK(xrCreateSwapchain(m_session, &swapchainCI, &depthSwapchainInfo.swapchain), "Failed to create Depth Swapchain");
		depthSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

		// Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
		uint32_t colorSwapchainImageCount = 0;
		OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr), "Failed to enumerate Color Swapchain Images.");
		XrSwapchainImageBaseHeader* colorSwapchainImages = m_graphicsAPI->AllocateSwapchainImageData(colorSwapchainInfo.swapchain, GraphicsAPI::SwapchainType::COLOR, colorSwapchainImageCount);
		OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages), "Failed to enumerate Color Swapchain Images.");

		uint32_t depthSwapchainImageCount = 0;
		OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr), "Failed to enumerate Depth Swapchain Images.");
		XrSwapchainImageBaseHeader* depthSwapchainImages = m_graphicsAPI->AllocateSwapchainImageData(depthSwapchainInfo.swapchain, GraphicsAPI::SwapchainType::DEPTH, depthSwapchainImageCount);
		OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages), "Failed to enumerate Depth Swapchain Images.");

		// Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
		for (uint32_t j = 0; j < colorSwapchainImageCount; j++) {
			GraphicsAPI::ImageViewCreateInfo imageViewCI;
			imageViewCI.image = m_graphicsAPI->GetSwapchainImage(colorSwapchainInfo.swapchain, j);
			imageViewCI.type = GraphicsAPI::ImageViewCreateInfo::Type::RTV;
			imageViewCI.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
			imageViewCI.format = colorSwapchainInfo.swapchainFormat;
			imageViewCI.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::COLOR_BIT;
			imageViewCI.baseMipLevel = 0;
			imageViewCI.levelCount = 1;
			imageViewCI.baseArrayLayer = 0;
			imageViewCI.layerCount = 1;
			colorSwapchainInfo.imageViews.push_back(m_graphicsAPI->CreateImageView(imageViewCI));
		}
		for (uint32_t j = 0; j < depthSwapchainImageCount; j++) {
			GraphicsAPI::ImageViewCreateInfo imageViewCI;
			imageViewCI.image = m_graphicsAPI->GetSwapchainImage(depthSwapchainInfo.swapchain, j);
			imageViewCI.type = GraphicsAPI::ImageViewCreateInfo::Type::DSV;
			imageViewCI.view = GraphicsAPI::ImageViewCreateInfo::View::TYPE_2D;
			imageViewCI.format = depthSwapchainInfo.swapchainFormat;
			imageViewCI.aspect = GraphicsAPI::ImageViewCreateInfo::Aspect::DEPTH_BIT;
			imageViewCI.baseMipLevel = 0;
			imageViewCI.levelCount = 1;
			imageViewCI.baseArrayLayer = 0;
			imageViewCI.layerCount = 1;
			depthSwapchainInfo.imageViews.push_back(m_graphicsAPI->CreateImageView(imageViewCI));
		}
	}
}

void App::destroy_swapchains()
{
	for (size_t i = 0; i < m_viewConfigurationViews.size(); i++)
	{
		SwapchainInfo& colorSwapchainInfo = m_colorSwapchainInfos[i];
		SwapchainInfo& depthSwapchainInfo = m_depthSwapchainInfos[i];

		// Destroy the color and depth image views from GraphicsAPI.
		for (VkImageView imageView : colorSwapchainInfo.imageViews) {
			//m_graphicsAPI->DestroyImageView(imageView);
		}
		for (VkImageView imageView : depthSwapchainInfo.imageViews) {
			//m_graphicsAPI->DestroyImageView(imageView);
		}

		// Free the Swapchain Image Data.
		m_graphicsAPI->FreeSwapchainImageData(colorSwapchainInfo.swapchain);
		m_graphicsAPI->FreeSwapchainImageData(depthSwapchainInfo.swapchain);

		// Destroy the swapchains.
		OPENXR_CHECK(xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain");
		OPENXR_CHECK(xrDestroySwapchain(depthSwapchainInfo.swapchain), "Failed to destroy Depth Swapchain");
	}
}

void App::render_frame()
{
	QUE_PROFILE;

	XrFrameState frameState{ XR_TYPE_FRAME_STATE };
	XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");

	// Tell the OpenXR compositor that the application is beginning the frame.
	XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

	// Variables for rendering and layer composition.
	bool rendered = false;
	RenderLayerInfo renderLayerInfo;
	renderLayerInfo.predictedDisplayTime = frameState.predictedDisplayTime;

	input->poll_actions(frameState.predictedDisplayTime, m_localSpace);

	// Check that the session is active and that we should render.
	bool sessionActive = (m_sessionState == XR_SESSION_STATE_SYNCHRONIZED || m_sessionState == XR_SESSION_STATE_VISIBLE || m_sessionState == XR_SESSION_STATE_FOCUSED);
	if (sessionActive && frameState.shouldRender) {
		// Render the stereo image and associate one of swapchain images with the XrCompositionLayerProjection structure.
		rendered = render_layer(renderLayerInfo);
		if (rendered) {
			renderLayerInfo.layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&renderLayerInfo.layerProjection));
		}
	}

	// Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = frameState.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = m_environmentBlendMode;
	frameEndInfo.layerCount = static_cast<uint32_t>(renderLayerInfo.layers.size());
	frameEndInfo.layers = renderLayerInfo.layers.data();
	OPENXR_CHECK(xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");
}

void App::create_instance()
{
	XrApplicationInfo appInfo{};
	appInfo.apiVersion = XR_CURRENT_API_VERSION;
	strcpy(appInfo.applicationName, "OpenXR Tutorial Chapter 2");
	appInfo.applicationVersion = 1;
	strcpy(appInfo.engineName, "No Engine");
	appInfo.engineVersion = 1;

	// enable extensions
	m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
	m_instanceExtensions.push_back(GetGraphicsAPIInstanceExtensionString(m_apiType));


	// Get all the API Layers from the OpenXR runtime.
	uint32_t apiLayerCount = 0;
	std::vector<XrApiLayerProperties> apiLayerProperties;
	OPENXR_CHECK(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr), "Failed to enumerate ApiLayerProperties.");
	apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
	OPENXR_CHECK(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()), "Failed to enumerate ApiLayerProperties.");

	// Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
	for (auto& requestLayer : m_apiLayers) {
		for (auto& layerProperty : apiLayerProperties) {
			// strcmp returns 0 if the strings match.
			if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0) {
				continue;
			}
			else {
				m_activeAPILayers.push_back(requestLayer.c_str());
				break;
			}
		}
	}

	// Get all the Instance Extensions from the OpenXR instance.
	uint32_t extensionCount = 0;
	std::vector<XrExtensionProperties> extensionProperties;
	OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "Failed to enumerate InstanceExtensionProperties.");
	extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
	OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()), "Failed to enumerate InstanceExtensionProperties.");

	// Check the requested Instance Extensions against the ones from the OpenXR runtime.
	// If an extension is found add it to Active Instance Extensions.
	// Log error if the Instance Extension is not found.
	for (auto& requestedInstanceExtension : m_instanceExtensions) {
		bool found = false;
		for (auto& extensionProperty : extensionProperties) {
			// strcmp returns 0 if the strings match.
			if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0) {
				continue;
			}
			else {
				m_activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
				found = true;
				break;
			}
		}
		if (!found) {
			XR_TUT_LOG_ERROR("Failed to find OpenXR instance extension: " << requestedInstanceExtension);
		}
	}

	XrInstanceCreateInfo createInfo{};
	createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	createInfo.createFlags = 0;
	createInfo.applicationInfo = appInfo;
	createInfo.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
	createInfo.enabledApiLayerNames = m_activeAPILayers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
	createInfo.enabledExtensionNames = m_activeInstanceExtensions.data();
	OPENXR_CHECK(xrCreateInstance(&createInfo, &m_xrInstance), "Failed to create Instance.");

	//
	//xrCreateInstance(&createInfo, &m_xrInstance);
}

void App::destroy_instance()
{
	OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy Instance.");
}

void App::create_debug()
{

}

void App::destroy_debug()
{

}

void App::get_properties()
{
	XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
	OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");

	XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - "
		<< XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}

void App::get_system()
{
	// Get the XrSystemId from the instance and the supplied XrFormFactor.
	XrSystemGetInfo systemGI{ XR_TYPE_SYSTEM_GET_INFO };
	systemGI.formFactor = m_xrFormFactor;
	OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGI, &m_xrSystemId), "Failed to get SystemID.");

	// Get the System's properties for some general information about the hardware and the vendor.
	OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_xrSystemId, &m_xrSystemProperties), "Failed to get SystemProperties.");
}

void App::create_session()
{
	XrSessionCreateInfo sessionCI{ XR_TYPE_SESSION_CREATE_INFO };

	m_graphicsAPI = std::make_shared<GraphicsAPI_Vulkan>(m_xrInstance, m_xrSystemId);

	sessionCI.next = m_graphicsAPI->GetGraphicsBinding();
	sessionCI.createFlags = 0;
	sessionCI.systemId = m_xrSystemId;

	OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");
}

void App::destroy_session()
{
	OPENXR_CHECK(xrDestroySession(m_session), "Failed to create XrSession");
}

void App::poll_events()
{
	QUE_PROFILE;

	XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
	auto XrPollEvents = [&]() -> bool {
		eventData = { XR_TYPE_EVENT_DATA_BUFFER };
		return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
		};


	while (XrPollEvents()) {
		switch (eventData.type) {
			// Log the number of lost events from the runtime.
		case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
			XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
			XR_TUT_LOG("OPENXR: Events Lost: " << eventsLost->lostEventCount);
			break;
		}
										   // Log that an instance loss is pending and shutdown the application.
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
			XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
			XR_TUT_LOG("OPENXR: Instance Loss Pending at: " << instanceLossPending->lossTime);
			m_sessionRunning = false;
			m_applicationRunning = false;
			break;
		}
													 // Log that the interaction profile has changed.
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
			XR_TUT_LOG("OPENXR: Interaction Profile changed for Session: " << interactionProfileChanged->session);
			if (interactionProfileChanged->session != m_session) {
				XR_TUT_LOG("XrEventDataInteractionProfileChanged for unknown Session");
				break;
			}

			input->record_actions();

			break;
		}
														   // Log that there's a reference space change pending.
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
			XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
			XR_TUT_LOG("OPENXR: Reference Space Change pending for Session: " << referenceSpaceChangePending->session);
			if (referenceSpaceChangePending->session != m_session) {
				XR_TUT_LOG("XrEventDataReferenceSpaceChangePending for unknown Session");
				break;
			}
			break;
		}
															  // Session State changes:
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
			if (sessionStateChanged->session != m_session) {
				XR_TUT_LOG("XrEventDataSessionStateChanged for unknown Session");
				break;
			}

			if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
				// SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
				XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
				sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
				OPENXR_CHECK(xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
				m_sessionRunning = true;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
				// SessionState is stopping. End the XrSession.
				OPENXR_CHECK(xrEndSession(m_session), "Failed to end Session.");
				m_sessionRunning = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
				// SessionState is exiting. Exit the application.
				m_sessionRunning = false;
				m_applicationRunning = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
				// SessionState is loss pending. Exit the application.
				// It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
				m_sessionRunning = false;
				m_applicationRunning = false;
			}
			// Store state for reference across the application.
			m_sessionState = sessionStateChanged->state;
			break;
		}
		default: {
			break;
		}
		}
	}
}

void App::calculate_frame_stats()
{
	m_delta_time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_last_time).count();
	m_last_time = std::chrono::high_resolution_clock::now();
	m_time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_start_time).count();
}

#if defined(__ANDROID__)
void App::AndroidAppHandleCmd(struct android_app* app, int32_t cmd)
{
	AndroidAppState* appState = (AndroidAppState*)app->userData;

	switch (cmd) {
		// There is no APP_CMD_CREATE. The ANativeActivity creates the application thread from onCreate().
		// The application thread then calls android_main().
	case APP_CMD_START: {
		break;
	}
	case APP_CMD_RESUME: {
		appState->resumed = true;
		break;
	}
	case APP_CMD_PAUSE: {
		appState->resumed = false;
		break;
	}
	case APP_CMD_STOP: {
		break;
	}
	case APP_CMD_DESTROY: {
		appState->nativeWindow = nullptr;
		break;
	}
	case APP_CMD_INIT_WINDOW: {
		appState->nativeWindow = app->window;
		break;
	}
	case APP_CMD_TERM_WINDOW: {
		appState->nativeWindow = nullptr;
		break;
	}
	}
}


void App::poll_system_events()
{
	// Checks whether Android has requested that application should by destroyed.
	if (androidApp->destroyRequested != 0) {
		m_applicationRunning = false;
		return;
	}
	while (true) {
		// Poll and process the Android OS system events.
		struct android_poll_source* source = nullptr;
		int events = 0;
		// The timeout depends on whether the application is active.
		const int timeoutMilliseconds = (!androidAppState.resumed && !m_sessionRunning && androidApp->destroyRequested == 0) ? -1 : 0;
		if (ALooper_pollAll(timeoutMilliseconds, nullptr, &events, (void**)&source) >= 0) {
			if (source != nullptr) {
				source->process(androidApp, source);
			}
		}
		else {
			break;
		}
	}
}
#else

void App::poll_system_events()
{
	return;
}

#endif