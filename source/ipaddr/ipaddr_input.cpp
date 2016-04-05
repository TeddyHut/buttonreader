#include "../../include/ipaddr/ipaddr_input.h"

int8_t const Ipaddr::Input_c::KEY_NONE = -1;
int8_t const Ipaddr::Input_c::KEY_BACKSPACE = -2;

void Ipaddr::Input_c::runover_init(void* rundata) {
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	eg::FunctionType temp_functionType;
	temp_functionType.functionType.push_back(eg::type::get_string_hash("Main::Input_c"));
	temp_functionType.set_basetype<eg::Input>();
	main_input = static_cast<Main::Input_c*>(requestPointer(rtrn, temp_functionType));
	if (rtrn) {
		Main::resultOutput(rtrn, rtrn_stream, rundata);
		return;
	}
}

void Ipaddr::Input_c::runover(void* rundata) {
	Main::Rundata* rundata_r = static_cast<Main::Rundata*>(rundata);
	if (main_input == nullptr)
		return;
	svcWaitSynchronization(main_input->out_buttonData_mutex, UINT64_MAX);
	if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Left])
		out_numeral = 0;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Up])
		out_numeral = 1;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Right])
		out_numeral = 2;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Down])
		out_numeral = 3;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Y])
		out_numeral = 4;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_X])
		out_numeral = 5;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_A])
		out_numeral = 6;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_B])
		out_numeral = 7;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_L])
		out_numeral = 8;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_R])
		out_numeral = 9;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_ZL] || main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_ZR])
		out_numeral = KEY_BACKSPACE;
	else if (main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Start])
		rundata_r->stop = true;
	else
		out_numeral = KEY_NONE;
	svcReleaseMutex(main_input->out_buttonData_mutex);
}

Ipaddr::Input_c::Input_c() : out_numeral(KEY_NONE) {
	functionType.functionType.push_back(eg::type::get_string_hash("Ipaddr::Input_c"));
}