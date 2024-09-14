#include "pch.h"
#include "input.h"

#include <vector>
#include <string>
#include <common/OpenXRHelper.h>
#include <common/DebugOutput.h>
#include <common/glm_helpers.h>
#include "profiler.h"

static XrPath CreateXrPath(XrInstance instance, const char* path_string) {
	XrPath xrPath;
	OPENXR_CHECK_PORTABLE(instance, xrStringToPath(instance, path_string, &xrPath), "Failed to create XrPath from string.");
	return xrPath;
}
static std::string FromXrPath(XrInstance instance, XrPath path) {
	uint32_t strl;
	char text[XR_MAX_PATH_LENGTH];
	XrResult res;
	res = xrPathToString(instance, path, XR_MAX_PATH_LENGTH, &strl, text);
	std::string str;
	if (res == XR_SUCCESS) {
		str = text;
	}
	else {
		OPENXR_CHECK_PORTABLE(instance, res, "Failed to retrieve path.");
	}
	return str;
}

Input::Input(XrInstance instance, XrSession* session) : m_xrInstance(instance), m_session(session)
{
}

void Input::create_action_set()
{
	XrActionSetCreateInfo actionSetCI{ XR_TYPE_ACTION_SET_CREATE_INFO };
	// The internal name the runtime uses for this Action Set.
	strncpy(actionSetCI.actionSetName, "que-actionset", XR_MAX_ACTION_SET_NAME_SIZE);
	// Localized names are required so there is a human-readable action name to show the user if they are rebinding Actions in an options screen.
	strncpy(actionSetCI.localizedActionSetName, "Que ActionSet", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
	OPENXR_CHECK(xrCreateActionSet(m_xrInstance, &actionSetCI, &m_actionSet), "Failed to create ActionSet.");
	// Set a priority: this comes into play when we have multiple Action Sets, and determines which Action takes priority in binding to a specific input.
	actionSetCI.priority = 0;

	auto CreateAction = [this](XrAction& xrAction, const char* name, XrActionType xrActionType, std::vector<const char*> subaction_paths = {}) -> void {
		XrActionCreateInfo actionCI{ XR_TYPE_ACTION_CREATE_INFO };
		// The type of action: float input, pose, haptic output etc.
		actionCI.actionType = xrActionType;
		// Subaction paths, e.g. left and right hand. To distinguish the same action performed on different devices.
		std::vector<XrPath> subaction_xrpaths;
		for (auto p : subaction_paths) {
			subaction_xrpaths.push_back(CreateXrPath(m_xrInstance, p));
		}
		actionCI.countSubactionPaths = (uint32_t)subaction_xrpaths.size();
		actionCI.subactionPaths = subaction_xrpaths.data();
		// The internal name the runtime uses for this Action.
		strncpy(actionCI.actionName, name, XR_MAX_ACTION_NAME_SIZE);
		// Localized names are required so there is a human-readable action name to show the user if they are rebinding the Action in an options screen.
		strncpy(actionCI.localizedActionName, name, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
		OPENXR_CHECK(xrCreateAction(m_actionSet, &actionCI, &xrAction), "Failed to create Action.");
		};

	// An Action for the position of the palm of the user's hand - appropriate for the location of a grabbing Actions.
	CreateAction(m_palmPoseAction, "palm-pose", XR_ACTION_TYPE_POSE_INPUT, { "/user/hand/left", "/user/hand/right" });
	CreateAction(m_buzzAction, "buzz", XR_ACTION_TYPE_VIBRATION_OUTPUT, { "/user/hand/left", "/user/hand/right" });
	CreateAction(m_grabCubeAction, "grab-cube", XR_ACTION_TYPE_FLOAT_INPUT, { "/user/hand/left", "/user/hand/right" });
	CreateAction(m_movementAction, "stick", XR_ACTION_TYPE_VECTOR2F_INPUT, { "/user/hand/left", "/user/hand/right" });
	CreateAction(m_interactionAction, "interaction", XR_ACTION_TYPE_BOOLEAN_INPUT, { "/user/hand/left", "/user/hand/right" });

	// For later convenience we create the XrPaths for the subaction path names.
	m_handPaths[0] = CreateXrPath(m_xrInstance, "/user/hand/left");
	m_handPaths[1] = CreateXrPath(m_xrInstance, "/user/hand/right");
}

void Input::suggest_bindings()
{
	auto SuggestBindings = [this](const char* profile_path, std::vector<XrActionSuggestedBinding> bindings) -> bool {
		// The application can call xrSuggestInteractionProfileBindings once per interaction profile that it supports.
		XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		interactionProfileSuggestedBinding.interactionProfile = CreateXrPath(m_xrInstance, profile_path);
		interactionProfileSuggestedBinding.suggestedBindings = bindings.data();
		interactionProfileSuggestedBinding.countSuggestedBindings = (uint32_t)bindings.size();
		if (xrSuggestInteractionProfileBindings(m_xrInstance, &interactionProfileSuggestedBinding) == XrResult::XR_SUCCESS)
			return true;
		LOG_INFO("Failed to suggest bindings with " << profile_path);
		return false;
		};

	bool any_ok = false;
	any_ok |= SuggestBindings("/interaction_profiles/oculus/touch_controller", { {m_palmPoseAction, CreateXrPath(m_xrInstance, "/user/hand/left/input/grip/pose")},
																			  {m_palmPoseAction, CreateXrPath(m_xrInstance, "/user/hand/right/input/grip/pose")},
																			  {m_buzzAction, CreateXrPath(m_xrInstance, "/user/hand/left/output/haptic")},
																			  {m_buzzAction, CreateXrPath(m_xrInstance, "/user/hand/right/output/haptic")},
																				{m_grabCubeAction, CreateXrPath(m_xrInstance, "/user/hand/left/input/squeeze/value")},
																				{m_grabCubeAction, CreateXrPath(m_xrInstance, "/user/hand/right/input/squeeze/value")},
																				{m_movementAction, CreateXrPath(m_xrInstance, "/user/hand/left/input/thumbstick")},
																				{m_interactionAction, CreateXrPath(m_xrInstance, "/user/hand/right/input/a/click")},

		});
}

void Input::create_action_poses()
{
	// Create an xrSpace for a pose action.
	auto CreateActionPoseSpace = [this](XrSession session, XrAction xrAction, const char* subaction_path = nullptr) -> XrSpace {
		XrSpace xrSpace;
		const XrPosef xrPoseIdentity = { {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
		// Create frame of reference for a pose action
		XrActionSpaceCreateInfo actionSpaceCI{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
		actionSpaceCI.action = xrAction;
		actionSpaceCI.poseInActionSpace = xrPoseIdentity;
		if (subaction_path)
			actionSpaceCI.subactionPath = CreateXrPath(m_xrInstance, subaction_path);
		OPENXR_CHECK(xrCreateActionSpace(session, &actionSpaceCI, &xrSpace), "Failed to create ActionSpace.");
		return xrSpace;
		};

	m_handPoseSpace[0] = CreateActionPoseSpace(*m_session, m_palmPoseAction, "/user/hand/left");
	m_handPoseSpace[1] = CreateActionPoseSpace(*m_session, m_palmPoseAction, "/user/hand/right");
}

void Input::attach_action_set()
{
	// Attach the action set we just made to the session. We could attach multiple action sets!
	XrSessionActionSetsAttachInfo actionSetAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	actionSetAttachInfo.countActionSets = 1;
	actionSetAttachInfo.actionSets = &m_actionSet;
	OPENXR_CHECK(xrAttachSessionActionSets(*m_session, &actionSetAttachInfo), "Failed to attach ActionSet to Session.");
}

void Input::poll_actions(XrTime time, XrSpace local_space)
{
	QUE_PROFILE;

	XrActiveActionSet activeActionSet{};
	activeActionSet.actionSet = m_actionSet;
	activeActionSet.subactionPath = XR_NULL_PATH;
	// Now we sync the Actions to make sure they have current data.
	XrActionsSyncInfo actionsSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
	actionsSyncInfo.countActiveActionSets = 1;
	actionsSyncInfo.activeActionSets = &activeActionSet;
	OPENXR_CHECK(xrSyncActions(*m_session, &actionsSyncInfo), "Failed to sync Actions.");


	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	// We pose a single Action, twice - once for each subAction Path.
	actionStateGetInfo.action = m_palmPoseAction;
	// For each hand, get the pose state if possible.
	for (int i = 0; i < 2; i++) {
		// Specify the subAction Path.
		actionStateGetInfo.subactionPath = m_handPaths[i];
		OPENXR_CHECK(xrGetActionStatePose(*m_session, &actionStateGetInfo, &m_handPoseState[i]), "Failed to get Pose State.");
		if (m_handPoseState[i].isActive) {
			XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
			XrResult res = xrLocateSpace(m_handPoseSpace[i], local_space, time, &spaceLocation);
			if (XR_UNQUALIFIED_SUCCESS(res) &&
				(spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
				(spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
				m_handPose[i] = spaceLocation.pose;
			}
			else {
				m_handPoseState[i].isActive = false;
			}
		}
	}

	for (int i = 0; i < 2; i++) {
		actionStateGetInfo.action = m_grabCubeAction;
		actionStateGetInfo.subactionPath = m_handPaths[i];
		OPENXR_CHECK(xrGetActionStateFloat(*m_session, &actionStateGetInfo, &m_grabState[i]), "Failed to get Float State of Grab Cube action.");
	}

	actionStateGetInfo.action = m_movementAction;
	actionStateGetInfo.subactionPath = m_handPaths[0];
	OPENXR_CHECK(xrGetActionStateVector2f(*m_session, &actionStateGetInfo, &m_movementActionState), "Failed to get Vector2 State of Movement action.");

	actionStateGetInfo.action = m_interactionAction;
	actionStateGetInfo.subactionPath = m_handPaths[1];
	OPENXR_CHECK(xrGetActionStateBoolean(*m_session, &actionStateGetInfo, &m_interactionState), "Failed to get Boolean State of Interaction action.");
}

void Input::record_actions()
{
	if (m_session) {
		// now we are ready to:
		XrInteractionProfileState interactionProfile = { XR_TYPE_INTERACTION_PROFILE_STATE, 0, 0 };
		// for each action, what is the binding?
		OPENXR_CHECK_PORTABLE(m_xrInstance, xrGetCurrentInteractionProfile(*m_session, m_handPaths[0], &interactionProfile), "Failed to get profile.");
		if (interactionProfile.interactionProfile) {
		std::cout << "user/hand/left ActiveProfile " << FromXrPath(m_xrInstance, interactionProfile.interactionProfile).c_str();
		}
		OPENXR_CHECK_PORTABLE(m_xrInstance, xrGetCurrentInteractionProfile(*m_session, m_handPaths[1], &interactionProfile), "Failed to get profile.");
		if (interactionProfile.interactionProfile) {
			std::cout << "user/hand/right ActiveProfile " << FromXrPath(m_xrInstance, interactionProfile.interactionProfile).c_str();
		}
	}
}

void Input::draw_imgui()
{
	if (ImGui::Begin("Input debug"))
	{
		ImGui::LabelText("Grab state:", "%f %f", m_grabState[0].currentState, m_grabState[1].currentState);
		ImGui::LabelText("Movement input", "X: %f, Y: %f", get_movement_input().x, get_movement_input().y);

		ImGui::End();
	}
}

std::vector<XrPosef> Input::get_controller_poses()
{
	return { m_handPose[0], m_handPose[1] };
}

std::vector<float> Input::get_grab_state()
{
	return { m_grabState[0].currentState, m_grabState[1].currentState };
}

glm::vec2 Input::get_movement_input()
{
	XrVector2f vel = m_movementActionState.currentState;
	return glm::to_glm(vel);
}

bool Input::get_interaction_button_down()
{
	return m_interactionState.changedSinceLastSync && m_interactionState.currentState;
}
