#pragma once

#include <string>
#include <sstream>
#include <3ds.h>
#include "../../engine/include/base/eg_engine.h"
#include "../main/main_include.h"
#include "ipaddr_include.h"
#include "ipaddr_model.h"

namespace Ipaddr {
	class View_c : public eg::View {
	public:
		void runover_init(void* rundata) override;
		void runover(void* rundata) override;
		PrintConsole* console;
		size_t consoleX;
		size_t consoleY;
		View_c();
	protected:
		Model_c* ipaddr_model;
	};
}