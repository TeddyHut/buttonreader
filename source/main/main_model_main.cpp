#include "../../include/main/main_model_main.h"

static std::string const _egNAME_FILE_seg_ = eg::util::truncateFilename(__FILE__);

size_t const Main::Model_Main_c::Stick_State_z = 4;
size_t const Main::Model_Main_c::stackSize_executeResult = 2 * 1024;	//Whatever this is for

//TODO: Create a second thread object that handles the bulk of this stuff.

void Main::Model_Main_c::runover_init(void* rundata) {
	{
		eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
		eg::FunctionType temp_functionType;
		temp_functionType.functionType.push_back(eg::type::get_string_hash("Main::Input_c"));
		temp_functionType.set_basetype<eg::Input>();
		input_main = static_cast<Input_c*>(requestPointer(rtrn, temp_functionType));
		if (rtrn) {
			resultOutput(rtrn, rtrn_stream, rundata);
			return;
		}
	}
	{
		eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;
		eg::DataReference dataReference;
		dataReference.dataType = eg::type::get_string_hash("dt_CommandData");
		dataReference.alteration = 0;
		loadData(rtrn, dataReference);
		if (rtrn) {
			resultOutput(rtrn, rtrn_stream, rundata);
			return;
		}
		command = *(static_cast<std::vector<CommandData>*>(dataReference.dataPointer));
		freeData(rtrn, dataReference);
		if (rtrn) {
			resultOutput(rtrn, rtrn_stream, rundata);
			return;
		}
	}
}

void Main::Model_Main_c::runover(void* rundata) {
	Rundata* rundata_r = static_cast<Rundata*>(rundata);
	if (input_main == nullptr)
		return;
	eg::GlbRtrn rtrn; std::stringstream rtrn_stream; rtrn.output_stream = &rtrn_stream;

	svcWaitSynchronization(main_stop_mutex, UINT64_MAX);
	if (main_stop)
		rundata_r->stop = true;
	svcReleaseMutex(main_stop_mutex);

	svcWaitSynchronization(rundata_r->console_mutex, UINT64_MAX);
	consoleSelect(rundata_r->top);
	printf("\x1b[29;0HNo of Threads: %u", thread_executeResult_populus);
	consoleSelect(rundata_r->bottom);
	svcReleaseMutex(rundata_r->console_mutex);

	tickFrames();
	process_commands(rtrn);
	merge_to_outbuf();

	svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
	leftStickPos_prev = input_main->out_buttonData.leftStick_pos;
	rightStickPos_prev = input_main->out_buttonData.rightStick_pos;
	svcReleaseMutex(input_main->out_buttonData_mutex);
}

Main::Model_Main_c::Model_Main_c() : main_stop(false), thread_executeResult_populus(0), input_main(nullptr) {
	functionType.functionType.push_back(eg::type::get_string_hash("Main::Model_Main_c"));
	svcCreateMutex(&out_buttonData_mutex, false);
	svcCreateMutex(&threadbuf_buttonData_mutex, false);
	svcCreateMutex(&thread_executeResult_mutex, false);
	svcCreateMutex(&main_stop_mutex, false);
}

Main::Model_Main_c::~Model_Main_c() {
	while (thread_executeResult_populus != 0) {	//Watch out for the compiler !_!
		tickFrames();
	}

	svcCloseHandle(main_stop_mutex);
	svcCloseHandle(out_buttonData_mutex);
	svcCloseHandle(threadbuf_buttonData_mutex);
	svcCloseHandle(thread_executeResult_mutex);
}

void Main::Model_Main_c::executeResult(void* args) {
	ExecuteResult_a* arg_struct = static_cast<ExecuteResult_a*>(args);
	
	ThreadInfo thread_info;
	thread_info.thread = threadGetCurrent();
	thread_info.commandSet = arg_struct->commandSet;
	svcCreateEvent(&(thread_info.frame_passed), 0);
	svcClearEvent(thread_info.frame_passed);

	svcWaitSynchronization(arg_struct->this_p->thread_executeResult_mutex, UINT64_MAX);
	arg_struct->this_p->thread_executeResult.push_back(thread_info);
	svcReleaseMutex(arg_struct->this_p->thread_executeResult_mutex);
	AtomicIncrement(&(arg_struct->this_p->thread_executeResult_populus));

	auto commandInstr = arg_struct->commandString.command.begin();
	while (*commandInstr != CommandSymbol_e::Command_Result) {
		commandInstr++;
	}
	commandInstr++;		//Move off Command_Result

	eg::Param<ButtonState_e> active_button(ButtonState_z);
	eg::Param<Stick_State_e> active_stick(Stick_State_z);
	for (; commandInstr != arg_struct->commandString.command.end(); commandInstr++) {
		switch (*commandInstr) {
		case(CommandSymbol_e::Command_Time_usec) : {
			int64_t sleep_time = to_uint32(arg_struct->commandString.data, arg_struct->data_itr)*1000;
			svcSleepThread(sleep_time);
			break;
		}
		case(CommandSymbol_e::Command_Pressed) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
			arg_struct->this_p->threadbuf_buttonData.button_held[arg_struct->commandString.button[arg_struct->button_itr]] = true;
			arg_struct->this_p->threadbuf_buttonData.button_pressed[arg_struct->commandString.button[arg_struct->button_itr]] = true;
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);
			active_button[arg_struct->commandString.button[arg_struct->button_itr]] = true;
			arg_struct->button_itr++;
			break;
		}
		case(CommandSymbol_e::Command_Released) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);											//Get the mutex
			arg_struct->this_p->threadbuf_buttonData.button_held[arg_struct->commandString.button[arg_struct->button_itr]] = false;		//Set the value to false
			arg_struct->this_p->threadbuf_buttonData.button_pressed[arg_struct->commandString.button[arg_struct->button_itr]] = true;	//This button is active (global)
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);															//Release the mutex
			active_button[arg_struct->commandString.button[arg_struct->button_itr]] = true;												//We have told this one to be active
			arg_struct->button_itr++;																									//And of course this
			break;
		}
		case(CommandSymbol_e::Command_leftStickPos_x) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
			arg_struct->this_p->threadbuf_buttonData.leftStick_pos.pos_x = to_int16(arg_struct->commandString.data, arg_struct->data_itr);
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_L] = true;										//Special stuff for this
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);
			active_stick[Stick_State_e::leftStick_x] = true;
			break;
		}
		case(CommandSymbol_e::Command_leftStickPos_y) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
			arg_struct->this_p->threadbuf_buttonData.leftStick_pos.pos_y = to_int16(arg_struct->commandString.data, arg_struct->data_itr);
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_R] = true;										//Special stuff for this
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);
			active_stick[Stick_State_e::leftStick_y] = true;
			break;
		}
		case(CommandSymbol_e::Command_rightStickPos_x) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
			arg_struct->this_p->threadbuf_buttonData.rightStick_pos.pos_x = to_int16(arg_struct->commandString.data, arg_struct->data_itr);
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_ZL] = true;										//Special stuff for this
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);
			active_stick[Stick_State_e::rightStick_x] = true;
			break;
		}
		case(CommandSymbol_e::Command_rightStickPos_y) : {
			svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
			arg_struct->this_p->threadbuf_buttonData.rightStick_pos.pos_y = to_int16(arg_struct->commandString.data, arg_struct->data_itr);
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_ZR] = true;										//Special stuff for this
			svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);
			active_stick[Stick_State_e::rightStick_y] = true;
			break;
		}
		case(CommandSymbol_e::Command_Stop) : {
			svcWaitSynchronization(arg_struct->this_p->main_stop_mutex, UINT64_MAX);
			arg_struct->this_p->main_stop = true;
			svcReleaseMutex(arg_struct->this_p->main_stop_mutex);
			break;
		}
		default: {
			break;
		}
		}
	}

	svcWaitSynchronization(thread_info.frame_passed, UINT64_MAX);

	svcWaitSynchronization(arg_struct->this_p->threadbuf_buttonData_mutex, U64_MAX);
	{
		//Maybe put this in another function
		if (active_button[ButtonState_e::Key_A])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_A] = false;
		if (active_button[ButtonState_e::Key_B])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_B] = false;
		if (active_button[ButtonState_e::Key_X])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_X] = false;
		if (active_button[ButtonState_e::Key_Y])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Y] = false;
		if (active_button[ButtonState_e::Key_Up])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Up] = false;
		if (active_button[ButtonState_e::Key_Down])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Down] = false;
		if (active_button[ButtonState_e::Key_Left])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Left] = false;
		if (active_button[ButtonState_e::Key_Right])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Right] = false;
		if (active_button[ButtonState_e::Key_L])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_L] = false;
		if (active_button[ButtonState_e::Key_R])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_R] = false;
		if (active_button[ButtonState_e::Key_ZL])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_ZL] = false;
		if (active_button[ButtonState_e::Key_ZR])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_ZR] = false;
		if (active_button[ButtonState_e::Key_Start])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Start] = false;
		if (active_button[ButtonState_e::Key_Select])
			arg_struct->this_p->threadbuf_buttonData.button_pressed[ButtonState_e::Key_Select] = false;
		if (active_stick[Stick_State_e::leftStick_x])
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_L] = false;
		if (active_stick[Stick_State_e::leftStick_y])
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_R] = false;
		if (active_stick[Stick_State_e::rightStick_x])
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_ZL] = false;
		if (active_stick[Stick_State_e::rightStick_y])
			arg_struct->this_p->threadbuf_buttonData.button_released[ButtonState_e::Key_ZR] = false;
	}
	svcReleaseMutex(arg_struct->this_p->threadbuf_buttonData_mutex);

	svcWaitSynchronization(arg_struct->this_p->thread_executeResult_mutex, U64_MAX);
	for (auto element = arg_struct->this_p->thread_executeResult.begin(); element != arg_struct->this_p->thread_executeResult.end(); element++) {
		if ((*element).thread == thread_info.thread) {
			arg_struct->this_p->thread_executeResult.erase(element);
			break;
		}
	}
	svcReleaseMutex(arg_struct->this_p->thread_executeResult_mutex);
	svcCloseHandle(thread_info.frame_passed);
	AtomicDecrement(&arg_struct->this_p->thread_executeResult_populus);
	delete arg_struct;
	threadExit(0);
}

void Main::Model_Main_c::merge_to_outbuf() {
	svcWaitSynchronization(threadbuf_buttonData_mutex, UINT64_MAX);
	svcWaitSynchronization(out_buttonData_mutex, UINT64_MAX);
	svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
	//TODO: Make it so that you can buttonData = buttonData
	work_buttonData.button_held = input_main->out_buttonData.button_held;
	work_buttonData.leftStick_pos = input_main->out_buttonData.leftStick_pos;
	work_buttonData.rightStick_pos = input_main->out_buttonData.rightStick_pos;
	{
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_A])
			(work_buttonData.button_held[ButtonState_e::Key_A]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_A));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_B])
			(work_buttonData.button_held[ButtonState_e::Key_B]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_B));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_X])
			(work_buttonData.button_held[ButtonState_e::Key_X]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_X));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Y])
			(work_buttonData.button_held[ButtonState_e::Key_Y]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Y));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Up])
			(work_buttonData.button_held[ButtonState_e::Key_Up]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Up));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Down])
			(work_buttonData.button_held[ButtonState_e::Key_Down]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Down));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Left])
			(work_buttonData.button_held[ButtonState_e::Key_Left]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Left));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Right])
			(work_buttonData.button_held[ButtonState_e::Key_Right]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Right));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_L])
			(work_buttonData.button_held[ButtonState_e::Key_L]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_L));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_R])
			(work_buttonData.button_held[ButtonState_e::Key_R]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_R));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_ZL])
			(work_buttonData.button_held[ButtonState_e::Key_ZL]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_ZL));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_ZR])
			(work_buttonData.button_held[ButtonState_e::Key_ZR]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_ZR));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Start])
			(work_buttonData.button_held[ButtonState_e::Key_Start]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Start));
		if (threadbuf_buttonData.button_pressed[ButtonState_e::Key_Select])
			(work_buttonData.button_held[ButtonState_e::Key_Select]) = (threadbuf_buttonData.button_held.get_param(ButtonState_e::Key_Select));
		if (threadbuf_buttonData.button_released[ButtonState_e::Key_L])
			work_buttonData.leftStick_pos.pos_x = threadbuf_buttonData.leftStick_pos.pos_x;
		if (threadbuf_buttonData.button_released[ButtonState_e::Key_R])
			work_buttonData.leftStick_pos.pos_y = threadbuf_buttonData.leftStick_pos.pos_y;
		if (threadbuf_buttonData.button_released[ButtonState_e::Key_ZL])
			work_buttonData.rightStick_pos.pos_x = threadbuf_buttonData.rightStick_pos.pos_x;
		if (threadbuf_buttonData.button_released[ButtonState_e::Key_ZR])
			work_buttonData.rightStick_pos.pos_y = threadbuf_buttonData.rightStick_pos.pos_y;
	}
	out_buttonData.button_held = work_buttonData.button_held;
	out_buttonData.leftStick_pos = work_buttonData.leftStick_pos;
	out_buttonData.rightStick_pos = work_buttonData.rightStick_pos;
	svcReleaseMutex(threadbuf_buttonData_mutex);
	svcReleaseMutex(out_buttonData_mutex);
	svcReleaseMutex(input_main->out_buttonData_mutex);
}

void Main::Model_Main_c::tickFrames() {
	svcWaitSynchronization(thread_executeResult_mutex, UINT64_MAX);
	for (auto&& element : thread_executeResult) {
		svcSignalEvent(element.frame_passed);
	}
	svcReleaseMutex(thread_executeResult_mutex);
}

void Main::Model_Main_c::process_commands(eg::GlbRtrn& rtrn) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::Model_Main_c::process_commands";
	//Iterate through command strings
	for (size_t i = 0; i < command.size();i++) {
		//Iterate through command instructions
		bool run_result = true;
		size_t button_itr = 0;
		size_t data_itr = 0;
		for (auto commandInstr = command[i].command.begin(); commandInstr != command[i].command.end(); commandInstr++) {
			switch (*commandInstr) {
			case(CommandSymbol_e::Command_Held) : {			//If it is a command held instruction
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!input_main->out_buttonData.button_held[command[i].button[button_itr]])
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				button_itr++;
				break;
			}
			case(CommandSymbol_e::Command_Pressed) : {		//If it is a command pressed instruction
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!input_main->out_buttonData.button_pressed[command[i].button[button_itr]])
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				button_itr++;
				break;
			}
			case(CommandSymbol_e::Command_Released) : {		//If it is a command released instruction
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!input_main->out_buttonData.button_released[command[i].button[button_itr]])
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				button_itr++;
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_x_Held) : {	//If it is a left stick range instruction
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Held, input_main->out_buttonData.leftStick_pos.pos_x, leftStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_y_Held) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Held, input_main->out_buttonData.leftStick_pos.pos_y, leftStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_x_Held) : {
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Held, input_main->out_buttonData.rightStick_pos.pos_x, rightStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_y_Held) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Held, input_main->out_buttonData.rightStick_pos.pos_y, rightStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_x_Pressed) : {	//If it is a left stick range instruction
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Pressed, input_main->out_buttonData.leftStick_pos.pos_x, leftStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_y_Pressed) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Pressed, input_main->out_buttonData.leftStick_pos.pos_y, leftStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_x_Pressed) : {
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Pressed, input_main->out_buttonData.rightStick_pos.pos_x, rightStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_y_Pressed) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Pressed, input_main->out_buttonData.rightStick_pos.pos_y, rightStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_x_Released) : {	//If it is a left stick range instruction
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Released, input_main->out_buttonData.leftStick_pos.pos_x, leftStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_leftStickPosRange_y_Released) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Released, input_main->out_buttonData.leftStick_pos.pos_y, leftStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_x_Released) : {
				int16_t pos_xa = to_int16(command[i].data, data_itr);
				int16_t pos_xb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Released, input_main->out_buttonData.rightStick_pos.pos_x, rightStickPos_prev.pos_x, pos_xa, pos_xb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_rightStickPosRange_y_Released) : {
				int16_t pos_ya = to_int16(command[i].data, data_itr);
				int16_t pos_yb = to_int16(command[i].data, data_itr);
				svcWaitSynchronization(input_main->out_buttonData_mutex, UINT64_MAX);
				if (!check_stickEvent(ButtonState_Compare_e::Command_Released, input_main->out_buttonData.rightStick_pos.pos_y, rightStickPos_prev.pos_y, pos_ya, pos_yb))
					run_result = false;
				svcReleaseMutex(input_main->out_buttonData_mutex);
				break;
			}
			case(CommandSymbol_e::Command_Result) : {
				if (run_result) {
					createThread_executeResult(rtrn, command[i], button_itr, data_itr, i);
					run_result = false;
				}
				break;
			}
			case(CommandSymbol_e::Command_Single) : {
				break;
			}
			default: {
				run_result = false;
				break;
			}
			}
			if (!run_result)
				break;
		}
	}
	eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Success_r);

}

void Main::Model_Main_c::createThread_executeResult(eg::GlbRtrn& rtrn, Main::CommandData const commandString, size_t button_itr, size_t data_itr, size_t commandSet) {
	static std::string const _egNAME_FUNCTION_seg_ = "Main::Model_Main_c::createThread_executeResult";
	bool create_a_thread = true;
	if (commandString.command[0] == CommandSymbol_e::Command_Single) {
		svcWaitSynchronization(thread_executeResult_mutex, UINT64_MAX);
		for (auto&& element : thread_executeResult) {
			if (element.commandSet == commandSet) {
				create_a_thread = false;
				break;
			}
		}
		svcReleaseMutex(thread_executeResult_mutex);
	}
	if ((thread_executeResult_populus <= 8) && create_a_thread) {
		Thread threadHandle;
		int32_t thread_priority;
		ExecuteResult_a* thread_arg = new ExecuteResult_a;
		thread_arg->this_p = this;
		thread_arg->commandString = commandString;
		thread_arg->button_itr = button_itr;
		thread_arg->data_itr = data_itr;
		thread_arg->commandSet = commandSet;
		svcGetThreadPriority(&thread_priority, CUR_THREAD_HANDLE);
		thread_priority--;
		threadHandle = threadCreate(Model_Main_c::executeResult, static_cast<void*>(thread_arg), stackSize_executeResult, thread_priority, -2, true);
		if (!threadHandle) {
			eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Failure_r);
			return;
		}
	}
	eg_GlbRtrn_egResult(rtrn, eg::Main_Result_Success_r);
}

int16_t Main::Model_Main_c::to_int16(std::vector<uint8_t> const& data, size_t& data_itr) {
	uint8_int16_u convert;
	convert.uint8_v[0] = data[data_itr]; data_itr++;
	convert.uint8_v[1] = data[data_itr]; data_itr++;
	return(convert.int16_v);
}

uint32_t Main::Model_Main_c::to_uint32(std::vector<uint8_t> const& data, size_t& data_itr) {
	uint8_uint32_u convert;
	convert.uint8_v[0] = data[data_itr]; data_itr++;
	convert.uint8_v[1] = data[data_itr]; data_itr++;
	convert.uint8_v[2] = data[data_itr]; data_itr++;
	convert.uint8_v[3] = data[data_itr]; data_itr++;
	return(convert.uint32_v);
}

Main::Stick_Position Main::Model_Main_c::uint8_to_stickPosition(std::vector<uint8_t> const& data, size_t& data_itr) {
	Stick_Position stick_pos;
	uint8_int16_u convert;
	convert.uint8_v[0] = data[data_itr]; data_itr++;
	convert.uint8_v[1] = data[data_itr]; data_itr++;
	stick_pos.pos_x = convert.int16_v;
	convert.uint8_v[0] = data[data_itr]; data_itr++;
	convert.uint8_v[1] = data[data_itr]; data_itr++;
	stick_pos.pos_y = convert.int16_v;
	return(stick_pos);
}

bool Main::Model_Main_c::check_stickEvent(ButtonState_Compare_e compare, int16_t const& current, int16_t const& previous, int16_t const& range0, int16_t const& range1) {
	switch (compare) {
	case(ButtonState_Compare_e::Command_Held) : {
		if ((current >= range0) && (current <= range1))
			return(true);
		break;
		}
	case(ButtonState_Compare_e::Command_Pressed) : {
		if (((current >= range0) && (current <= range1)) && (!((previous >= range0) && (previous <= range1))))
			return(true);
		break;
	}
	case(ButtonState_Compare_e::Command_Released) : {
		if ((!((current >= range0) && (current <= range1))) && ((previous >= range0) && (previous <= range1)))
			return(true);
		break;
	}
	default: {
		break;
	}
	}
	return(false);
}
