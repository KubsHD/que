#pragma once

#include <openxr/openxr.h>

#include <core/types.h>

enum ControllerType {
	LEFT = 0,
	RIGHT = 1,
};

enum class ControllerButton {
	A,
	B,
	X,
	Y,
	THUMBSTICK_L,
	THUMBSTICK_R,
	OPTIONS,

	MAX
};

class XrInput {
public:
	XrInput(XrInstance instance, XrSession* session);

	void create_action_set();
	void suggest_bindings();

	void create_action_poses();
	void attach_action_set();

	void poll_actions(XrTime time, XrSpace local_space);

	void record_actions();

	void draw_imgui();

	std::vector<XrPosef> get_controller_poses();
	std::vector<float> get_grab_state();
	glm::vec2 get_movement_input();

	bool get_interaction_button_down();
	bool get_interaction_button();
	bool get_interaction_button_up();

	bool get_grab_button_down(ControllerType type);
	bool get_grab_button(ControllerType type);
	bool get_grab_button_up(ControllerType type);
private:
	XrInstance m_xrInstance;
	XrSession* m_session;

	// input stuff
	XrActionSet m_actionSet;
	// An action for grabbing blocks, and an action to change the color of a block.
	XrAction m_grabCubeAction, m_spawnCubeAction, m_changeColorAction, m_interactionAction;
	// The realtime states of these actions.
	XrActionStateFloat m_grabState[2] = { {XR_TYPE_ACTION_STATE_FLOAT}, {XR_TYPE_ACTION_STATE_FLOAT} };
	XrActionStateBoolean m_changeColorState[2] = { {XR_TYPE_ACTION_STATE_BOOLEAN}, {XR_TYPE_ACTION_STATE_BOOLEAN} };
	XrActionStateBoolean m_spawnCubeState = { XR_TYPE_ACTION_STATE_BOOLEAN };

	XrActionStateBoolean m_interactionState = { XR_TYPE_ACTION_STATE_BOOLEAN };
	XrActionStateBoolean m_lastInteractionState = { XR_TYPE_ACTION_STATE_BOOLEAN };


	// The haptic output action for grabbing cubes.
	XrAction m_buzzAction;

	// thumbstick movement action
	XrAction m_movementAction;
	XrActionStateVector2f m_movementActionState = { XR_TYPE_ACTION_STATE_VECTOR2F };
	// The current haptic output value for each controller.
	float m_buzz[2] = { 0, 0 };
	// The action for getting the hand or controller position and orientation.
	XrAction m_palmPoseAction;
	// The XrPaths for left and right hand hands or controllers.
	XrPath m_handPaths[2] = { 0, 0 };
	// The spaces that represents the two hand poses.
	XrSpace m_handPoseSpace[2];
	XrActionStatePose m_handPoseState[2] = { {XR_TYPE_ACTION_STATE_POSE}, {XR_TYPE_ACTION_STATE_POSE} };
	// The current poses obtained from the XrSpaces.
	XrPosef m_handPose[2] = {
		{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1}},
		{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1}} };

	// last grab state
	float m_lastGrabState[2] = { 0, 0 };
};