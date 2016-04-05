#pragma once

#include <string>
#include <sstream>
#include <3ds.h>
#include <base/eg_engine.h>
#include "../main/main_include.h"
#include "ipaddr_include.h"
#include "ipaddr_input.h"

namespace Ipaddr {
	class Model_c : public eg::Model {
	public:
		void runover_init(void* rundata) override;
		void runover(void* rundata) override;

		uint8_t out_ipaddr_bcd[12];
		uint8_t out_ipaddr_byte[4];
		uint8_t string_location;

		Model_c();
	protected:
		void update_ipaddrByte();

		Main::Input_c* main_input;
	};
}