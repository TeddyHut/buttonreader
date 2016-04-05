#include "../../include/ipaddr/ipaddr_view.h"

void Ipaddr::View_c::runover_init(void* rundata) {
	Main::Rundata* rundata_r = static_cast<Main::Rundata*>(rundata);
	consoleSelect(rundata_r->bottom);
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	eg::FunctionType temp_functionType;
	temp_functionType.functionType.push_back(eg::type::get_string_hash("Ipaddr::Model_c"));
	temp_functionType.set_basetype<eg::Model>();
	ipaddr_model = static_cast<Model_c*>(requestPointer(rtrn, temp_functionType));
	if (rtrn) {
		rtrn.print_output();
		printf(rtrn_stream.str().c_str());
		return;
	}
}

void Ipaddr::View_c::runover(void* rundata) {
	if ((ipaddr_model == nullptr) || (console == nullptr))
		return;
	Main::Rundata* rundata_r = static_cast<Main::Rundata*>(rundata);
	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(console);
	printf("\x1b[%u;%uH%03u.%03u.%03u.%03u", consoleY, consoleX, ipaddr_model->out_ipaddr_byte[0], ipaddr_model->out_ipaddr_byte[1], ipaddr_model->out_ipaddr_byte[2], ipaddr_model->out_ipaddr_byte[3]);
	printf("\x1b[%u;%uH               ", consoleY + 1, consoleX);
	printf("\x1b[%u;%uH^", consoleY + 1, (ipaddr_model->string_location == 11) ? 14 : (consoleX + ipaddr_model->string_location + (ipaddr_model->string_location / 3)));
	svcReleaseMutex(rundata_r->console_mutex);
}

Ipaddr::View_c::View_c() : console(nullptr), consoleX(0), consoleY(0), ipaddr_model(nullptr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Ipaddr::View_c"));
}