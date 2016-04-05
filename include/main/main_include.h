#pragma once

#include <vector>
#include <sstream>
#include <cstdio>
#include <3ds.h>
#include <base/eg_engine.h>

namespace Main {
	union uint8_int16_u {
		uint8_t uint8_v[2];
		int16_t int16_v;
	};
	union uint8_uint32_u {
		uint8_t uint8_v[4];
		uint32_t uint32_v;
	};
	enum class ButtonState_e : size_t {
		Key_A = 0,
		Key_B,
		Key_X,
		Key_Y,
		Key_Up,
		Key_Down,
		Key_Left,
		Key_Right,
		Key_L,
		Key_R,
		Key_ZL,
		Key_ZR,
		Key_Start,
		Key_Select,
		Key_Home
	}; extern size_t const ButtonState_z;
	struct Rundata {
		PrintConsole* top;
		PrintConsole* bottom;
		Handle console_mutex;
		bool stop;
	};
	struct Stick_Position {
	public:
		Stick_Position& operator=(circlePosition const& npos);

		int16_t pos_x;	//Approx -9C (left) to 9C (right)
		int16_t pos_y;	//Approx -9C (botton) to 9C (top)
		Stick_Position();
		Stick_Position(int16_t npos_x, int16_t npos_y);
		Stick_Position(circlePosition const& npos);
	};
	struct ButtonData {
	public:
		eg::Param<ButtonState_e> button_held;
		eg::Param<ButtonState_e> button_pressed;
		eg::Param<ButtonState_e> button_released;
		Stick_Position leftStick_pos;
		Stick_Position rightStick_pos;
		ButtonData();
	};
	enum class CommandSymbol_e : uint8_t {
		Command_leftStickPos_x = 0,
		Command_leftStickPos_y,
		Command_rightStickPos_x,
		Command_rightStickPos_y,
		Command_leftStickPosRange_x_Held,
		Command_leftStickPosRange_y_Held,
		Command_rightStickPosRange_x_Held,
		Command_rightStickPosRange_y_Held,
		Command_leftStickPosRange_x_Pressed,
		Command_leftStickPosRange_y_Pressed,
		Command_rightStickPosRange_x_Pressed,
		Command_rightStickPosRange_y_Pressed,
		Command_leftStickPosRange_x_Released,
		Command_leftStickPosRange_y_Released,
		Command_rightStickPosRange_x_Released,
		Command_rightStickPosRange_y_Released,
		Command_Time_usec,
		Command_Held,
		Command_Pressed,
		Command_Released,
		Command_Result,
		Command_Single,
		Command_Stop
	};
	enum class ButtonState_Compare_e : uint8_t {
		Command_Held = 0,
		Command_Pressed,
		Command_Released
	};
	struct CommandData {
	public:
		std::vector<CommandSymbol_e> command;
		std::vector<ButtonState_e> button;
		std::vector<uint8_t> data;
	};
	void resultOutput(eg::GlbRtrn const& rtrn, std::stringstream const& rtrn_stream, void* rundata);
}