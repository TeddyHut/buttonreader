#pragma once

#include <memory>
#include <cstring>
#include <base/eg_engine.h>
#include "main_include.h"
#include "main_input.h"
#include "main_model_main.h"
#include "main_model_socket.h"
#include "main_view_text.h"
#include "main_dataManipulator.h"
#include "../ipaddr/ipaddr_controller.h"

namespace Main {
	class Controller_c : public eg::Controller {
	public:
		enum class Stage_e : uint8_t {
			Ipaddr,
			Main
		};
		void runover_init(void* rundata) override;
		void runover(void* rundata) override;
		Controller_c();
	protected:
		uint8_t server_ipaddress[4];
		Stage_e stage;
		std::unique_ptr<Ipaddr::Controller_c> ipaddr_controller;
		DataManipulator_c main_dataManipulator;
		Input_c main_input;
		Model_Main_c main_model;
		Model_Socket_c socket_model;
		View_Text_c main_view;
	};
}