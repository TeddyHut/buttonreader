#pragma once

#include <cstdio>
#include <3ds.h>
#include <base/eg_engine.h>
#include "../main/main_include.h"
#include "../main/main_input.h"
#include "ipaddr_include.h"

namespace Ipaddr {
	class Input_c : public eg::Input {
	public:
		static int8_t const KEY_NONE;
		static int8_t const KEY_BACKSPACE;
		void runover_init(void* rundata) override;
		void runover(void* rundata) override;
		int8_t out_numeral;
		Input_c();
	protected:
		Main::Input_c* main_input;
	};
}