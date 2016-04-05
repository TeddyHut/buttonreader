#pragma once

#include <3ds.h>
#include "../../engine/include/base/eg_engine.h"
#include "main_include.h"

namespace Main {
	class Input_c : public eg::Input {
	public:
		void runover(void* rundata);

		Handle out_buttonData_mutex;
		ButtonData out_buttonData;

		Input_c();
		virtual ~Input_c();
	protected:
		eg::Param<ButtonState_e> copy_buttonState(uint32_t const buttonreg) const;
	};
}