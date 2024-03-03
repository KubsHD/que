#include <common/DebugOutput.h>
#include <common/GraphicsAPI_Vulkan.h>
#include <common/OpenXRDebugUtils.h>

#include <memory>

class App {
public:
	App(GraphicsAPI_Type apiType) : m_apiType(apiType)
	{
	}
	~App() = default;
	void Run()
	{
		create_instance();
		create_debug();
		get_properties();
		get_system();

		create_session();

		while (m_applicationRunning)
		{
			poll_system_events();
			poll_events();
			if (m_sessionRunning)
			{
				render_frame();
			}
		}
		
		destroy_session();
		destroy_debug();
		destroy_instance();
	}
private:
	void render_frame()
	{
		XrFrameState frameState{ XR_TYPE_FRAME_STATE };
		XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
		OPENXR_CHECK(xrWaitFrame(m_session, &frameWaitInfo, &frameState), "Failed to wait for XR Frame.");

		// Tell the OpenXR compositor that the application is beginning the frame.
		XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
		OPENXR_CHECK(xrBeginFrame(m_session, &frameBeginInfo), "Failed to begin the XR Frame.");

		// Tell OpenXR that we are finished with this frame; specifying its display time, environment blending and layers.
		XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
		frameEndInfo.displayTime = frameState.predictedDisplayTime;
		frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		frameEndInfo.layerCount = 0;
		frameEndInfo.layers = nullptr;
		OPENXR_CHECK(xrEndFrame(m_session, &frameEndInfo), "Failed to end the XR Frame.");
	}

	void create_instance() {
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
	void destroy_instance() {
		OPENXR_CHECK(xrDestroyInstance(m_xrInstance), "Failed to destroy Instance.");
	}
	void create_debug() {
	}
	void destroy_debug() {
	}
	void get_properties() {
		XrInstanceProperties instanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
		OPENXR_CHECK(xrGetInstanceProperties(m_xrInstance, &instanceProperties), "Failed to get InstanceProperties.");

		XR_TUT_LOG("OpenXR Runtime: " << instanceProperties.runtimeName << " - "
			<< XR_VERSION_MAJOR(instanceProperties.runtimeVersion) << "."
			<< XR_VERSION_MINOR(instanceProperties.runtimeVersion) << "."
			<< XR_VERSION_PATCH(instanceProperties.runtimeVersion));
	}
	void get_system() {
		// Get the XrSystemId from the instance and the supplied XrFormFactor.
		XrSystemGetInfo systemGI{ XR_TYPE_SYSTEM_GET_INFO };
		systemGI.formFactor = m_xrFormFactor;
		OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGI, &m_xrSystemId), "Failed to get SystemID.");

		// Get the System's properties for some general information about the hardware and the vendor.
		OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_xrSystemId, &m_xrSystemProperties), "Failed to get SystemProperties.");
	}

	void create_session()
	{
		XrSessionCreateInfo sessionCI{ XR_TYPE_SESSION_CREATE_INFO };

		m_graphicsAPI = std::make_unique<GraphicsAPI_Vulkan>(m_xrInstance, m_xrSystemId);
		
		sessionCI.next = m_graphicsAPI->GetGraphicsBinding();
		sessionCI.createFlags = 0;
		sessionCI.systemId = m_xrSystemId;
		
		OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");
	}

	void destroy_session()
	{
		OPENXR_CHECK(xrDestroySession(m_session), "Failed to create XrSession");
	}

	void poll_events()
	{
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
					sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
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

	void poll_system_events()
	{
	}
private:

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
	std::unique_ptr<GraphicsAPI> m_graphicsAPI = nullptr;
	XrSession m_session = XR_NULL_HANDLE;

	XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;

	bool m_applicationRunning = true;
	bool m_sessionRunning = false;
};

void App_Main(GraphicsAPI_Type apiType) {
	DebugOutput debugOutput;  // This redirects std::cerr and std::cout to the IDE's output or Android Studio's logcat.
	XR_TUT_LOG("OpenXR Tutorial Chapter 2");
	App app(apiType);
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
		App_Main(VULKAN);
	}
}
#endif