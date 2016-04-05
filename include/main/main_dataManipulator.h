#pragma once

#include <string>
#include <sstream>
#include <cstdio>
#include <3ds.h>
#include <base/eg_engine.h>
#include "main_include.h"

namespace Main {
	class DataManipulator_c : public eg::DataManipulator {
	public:
		DataManipulator_c();
	protected:
		static bool dataManipulator_loadData_impl(eg::GlbRtrn& rtrn, eg::DataReference& dataReference);
		static bool dataManipulator_freeData_impl(eg::GlbRtrn& rtrn, eg::DataReference& dataReference);
		static void resolve_symbol(CommandData& commandData, size_t& expected_button, size_t& expected_data, std::string& symbol);
		static void to_uint8(int16_t in, uint8_t* out);
		static void to_uint8(uint32_t in, uint8_t* out);
	};
}