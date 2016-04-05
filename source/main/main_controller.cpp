#include "../../include/main/main_controller.h"

void Main::Controller_c::runover_init(void* rundata) {
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	main_input.set_boundController(this);
	ipaddr_controller.reset(new Ipaddr::Controller_c);
	ipaddr_controller->view.console = rundata_r->top;
	ipaddr_controller->set_boundController(this);
}

void Main::Controller_c::runover(void* rundata) {
	switch (stage) {
	case(Stage_e::Ipaddr) : {
		svcWaitSynchronization(main_input.out_buttonData_mutex, UINT64_MAX);
		if (main_input.out_buttonData.button_pressed[ButtonState_e::Key_A]) {
			memcpy(server_ipaddress, ipaddr_controller->model.out_ipaddr_byte, 4);
			ipaddr_controller.reset();
			memcpy(socket_model.in_ipaddr, server_ipaddress, 4);
			socket_model.in_port = 3000;
			main_dataManipulator.set_boundController(this);
			main_model.set_boundController(this);
			socket_model.set_boundController(this);
			main_view.set_boundController(this);
			consoleClear();
			stage = Stage_e::Main;
		}
		svcReleaseMutex(main_input.out_buttonData_mutex);
		break;
	}
	case(Stage_e::Main) : {
		break;
	}
	default: {
		break;
	}
	}
}

Main::Controller_c::Controller_c() : stage(Stage_e::Ipaddr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Main::Controller_c"));
}