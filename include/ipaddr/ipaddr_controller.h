#pragma once

#include <base/eg_engine.h>
#include "../main/main_include.h"
#include "ipaddr_include.h"
#include "ipaddr_input.h"
#include "ipaddr_model.h"
#include "ipaddr_view.h"

namespace Ipaddr {
	class Controller_c : public eg::Controller {
	public:
		void runover_init(void* rundata) override;
		Model_c model;
		View_c view;
		Controller_c();
	};
}