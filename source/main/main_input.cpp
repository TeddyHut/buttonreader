#include "../../include/main/main_input.h"

void Main::Input_c::runover(void* rundata) {
	circlePosition circlePad_pos;
	circlePosition Cstick_pos;
	hidCircleRead(&circlePad_pos);
	irrstCstickRead(&Cstick_pos);
	svcWaitSynchronization(out_buttonData_mutex, UINT64_MAX);
	out_buttonData.button_pressed = copy_buttonState(hidKeysDown());
	out_buttonData.button_released = copy_buttonState(hidKeysUp());
	out_buttonData.button_held = copy_buttonState(hidKeysHeld());
	out_buttonData.leftStick_pos = circlePad_pos;
	out_buttonData.rightStick_pos = Cstick_pos;
	svcReleaseMutex(out_buttonData_mutex);
}

Main::Input_c::Input_c() {
	functionType.functionType.push_back(eg::type::get_string_hash("Main::Input_c"));
	svcCreateMutex(&out_buttonData_mutex, false);
}

Main::Input_c::~Input_c() {
	svcCloseHandle(out_buttonData_mutex);
}

eg::Param<Main::ButtonState_e> Main::Input_c::copy_buttonState(uint32_t const regdata) const {
	eg::Param<ButtonState_e> param(ButtonState_z);
	if (regdata & KEY_A)
		param[ButtonState_e::Key_A] = true;
	if (regdata & KEY_B)
		param[ButtonState_e::Key_B] = true;
	if (regdata & KEY_X)
		param[ButtonState_e::Key_X] = true;
	if (regdata & KEY_Y)
		param[ButtonState_e::Key_Y] = true;
	if (regdata & KEY_DUP)
		param[ButtonState_e::Key_Up] = true;
	if (regdata & KEY_DDOWN)
		param[ButtonState_e::Key_Down] = true;
	if (regdata & KEY_DLEFT)
		param[ButtonState_e::Key_Left] = true;
	if (regdata & KEY_DRIGHT)
		param[ButtonState_e::Key_Right] = true;
	if (regdata & KEY_L)
		param[ButtonState_e::Key_L] = true;
	if (regdata & KEY_R)
		param[ButtonState_e::Key_R] = true;
	if (regdata & KEY_ZL)
		param[ButtonState_e::Key_ZL] = true;
	if (regdata & KEY_ZR)
		param[ButtonState_e::Key_ZR] = true;
	if (regdata & KEY_START)
		param[ButtonState_e::Key_Start] = true;
	if (regdata & KEY_SELECT)
		param[ButtonState_e::Key_Select] = true;
	return(param);
}