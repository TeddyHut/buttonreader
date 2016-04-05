#include "../../include/main/main_include.h"

size_t const Main::ButtonState_z = 15;

Main::Stick_Position& Main::Stick_Position::operator=(circlePosition const& npos) {
	pos_x = npos.dx;
	pos_y = npos.dy;
	return(*this);
}

Main::Stick_Position::Stick_Position() : pos_x(0), pos_y(0) {
}

Main::Stick_Position::Stick_Position(int16_t npos_x, int16_t npos_y) : pos_x(npos_x), pos_y(npos_y) {
}

Main::Stick_Position::Stick_Position(circlePosition const& npos) : pos_x(npos.dx), pos_y(npos.dy) {
}

Main::ButtonData::ButtonData() {
	button_held.add_param(ButtonState_z);
	button_pressed.add_param(ButtonState_z);
	button_released.add_param(ButtonState_z);
}

void Main::resultOutput(eg::GlbRtrn const& rtrn, std::stringstream const& rtrn_stream, void* rundata) {
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	rtrn.print_output();
	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->bottom);
	printf(rtrn_stream.str().c_str());
	svcReleaseMutex(rundata_r->console_mutex);
}