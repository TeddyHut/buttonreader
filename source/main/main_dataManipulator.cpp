#include "../../include/main/main_dataManipulator.h"

static std::string const _egNAME_FILE_seg_ = eg::util::truncateFilename(__FILE__);

Main::DataManipulator_c::DataManipulator_c() {
	functionType.functionType.push_back(eg::type::get_string_hash("dt_CommandData"));
	dataManipulator_loadData_exe = [this](eg::GlbRtrn& rtrn, eg::DataReference& dataReference)->bool { return(this->dataManipulator_loadData_impl(rtrn, dataReference)); };
	dataManipulator_freeData_exe = [this](eg::GlbRtrn& rtrn, eg::DataReference& dataReference)->bool { return(this->dataManipulator_freeData_impl(rtrn, dataReference)); };
}

bool Main::DataManipulator_c::dataManipulator_loadData_impl(eg::GlbRtrn& rtrn, eg::DataReference& dataReference) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::DataManipulator_c::dataManipulator_loadData_impl";
	if (dataReference.dataType == eg::type::get_string_hash("dt_CommandData")) {
		std::vector<CommandData>* tokenList = new std::vector<CommandData>;
		FILE* file = fopen("button_command.txt", "r");
		if (!file) {
			eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
			return(rtrn);
		}
		while (!feof(file)) {
			CommandData tokenLine;
			size_t expected_data = 0;
			size_t expected_button = 0;
			char cline[128];
			fgets(cline, 128, file);
			std::string line = cline;
			std::stringstream stream(line);
			std::string token;
			while (stream >> token) {
				resolve_symbol(tokenLine, expected_button, expected_data, token);
			}
			bool add_tokenLine = true;
			if (expected_button != tokenLine.button.size()) {
				eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
				add_tokenLine = false;
			}
			if (expected_button != tokenLine.button.size()) {
				eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
				add_tokenLine = false;
			}
			if (add_tokenLine) {
				tokenList->push_back(tokenLine);
			}
		}
		fclose(file);
		dataReference.dataPointer = static_cast<void*>(tokenList);
		dataReference.dataSize = sizeof(*tokenList);
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Success_r);
	}
	else
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
	return(rtrn);
}

bool Main::DataManipulator_c::dataManipulator_freeData_impl(eg::GlbRtrn& rtrn, eg::DataReference& dataReference) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::DataManipulator_c::dataManipulator_freeData_impl";
	if (dataReference.dataType == eg::type::get_string_hash("dt_CommandData")) {
		std::vector<CommandData>* data = static_cast<std::vector<CommandData>*>(dataReference.dataPointer);
		delete data;
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Success_r);
	}
	else
		eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
	return(rtrn);
}

void Main::DataManipulator_c::resolve_symbol(Main::CommandData& commandData, size_t& expected_button, size_t& expected_data, std::string& symbol) {
	//TODO: Use std::map here instead (when not tired and can be bothered figureing it out)
	//printf("\x1b[28;0H");
	if (symbol == "hld") {
		commandData.command.push_back(CommandSymbol_e::Command_Held);
		expected_button++;
	}
	else if (symbol == "prs") {
		commandData.command.push_back(CommandSymbol_e::Command_Pressed);
		expected_button++;
	}
	else if (symbol == "rls") {
		commandData.command.push_back(CommandSymbol_e::Command_Released);
		expected_button++;
	}
	else if (symbol == "sng") {
		commandData.command.push_back(CommandSymbol_e::Command_Single);
	}
	else if (symbol == ">") {
		commandData.command.push_back(CommandSymbol_e::Command_Result);
	}
	else if (symbol == "dlu") {
		commandData.command.push_back(CommandSymbol_e::Command_Time_usec);
		expected_data += sizeof(uint32_t);
	}
	else if (symbol == "lrxh") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_x_Held);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lryh") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_y_Held);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rrxh") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_x_Held);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rryh") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_y_Held);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lrxp") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_x_Pressed);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lryp") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_y_Pressed);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rrxp") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_x_Pressed);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rryp") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_y_Pressed);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lrxr") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_x_Released);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lryr") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPosRange_y_Released);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rrxr") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_x_Released);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "rryr") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPosRange_y_Released);
		expected_data += sizeof(int16_t) * 2;
	}
	else if (symbol == "lsx") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPos_x);
		expected_data += sizeof(int16_t);
	}
	else if (symbol == "lsy") {
		commandData.command.push_back(CommandSymbol_e::Command_leftStickPos_y);
		expected_data += sizeof(int16_t);
	}
	else if (symbol == "rsx") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPos_x);
		expected_data += sizeof(int16_t);
	}
	else if (symbol == "rsy") {
		commandData.command.push_back(CommandSymbol_e::Command_rightStickPos_y);
		expected_data += sizeof(int16_t);
	}
	else if (symbol == "bpa") {
		commandData.button.push_back(ButtonState_e::Key_A);
	}
	else if (symbol == "bpb") {
		commandData.button.push_back(ButtonState_e::Key_B);
	}
	else if (symbol == "bpx") {
		commandData.button.push_back(ButtonState_e::Key_X);
	}
	else if (symbol == "bpy") {
		commandData.button.push_back(ButtonState_e::Key_Y);
	}
	else if (symbol == "dpu") {
		commandData.button.push_back(ButtonState_e::Key_Up);
	}
	else if (symbol == "dpd") {
		commandData.button.push_back(ButtonState_e::Key_Down);
	}
	else if (symbol == "dpl") {
		commandData.button.push_back(ButtonState_e::Key_Left);
	}
	else if (symbol == "dpr") {
		commandData.button.push_back(ButtonState_e::Key_Right);
	}
	else if (symbol == "sbl") {
		commandData.button.push_back(ButtonState_e::Key_L);
	}
	else if (symbol == "sbr") {
		commandData.button.push_back(ButtonState_e::Key_R);
	}
	else if (symbol == "szl") {
		commandData.button.push_back(ButtonState_e::Key_ZL);
	}
	else if (symbol == "szr") {
		commandData.button.push_back(ButtonState_e::Key_ZR);
	}
	else if (symbol == "cst") {
		commandData.button.push_back(ButtonState_e::Key_Start);
	}
	else if (symbol == "csl") {
		commandData.button.push_back(ButtonState_e::Key_Select);
	}
	else if (symbol == "chm") {
		commandData.button.push_back(ButtonState_e::Key_Home);
	}
	else if (symbol == "stp") {
		commandData.command.push_back(CommandSymbol_e::Command_Stop);
	}
	else if (symbol.substr(0, 4) == "i16_") {
		std::stringstream convert;
		convert << symbol.substr(4);
		int16_t out;
		convert >> out;
		uint8_t data[2];
		to_uint8(out, data);
		commandData.data.push_back(data[0]);
		commandData.data.push_back(data[1]);
	}
	else if (symbol.substr(0, 4) == "u32_") {
		std::stringstream convert;
		convert << symbol.substr(4);
		uint32_t out;
		convert >> out;
		uint8_t data[4];
		to_uint8(out, data);
		commandData.data.push_back(data[0]);
		commandData.data.push_back(data[1]);
		commandData.data.push_back(data[2]);
		commandData.data.push_back(data[3]);
	}
	else {
	}
}

void Main::DataManipulator_c::to_uint8(int16_t const in, uint8_t* out) {
	uint8_int16_u convert;
	convert.int16_v = in;
	out[0] = convert.uint8_v[0];
	out[1] = convert.uint8_v[1];
}

void Main::DataManipulator_c::to_uint8(uint32_t const in, uint8_t* out) {
	uint8_uint32_u convert;
	convert.uint32_v = in;
	out[0] = convert.uint8_v[0];
	out[1] = convert.uint8_v[1];
	out[2] = convert.uint8_v[2];
	out[3] = convert.uint8_v[3];
}