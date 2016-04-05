#pragma once

#include <string>
#include <memory>
#include <cstdio>
#include <malloc.h>
#include <3ds.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <base/eg_engine.h>
#include "main_include.h"
#include "main_model_main.h"

namespace Main {
	class Model_Socket_c : public eg::Model {
	public:
		void runover_init(void* rundata) override;	//Starts the thread
		void runover(void* rundata) override;	//Checks the thread status and whatnot

		std::string out_info;
		Handle out_info_mutex;

		uint8_t in_ipaddr[4];
		uint16_t in_port;
		Handle in_ipaddr_port_mutex;

		Model_Socket_c();
		~Model_Socket_c();
	protected:
		enum class Command_Packet_e : uint32_t {
			Command_Null = 0,
			Command_SendButtonData = 1
		};
		enum class KeyFlag_e : uint16_t {
			Key_A		= 0b0000000000000001,
			Key_B		= 0b0000000000000010,
			Key_X		= 0b0000000000000100,
			Key_Y		= 0b0000000000001000,
			Key_Up		= 0b0000000000010000,
			Key_Down	= 0b0000000000100000,
			Key_Left	= 0b0000000001000000,
			Key_Right	= 0b0000000010000000,
			Key_L		= 0b0000000100000000,
			Key_R		= 0b0000001000000000,
			Key_ZL		= 0b0000010000000000,
			Key_ZR		= 0b0000100000000000,
			Key_Start	= 0b0001000000000000,
			Key_Select	= 0b0010000000000000,
			Key_Home	= 0b0100000000000000
		};
		struct Packet {
			uint32_t command;
		};
		struct Packet_Button : public Packet {
			uint16_t keys_held;
			int16_t stick_left_x;
			int16_t stick_left_y;
			int16_t stick_right_x;
			int16_t stick_right_y;
		};
		struct SocketThread_data {
			Rundata* rundata;
			Model_Socket_c* this_p;
		};
		struct SocketService {
			static size_t const buffer_size;
			static size_t const align;
			void* buffer_pointer;
		};
		static uint32_t to_uint32(uint8_t const*const in);
		static void runover_thread(void* data);	//Calls this_p->runover_main
		void runover_main(void* rundata);
		void runover_main_init(eg::GlbRtrn& rtrn, void* rundata);
		void runover_main_deinit(void* rundata);

		void copy_buttonData(Packet_Button& out_packet) const;
		void packetButton_to_buffer(Packet_Button const& out_packet, uint8_t*const out_buffer) const;

		SocketService socketService;
		Handle wifi_connected;

		volatile uint32_t socketThread_continue;
		Thread socketThread;
		sockaddr_in server_socketAddress;
		in_addr server_address;

		bool runover_attemptWifi;

		bool run_main_runover_init;
		int main_socket;
		Model_Main_c* main_model;
	};
}