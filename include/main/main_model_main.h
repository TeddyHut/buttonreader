#pragma once

#include <stdio.h>
#include <vector>
#include <sstream>
#include <3ds.h>
#include "../../engine/include/base/eg_engine.h"
#include "main_include.h"
#include "main_input.h"

namespace Main {
	class Model_Main_c : public eg::Model {
	public:
		void runover_init(void* rundata);
		void runover(void* rundata);

		Handle out_buttonData_mutex;
		ButtonData out_buttonData;

		Model_Main_c();
		virtual ~Model_Main_c();
	protected:
		enum class Stick_State_e : size_t {
			leftStick_x = 0,
			leftStick_y,
			rightStick_x,
			rightStick_y
		}; static size_t const Stick_State_z;
		struct ExecuteResult_a {
			Model_Main_c* this_p;
			CommandData commandString;
			size_t button_itr;
			size_t data_itr;
			size_t commandSet;
		};
		struct ThreadInfo {
			Thread thread;
			Handle frame_passed;
			size_t commandSet;
		};

		static size_t const stackSize_executeResult;
		static void executeResult(void* args);

		void merge_to_outbuf();
		void tickFrames();

		void process_commands(eg::GlbRtrn& rtrn);
		void createThread_executeResult(eg::GlbRtrn& rtrn, CommandData const commandString, size_t button_itr, size_t data_itr, size_t commandSet);

		static int16_t to_int16(std::vector<uint8_t> const& data, size_t& data_itr);
		static uint32_t to_uint32(std::vector<uint8_t> const& data, size_t& data_itr);
		static Stick_Position uint8_to_stickPosition(std::vector<uint8_t> const& data, size_t& data_itr);
		static bool check_stickEvent(ButtonState_Compare_e compare, int16_t const& current, int16_t const& previous, int16_t const& range0, int16_t const& range1);

		Handle main_stop_mutex;
		bool main_stop;
		Stick_Position leftStickPos_prev;
		Stick_Position rightStickPos_prev;
		ButtonData work_buttonData;
		Handle threadbuf_buttonData_mutex;
		ButtonData threadbuf_buttonData;
		std::vector<CommandData> command;
		Handle thread_executeResult_mutex;
		std::vector<ThreadInfo> thread_executeResult;
		volatile size_t thread_executeResult_populus;
		Input_c* input_main;
	};
}