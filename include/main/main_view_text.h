#pragma once

#include <sstream>
#include <stdio.h>
#include "../../engine/include/base/eg_engine.h"
#include "main_include.h"
#include "main_model_main.h"

namespace Main {
	class View_Text_c : public eg::View {
	public:
		void runover_init(void* rundata);
		void runover(void* rundata);
		View_Text_c();
	protected:
		Model_Main_c* model_main;
	};
}