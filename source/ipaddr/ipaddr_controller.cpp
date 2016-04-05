#include "../../include/ipaddr/ipaddr_controller.h"

void Ipaddr::Controller_c::runover_init(void* rundata) {
	externalController.push_back(boundController);
	model.set_boundController(this);
	view.set_boundController(this);
}

Ipaddr::Controller_c::Controller_c() {
	functionType.functionType.push_back(eg::type::get_string_hash("Ipaddr::Controller_c"));
}