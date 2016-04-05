#include "../../include/main/main_view_text.h"

void Main::View_Text_c::runover_init(void* rundata) {
	eg::FunctionType request_temp;
	request_temp.functionType.push_back(eg::type::get_string_hash("Main::Model_Main_c"));
	request_temp.set_basetype<eg::Model>();
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	model_main = static_cast<Model_Main_c*>(requestPointer(rtrn, request_temp));
	if (rtrn) {
		resultOutput(rtrn, rtrn_stream, rundata);
		return;
	}
}

void Main::View_Text_c::runover(void* rundata) {
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	if (!model_main)
		return;
	svcWaitSynchronization(model_main->out_buttonData_mutex, UINT64_MAX);
	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->top);
	printf("\x1b[0;0HKey A: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_A]));
	printf("\x1b[1;0HKey B: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_B]));
	printf("\x1b[2;0HKey X: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_X]));
	printf("\x1b[3;0HKey Y: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Y]));
	printf("\x1b[4;0HKey Up: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Up]));
	printf("\x1b[5;0HKey Down: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Down]));
	printf("\x1b[6;0HKey Left: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Left]));
	printf("\x1b[7;0HKey Right: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Right]));
	printf("\x1b[8;0HKey L: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_L]));
	printf("\x1b[9;0HKey R: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_R]));
	printf("\x1b[10;0HKey ZL: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_ZL]));
	printf("\x1b[11;0HKey ZR: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_ZR]));
	printf("\x1b[12;0HKey Start: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Start]));
	printf("\x1b[13;0HKey Select: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Select]));
	printf("\x1b[14;0HKey Home: %u\n", static_cast<bool&>(model_main->out_buttonData.button_held[ButtonState_e::Key_Home]));
	printf("\x1b[15;0HLeftStickPos: %5i %5i\n", model_main->out_buttonData.leftStick_pos.pos_x, model_main->out_buttonData.leftStick_pos.pos_y);
	printf("\x1b[16;0HRightStickPos: %5i %5i\n", model_main->out_buttonData.rightStick_pos.pos_x, model_main->out_buttonData.rightStick_pos.pos_y);
	svcReleaseMutex(rundata_r->console_mutex);
	svcReleaseMutex(model_main->out_buttonData_mutex);
}

Main::View_Text_c::View_Text_c() : model_main(nullptr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Main::View_Text_c"));
}