#include "pch.h"

#include "xr_platform.h"

#include <core/profiler.h>
#include <common/DebugOutput.h>
#include <gfx/rhi/gfx_device.h>
#include "xr_wrapper.h"
#include <common/vk_initializers.h>
#include <base/numerics/safe_conversions.h>

#include <entt/entt.hpp>
#include <common/glm_helpers.h>
#include <gfx/camera.h>

#if defined(__ANDROID__)
android_app* OpenXRPlatform::androidApp = nullptr;
OpenXRPlatform::AndroidAppState OpenXRPlatform::androidAppState = {};
#endif

OpenXRPlatform::OpenXRPlatform()
{

}

void OpenXRPlatform::init(entt::registry& reg)
{
	{
		QUE_PROFILE_SECTION("OpenXr Init");

		Xr::Init();
		GfxDevice::InitXr(Xr::instance, Xr::systemId);
		Xr::GetVulkanGraphicsRequirements();

		get_properties();
		get_view_configuration_views();

		create_session();
		create_swapchains();

		// init renderer
		m_renderer = new Renderer2(m_colorSwapchainInfos[0], reg);

		create_reference_space();

		input = std::make_shared<XrInput>(Xr::instance, &m_session);
		input->create_action_set();
		input->suggest_bindings();
		input->create_action_poses();
		input->attach_action_set();
	}
}


void OpenXRPlatform::destroy()
{
	destroy_reference_space();
	destroy_swapchains();
	destroy_session();

	delete m_renderer;

	GfxDevice::Destroy();
	Xr::Destroy();
}

void OpenXRPlatform::poll()
{
	poll_system_events();
	poll_events();
}

void OpenXRPlatform::update()
{
	m_renderer->update();
}

void OpenXRPlatform::create_reference_space()
{
	XrReferenceSpaceCreateInfo referenceSpaceCI{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	referenceSpaceCI.poseInReferenceSpace = { {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
	OPENXR_CHECK_PORTABLE(Xr::instance, xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_localSpace), "Failed to create ReferenceSpace.");
}

void OpenXRPlatform::destroy_reference_space()
{
	OPENXR_CHECK_PORTABLE(Xr::instance, xrDestroySpace(m_localSpace), "Failed to destroy ReferenceSpace.");
}

bool OpenXRPlatform::render_layer(RenderLayerInfo& info)
{
	QUE_PROFILE;


	// Resize the layer projection views to match the view count. The layer projection views are used in the layer projection.
	//info.layerProjectionViews.resize(viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });

	// Per view in the view configuration:
	for (uint32_t i = 0; i < 1; i++) {
		Swapchain& colorSwapchainInfo = m_colorSwapchainInfos[i];
		Swapchain& depthSwapchainInfo = m_depthSwapchainInfos[i];

		// Acquire and wait for an image from the swapchains.
		// Get the image index of an image in the swapchains.
		// The timeout is infinite.
		uint32_t colorImageIndex = 0;
		uint32_t depthImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		OPENXR_CHECK_PORTABLE(Xr::instance, xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
		OPENXR_CHECK_PORTABLE(Xr::instance, xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

		XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		waitInfo.timeout = XR_INFINITE_DURATION;
		OPENXR_CHECK_PORTABLE(Xr::instance, xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
		OPENXR_CHECK_PORTABLE(Xr::instance, xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

		// Get the width and height and construct the viewport and scissors.
		const uint32_t& width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		const uint32_t& height = m_viewConfigurationViews[i].recommendedImageRectHeight;


		// Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
		// This also associates the swapchain image with this layer projection view.
		info.layerProjectionViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		//info.layerProjectionViews[i].pose = views[i].pose;
		//info.layerProjectionViews[i].fov = views[i].fov;
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
		//frameRenderInfo.view = views[i];



		// Give the swapchain image back to OpenXR, allowing the compositor to use the image.
		XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		OPENXR_CHECK_PORTABLE(Xr::instance, xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
		OPENXR_CHECK_PORTABLE(Xr::instance, xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");
	}

	// Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
	info.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	info.layerProjection.space = m_localSpace;
	info.layerProjection.viewCount = static_cast<uint32_t>(info.layerProjectionViews.size());
	info.layerProjection.views = info.layerProjectionViews.data();

	return true;
}

void OpenXRPlatform::get_view_configuration_views()
{
	// Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
	uint32_t viewConfigurationViewCount = 0;
	OPENXR_CHECK_PORTABLE(Xr::instance, xrEnumerateViewConfigurationViews(Xr::instance, Xr::systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
	m_viewConfigurationViews.resize(viewConfigurationViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	OPENXR_CHECK_PORTABLE(Xr::instance, xrEnumerateViewConfigurationViews(Xr::instance, Xr::systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");
}

void OpenXRPlatform::create_swapchains()
{

	uint32_t formatCount = 0;
	OPENXR_CHECK_PORTABLE(Xr::instance, xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr), "Failed to enumerate Swapchain Formats");
	std::vector<int64_t> formats(formatCount);
	OPENXR_CHECK_PORTABLE(Xr::instance, xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data()), "Failed to enumerate Swapchain Formats");
	if (GfxDevice::select_supported_color_format(formats) == 0) {
		std::cerr << "Failed to find depth format for Swapchain." << std::endl;
		DEBUG_BREAK;
	}

	m_colorSwapchainInfos.resize(m_viewConfigurationViews.size());

	// for each eye
	for (size_t i = 0; i < m_viewConfigurationViews.size(); i++) {
		Swapchain& colorSwapchainInfo = m_colorSwapchainInfos[i];

		// Color.
		XrSwapchainCreateInfo swapchainCI{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainCI.createFlags = 0;
		swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
		swapchainCI.format = GfxDevice::select_supported_color_format(formats);
		swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount;
		swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
		swapchainCI.faceCount = 1;
		swapchainCI.arraySize = 1;
		swapchainCI.mipCount = 1;
		OPENXR_CHECK_PORTABLE(Xr::instance, xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain), "Failed to create Color Swapchain");

		colorSwapchainInfo.swapchainFormat = (VkFormat)swapchainCI.format; 
		colorSwapchainInfo.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
		colorSwapchainInfo.height = m_viewConfigurationViews[i].recommendedImageRectHeight;

		// create image views
		// color:

		uint32_t colorSwapchainImageCount = 0;
		OPENXR_CHECK_PORTABLE(Xr::instance, xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr), "Failed to enumerate Color Swapchain Images.");

		colorSwapchainInfo.swapchainImageHandles = std::vector<XrSwapchainImageVulkanKHR>(colorSwapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });

		OPENXR_CHECK_PORTABLE(Xr::instance, 
			xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, (XrSwapchainImageBaseHeader*)colorSwapchainInfo.swapchainImageHandles.data()),
			"Failed to enumerate Color Swapchain Images."
		);

		// Per image in the eye
		for (uint32_t j = 0; j < colorSwapchainImageCount; j++) {

			auto ivinfo = vkinit::imageview_create_info(colorSwapchainInfo.swapchainFormat, colorSwapchainInfo.swapchainImageHandles[j].image, VK_IMAGE_ASPECT_COLOR_BIT);
			colorSwapchainInfo.swapchainImages.push_back(GfxDevice::create_image_view(ivinfo));
		}
	}
}

void OpenXRPlatform::destroy_swapchains()
{
	for (size_t i = 0; i < m_viewConfigurationViews.size(); i++)
	{
		Swapchain& colorSwapchainInfo = m_colorSwapchainInfos[i];

		// Destroy the color and depth image views from GraphicsAPI.
		for (VkImageView imageView : colorSwapchainInfo.swapchainImages) {
			GfxDevice::destroy_image_view(imageView);
		}

		// Destroy the swapchain.
		OPENXR_CHECK_PORTABLE(Xr::instance, xrDestroySwapchain(colorSwapchainInfo.swapchain), "Failed to destroy Color Swapchain");
	}
}

void OpenXRPlatform::render()
{
	if (!m_sessionRunning)
		return;

	QUE_PROFILE;

	XrFrameState frameState{ XR_TYPE_FRAME_STATE };
	XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	OPENXR_CHECK_PORTABLE(Xr::instance, xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");
	input->poll_actions(frameState.predictedDisplayTime, m_localSpace);

	XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	OPENXR_CHECK_PORTABLE(Xr::instance, xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

	
	std::vector<XrView> views(m_viewConfigurationViews.size(), { XR_TYPE_VIEW });

	XrViewState viewState{ XR_TYPE_VIEW_STATE };

	XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType = m_viewConfiguration;
	viewLocateInfo.displayTime = frameState.predictedDisplayTime;
	viewLocateInfo.space = m_localSpace;

	uint32_t viewCount = 0;
	OPENXR_CHECK(xrLocateViews(m_session, &viewLocateInfo, &viewState, static_cast<uint32_t>(views.size()), &viewCount, views.data()), "Failed to locate views");
	

	// Per view in the view configuration:
	for (uint32_t i = 0; i < viewCount; i++) {
		Swapchain& colorSwapchainInfo = m_colorSwapchainInfos[i];

		uint32_t colorImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");

		XrSwapchainImageWaitInfo waitImageInfo{};
		waitImageInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
		waitImageInfo.timeout = std::numeric_limits<int64_t>::max();
		OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitImageInfo), "Failed to wait for Image from the Color Swapchain");
		

		// process view data
		auto& view = views[i];

		glm::mat4 projection = glm::to_glm_projection(view.fov);

		auto camera_position = m_renderer->get_camera_position() + glm::vec3(view.pose.position.x, view.pose.position.y, view.pose.position.z);

		// https://gitlab.com/amini-allight/openxr-tutorial/-/blob/master/examples/part-9/openxr_example.cpp?ref_type=heads#L1434
		glm::mat4 viewMatrix = glm::inverse(
			glm::translate(glm::mat4(1.0f), glm::vec3(camera_position))
			* glm::mat4_cast(glm::quat(view.pose.orientation.w, view.pose.orientation.x, view.pose.orientation.y, view.pose.orientation.z))
		);


		CameraRenderData crd;
		crd.model = glm::mat4(1.0f);
		crd.projection = projection;
		crd.view = viewMatrix;
		crd.position = glm::vec3(camera_position);

		RenderTarget rt;

		rt.image.image = colorSwapchainInfo.swapchainImageHandles[colorImageIndex].image;
		rt.image.view = colorSwapchainInfo.swapchainImages[colorImageIndex];

		rt.size.x = colorSwapchainInfo.width;
		rt.size.y = colorSwapchainInfo.height;

		rt.format = colorSwapchainInfo.swapchainFormat;

		m_renderer->draw_xr(rt, crd);

		XrSwapchainImageReleaseInfo releaseImageInfo{};
		releaseImageInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
		OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseImageInfo), "Failed to release Image back to the Color Swapchain");
	}


	XrCompositionLayerProjectionView projectedViews[2]{};

	for (size_t i = 0; i < m_eye_count; i++)
	{
		projectedViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		projectedViews[i].pose = views[i].pose;
		projectedViews[i].fov = views[i].fov;
		projectedViews[i].subImage = {
			m_colorSwapchainInfos[i].swapchain,
			{
				{ 0, 0 },
				{ (int32_t)m_colorSwapchainInfos[i].width, (int32_t)m_colorSwapchainInfos[i].height }
			},
			0
		};
	}

	XrCompositionLayerProjection layer{};
	layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
	layer.space = m_localSpace;
	layer.viewCount = m_eye_count;
	layer.views = projectedViews;

	auto pLayer = (const XrCompositionLayerBaseHeader*)&layer;


	// Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = frameState.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.layerCount = 1;
	frameEndInfo.layers = &pLayer;
	OPENXR_CHECK_PORTABLE(Xr::instance, xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");

	QUE_PROFILE_FRAME;
}

void OpenXRPlatform::run()
{

}

void OpenXRPlatform::create_instance()
{
}

void OpenXRPlatform::destroy_instance()
{
	OPENXR_CHECK_PORTABLE(Xr::instance, xrDestroyInstance(Xr::instance), "Failed to destroy Instance.");
}

void OpenXRPlatform::get_properties()
{
	XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
	OPENXR_CHECK_PORTABLE(Xr::instance, xrGetInstanceProperties(Xr::instance, &instanceProperties), "Failed to get InstanceProperties.");

	LOG_INFO("OpenXR Runtime: " << instanceProperties.runtimeName << " - "
		<< XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
		<< XR_VERSION_PATCH(instanceProperties.runtimeVersion));
}


void OpenXRPlatform::create_session()
{
	XrSessionCreateInfo sessionCI{ XR_TYPE_SESSION_CREATE_INFO };

	m_binding = GfxDevice::create_xr_graphics_binding();

	sessionCI.next = &m_binding;
	sessionCI.createFlags = 0;
	sessionCI.systemId = Xr::systemId;

	OPENXR_CHECK_PORTABLE(Xr::instance, xrCreateSession(Xr::instance, &sessionCI, &m_session), "Failed to create Session.");
}

void OpenXRPlatform::destroy_session()
{
	OPENXR_CHECK_PORTABLE(Xr::instance, xrDestroySession(m_session), "Failed to create XrSession");
}

void OpenXRPlatform::poll_events()
{
	QUE_PROFILE;

	XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
	
	while (xrPollEvent(Xr::instance, &eventData) == XR_SUCCESS) {
		switch (eventData.type) {
			// Log the number of lost events from the runtime.
		case XR_TYPE_EVENT_DATA_EVENTS_LOST: {
			XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
			LOG_INFO("OPENXR: Events Lost: " << eventsLost->lostEventCount);
			break;
		}
										   // Log that an instance loss is pending and shutdown the application.
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
			XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
			LOG_INFO("OPENXR: Instance Loss Pending at: " << instanceLossPending->lossTime);
			m_sessionRunning = false;
			is_running = false;
			break;
		}
													 // Log that the interaction profile has changed.
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
			LOG_INFO("OPENXR: Interaction Profile changed for Session: " << interactionProfileChanged->session);
			if (interactionProfileChanged->session != m_session) {
				LOG_INFO("XrEventDataInteractionProfileChanged for unknown Session");
				break;
			}

			input->record_actions();

			break;
		}
														   // Log that there's a reference space change pending.
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
			XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
			LOG_INFO("OPENXR: Reference Space Change pending for Session: " << referenceSpaceChangePending->session);
			if (referenceSpaceChangePending->session != m_session) {
				LOG_INFO("XrEventDataReferenceSpaceChangePending for unknown Session");
				break;
			}
			break;
		}
															  // Session State changes:
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
			if (sessionStateChanged->session != m_session) {
				LOG_INFO("XrEventDataSessionStateChanged for unknown Session");
				break;
			}

			if (sessionStateChanged->state == XR_SESSION_STATE_READY) {
				// SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
				XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
				sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
				OPENXR_CHECK_PORTABLE(Xr::instance, xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
				m_sessionRunning = true;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING) {
				// SessionState is stopping. End the XrSession.
				OPENXR_CHECK_PORTABLE(Xr::instance, xrEndSession(m_session), "Failed to end Session.");
				m_sessionRunning = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_EXITING) {
				// SessionState is exiting. Exit the application.
				m_sessionRunning = false;
				is_running = false;
			}
			if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
				// SessionState is loss pending. Exit the application.
				// It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
				m_sessionRunning = false;
				is_running = false;
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

void OpenXRPlatform::calculate_frame_stats()
{
	/*m_delta_time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_last_time).count();
	m_last_time = std::chrono::high_resolution_clock::now();
	m_time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_start_time).count();*/
}

void OpenXRPlatform::get_requirements()
{
}

void OpenXRPlatform::init_gfx_device_for_xr()
{
}

#if defined(__ANDROID__)
void OpenXRPlatform::AndroidAppHandleCmd(struct android_app* app, int32_t cmd)
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


void OpenXRPlatform::poll_system_events()
{
	// Checks whether Android has requested that application should by destroyed.
	if (androidApp->destroyRequested != 0) {
		is_running = false;
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

void OpenXRPlatform::poll_system_events()
{
	return;
}

#endif