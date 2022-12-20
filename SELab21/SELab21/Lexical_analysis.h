#pragma once
#include "IT.h"
#include "LT.h"
#include "In.h"
#include "Parm.h"
#include "Log.h"

namespace Lex_analysis
{
	enum operations_priority
	{
		PRIORITY_LEFTTHESIS = 0,	// (
		PRIORITY_RIGHTTHESIS = 0,	// )
		PRIORITY_COMMA = 1,			// ,
		PRIORITY_PLUS = 2,			// +
		PRIORITY_MINUS = 2,			// -
		PRIORITY_STAR = 3,			// *
		PRIORITY_DIRSLASH = 3,		// /
		PRIORITY_PERCENT = 3,		// %
	};
	enum TYPEOFFST
	{
		_TYPEINT = 1, 
		_TYPESTRING, 
		_TYPEFUNCTION,
		_STRCREATE,
		_STRRETURN, 
		_STRMAIN,
		_STRPRINT, 
		_LEFTBRACE,
		_RIGHTBRACE, 
		_LEFTTHESIS,
		_RIGHTTHESIS, 
		_SEMICOLON, 
		_COMMA,
		_PLUS, 
		_MINUS, 
		_DEL,
		_UMN,
		_PERCENT,
		_RAVNO, 
		_LITERAL_INT, 
		_LITERAL_STR,
		_FSTIDENF, 
		_STANDART_LIBRARY
	};
	struct LEX_FLAGS
	{
		bool __flag_int = false; // (типы данных идентификаторов ) флаг integer
		bool __flag_string = false; //  (типы данных идентификаторов ) флаг string

		bool __flag_function = false; // ( типы идентификаторов ) флаг функции
		bool __flag_function_external = false; // ( типы идентификаторов ) флаг функции
		bool __flag_parameter = false; // ( типы идентификаторов ) флаг параметр

		// функции стандартой библиотеки
		//interger strlen (string) Ц длина строки;
		//string substr(string, integer, integer) Ц подстрока;
		bool __flag_standard_library = false;

		bool __flag_declare = false; // флаг declare

		bool __flag_operator = false; // флаг оператора
	};

	struct LEX {
		IT::IdTable idtable;
		LT::LexTable lextable;
	};

	short Proverka_FST(char* c);
	void Add_To_LT(int line, char c);
	LEX Lexical_analysis(Parm::PARM parm, In::IN in);
	short Get_Priority(char c);
	void LTLog(LT::LexTable lt, Log::LOG log);
	void ITLog(IT::IdTable it, Log::LOG log);
};