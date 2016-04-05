#include "../../include/main/main_model_socket.h"

static std::string const _egNAME_FILE_seg_ = eg::util::truncateFilename(__FILE__);

size_t const Main::Model_Socket_c::SocketService::buffer_size = 0x100000;
size_t const Main::Model_Socket_c::SocketService::align = 0x1000;

void Main::Model_Socket_c::runover_init(void* rundata) {
	acInit();

	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	eg::FunctionType temp_functionType;
	temp_functionType.functionType.push_back(eg::type::get_string_hash("Main::Model_c"));
	temp_functionType.set_basetype<eg::Model>();
	main_model = static_cast<Model_Main_c*>(requestPointer(rtrn, temp_functionType));
	if (rtrn) {
		svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
		consoleSelect(rundata_r->bottom);
		rtrn.print_output();
		printf(rtrn_stream.str().c_str());
		svcReleaseMutex(rundata_r->console_mutex);
		return;
	}

	SocketThread_data* thread_data = new SocketThread_data;
	int32_t thread_priority;
	thread_data->rundata = static_cast<Rundata*>(rundata);
	thread_data->this_p = this;
	svcGetThreadPriority(&thread_priority, CUR_THREAD_HANDLE);
	thread_priority--;
	socketThread = threadCreate(Main::Model_Socket_c::runover_thread, thread_data, 2 * 1024, thread_priority, -2, false);
	//Todo: The usual error checking here.
}

void Main::Model_Socket_c::runover(void* rundata) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::Model_Socket_c::runover";
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	if (runover_attemptWifi) {
		uint32_t wifi_out = 0;
		Result result = ACU_GetWifiStatus(&wifi_out);
		if (result) {
			eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
			resultOutput(rtrn, rtrn_stream, rundata);
			AtomicDecrement(&socketThread_continue);
			runover_attemptWifi = false;
			svcSignalEvent(wifi_connected);
			return;
		}
		if (wifi_out) {
			socketService.buffer_pointer = memalign(socketService.align, socketService.buffer_size);
			if (socketService.buffer_pointer == nullptr) {
				eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
				resultOutput(rtrn, rtrn_stream, rundata);
				AtomicDecrement(&socketThread_continue);
				runover_attemptWifi = false;
				svcSignalEvent(wifi_connected);
				return;
			}
			result = socInit(static_cast<uint32_t*>(socketService.buffer_pointer), socketService.buffer_size);
			if (result) {
				svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
				consoleSelect(rundata_r->bottom);
				printf("socInit Error: %lx\n", static_cast<int32_t>(result));
				svcReleaseMutex(rundata_r->console_mutex);
				eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
				resultOutput(rtrn, rtrn_stream, rundata);
				AtomicDecrement(&socketThread_continue);
				runover_attemptWifi = false;
				svcSignalEvent(wifi_connected);
				return;
			}
			svcSignalEvent(wifi_connected);
			runover_attemptWifi = false;
		}
	}
}

Main::Model_Socket_c::Model_Socket_c() : wifi_connected(false), socketThread_continue(1), runover_attemptWifi(true), run_main_runover_init(true), main_model(nullptr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Main::Model_Socket_c"));
	svcCreateMutex(&out_info_mutex, false);
	svcCreateMutex(&in_ipaddr_port_mutex, false);
	svcCreateEvent(&wifi_connected, 0);
	svcClearEvent(wifi_connected);
}

Main::Model_Socket_c::~Model_Socket_c() {
	AtomicDecrement(&socketThread_continue);
	threadJoin(socketThread, UINT64_MAX);
	threadFree(socketThread);
	svcCloseHandle(out_info_mutex);
	svcCloseHandle(in_ipaddr_port_mutex);
	svcCloseHandle(wifi_connected);
	socExit();
	linearFree(socketService.buffer_pointer);
	acExit();
}

uint32_t Main::Model_Socket_c::to_uint32(uint8_t const*const in) {
	Main::uint8_uint32_u convert;
	for (size_t i = 0; i < 4; i++) {
		convert.uint8_v[i] = in[i];
	}
	return(convert.uint32_v);
}

void Main::Model_Socket_c::runover_thread(void* data) {
	SocketThread_data* data_r = static_cast<SocketThread_data*>(data);
	//This is the thread.
	while (data_r->this_p->socketThread_continue != 0) {
		data_r->this_p->runover_main(data_r->rundata);
	}
	data_r->this_p->runover_main_deinit(data_r->rundata);
	delete data_r;
}

void Main::Model_Socket_c::runover_main(void* rundata) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::Model_Socket_c::runover_main";
	//This is the one that is running in the thread.
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
	if (run_main_runover_init) {
		runover_main_init(rtrn, rundata);
		if (rtrn) {
			resultOutput(rtrn, rtrn_stream, rundata);
			if(socketThread_continue != 0)
				AtomicDecrement(&socketThread_continue);
			return;
		}
		run_main_runover_init = false;
	}
	Packet in_packet;
	Command_Packet_e packetCommand;
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[sizeof(Packet)]);
	int main_result = 0;
	main_result = recv(main_socket, buffer.get(), sizeof(Packet), MSG_PEEK);
	if (main_result == 0)
		packetCommand = Command_Packet_e::Command_Null;
	else {
		in_packet.command = ntohl(*reinterpret_cast<uint32_t*>(buffer.get()));
		packetCommand = static_cast<Command_Packet_e>(in_packet.command);
	}
	switch (packetCommand) {
	case(Command_Packet_e::Command_Null) : {
		break;
	}
	case(Command_Packet_e::Command_SendButtonData) : {
		Packet_Button out_packet;
		out_packet.command = static_cast<uint32_t>(Command_Packet_e::Command_SendButtonData);
		copy_buttonData(out_packet);
		std::unique_ptr<uint8_t[]> out_buffer(new uint8_t[sizeof(Packet_Button)]);
		packetButton_to_buffer(out_packet, out_buffer.get());
		main_result = send(main_socket, out_buffer.get(), sizeof(Packet_Button), 0);
		if (main_result == -1) {
			eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
			resultOutput(rtrn, rtrn_stream, rundata);
			return;
		}
		break;
	}
	default: {
		break;
	}
	}

}

void Main::Model_Socket_c::runover_main_init(eg::GlbRtrn& rtrn, void* rundata) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::Model_Socket_c::runover_main_init";
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	int main_result = 0;

	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->bottom);
	printf("Waiting for WiFi...\n");
	svcReleaseMutex(rundata_r->console_mutex);
	svcWaitSynchronization(wifi_connected, UINT64_MAX);
	if (socketThread_continue == 0) {
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
		return;
	}
	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->bottom);
	printf("WiFi up!\n");
	svcReleaseMutex(rundata_r->console_mutex);

	main_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (main_socket < 0) {
		svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
		consoleSelect(rundata_r->bottom);
		printf("Error: %u, %s\n", errno, strerror(errno));
		svcReleaseMutex(rundata_r->console_mutex);
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
	}

	int prev_flags = fcntl(main_socket, F_GETFD);
	fcntl(main_socket, F_SETFD, prev_flags | O_NONBLOCK);

	char ip_addr_s[16];
	sprintf(ip_addr_s, "%03u.%03u.%03u.%03u", in_ipaddr[0], in_ipaddr[1], in_ipaddr[2], in_ipaddr[3]);

	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->bottom);
	printf("Attemping to connect to server:\n%s\n", ip_addr_s);
	svcReleaseMutex(rundata_r->console_mutex);

	main_result = inet_aton(ip_addr_s, &server_address);
	if (!main_result) {
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
		return;
	}
	server_socketAddress.sin_family = AF_INET;
	server_socketAddress.sin_port = htons(in_port);
	server_socketAddress.sin_addr = server_address;

	main_result = connect(main_socket, reinterpret_cast<sockaddr*>(&server_socketAddress), sizeof(server_socketAddress));
	if (main_result != 0) {
		svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
		consoleSelect(rundata_r->bottom);
		printf("Failed to connect to server!\n");
		svcReleaseMutex(rundata_r->console_mutex);
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
		return;
	}

	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->bottom);
	printf("Connection successful!\n");
	svcReleaseMutex(rundata_r->console_mutex);
}

void Main::Model_Socket_c::runover_main_deinit(void* rundata) {
	shutdown(main_socket, SHUT_RDWR);
	closesocket(main_socket);
}

void Main::Model_Socket_c::copy_buttonData(Packet_Button& out_packet) const {
	out_packet.keys_held = 0;
	if (main_model == nullptr)
		return;
	svcWaitSynchronization(main_model->out_buttonData_mutex, UINT64_MAX);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_A])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_A);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_B])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_B);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_X])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_X);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Y])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Y);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Up])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Up);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Down])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Down);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Left])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Left);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Right])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Right);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_L])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_L);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_R])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_R);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_ZL])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_ZL);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_ZR])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_ZR);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Start])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Start);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Select])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Select);
	if (main_model->out_buttonData.button_held[ButtonState_e::Key_Home])
		out_packet.keys_held |= static_cast<uint16_t>(KeyFlag_e::Key_Home);
	out_packet.stick_left_x = main_model->out_buttonData.leftStick_pos.pos_x;
	out_packet.stick_left_y = main_model->out_buttonData.leftStick_pos.pos_y;
	out_packet.stick_right_x = main_model->out_buttonData.rightStick_pos.pos_x;
	out_packet.stick_right_y = main_model->out_buttonData.rightStick_pos.pos_y;
	svcReleaseMutex(main_model->out_buttonData_mutex);
}

void Main::Model_Socket_c::packetButton_to_buffer(Packet_Button const& out_packet, uint8_t*const out_buffer) const {
	size_t offset = 0;
	*reinterpret_cast<uint32_t*>(out_buffer + offset) = htonl(out_packet.command); offset += sizeof(uint32_t);
	*reinterpret_cast<uint16_t*>(out_buffer + offset) = htons(out_packet.keys_held); offset += sizeof(uint16_t);
	*reinterpret_cast<uint16_t*>(out_buffer + offset) = htons(*reinterpret_cast<uint16_t const*const>(&out_packet.stick_left_x)); offset += sizeof(uint16_t);
	*reinterpret_cast<uint16_t*>(out_buffer + offset) = htons(*reinterpret_cast<uint16_t const*const>(&out_packet.stick_left_y)); offset += sizeof(uint16_t);
	*reinterpret_cast<uint16_t*>(out_buffer + offset) = htons(*reinterpret_cast<uint16_t const*const>(&out_packet.stick_right_x)); offset += sizeof(uint16_t);
	*reinterpret_cast<uint16_t*>(out_buffer + offset) = htons(*reinterpret_cast<uint16_t const*const>(&out_packet.stick_right_y)); offset += sizeof(uint16_t);
}