#include "../../include/ipaddr/ipaddr_model.h"

void Ipaddr::Model_c::runover_init(void* rundata) {
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

void Ipaddr::Model_c::runover(void* rundata) {
	if (main_input == nullptr)
		return;
	svcWaitSynchronization(main_input->out_buttonData_mutex, UINT64_MAX);
	if ((main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Right]) && (string_location < 11))
		string_location += 1;
	else if ((main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Left]) && (string_location > 0))
		string_location -= 1;
	else if ((main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Up]) && (out_ipaddr_bcd[string_location] < 9))
		out_ipaddr_bcd[string_location] += 1;
	else if ((main_input->out_buttonData.button_pressed[Main::ButtonState_e::Key_Down]) && (out_ipaddr_bcd[string_location] > 0))
		out_ipaddr_bcd[string_location] -= 1;
	svcReleaseMutex(main_input->out_buttonData_mutex);
	update_ipaddrByte();
}

Ipaddr::Model_c::Model_c() : out_ipaddr_bcd{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 }, out_ipaddr_byte { 0, 0, 0, 0 }, string_location(0), main_input(nullptr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Ipaddr::Model_c"));
}

void Ipaddr::Model_c::update_ipaddrByte() {
	auto convertToByte = [](uint8_t* bcd)->uint8_t {
		uint8_t output = 0;
		output += bcd[0] * 100;
		output += bcd[1] * 10;
		output += bcd[2];
		return(output);
	};
	for (size_t i = 0; i < 4; i++) {
		out_ipaddr_byte[i] = convertToByte(out_ipaddr_bcd + (i * 3));
	}
}