#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <string>
#include "stdafx.h"
#include "Parm.h"
#include "IT.h"
#include "LT.h"
#include "Lexical_analysis.h"
#include "In.h"
#include "Error.h"
#include "FST.h"

LT::LexTable __LexTable = LT::Create(LT_MAXSIZE - 1);
IT::IdTable __IdTable = IT::Create(TI_MAXSIZE - 1);
std::fstream file;

namespace Lex_analysis
{
	void ITLog(IT::IdTable it, Log::LOG log)
	{
		int i, numberOP = 0;
		*log.stream << "\nТаблица идентификаторов:\n\n";
		*log.stream << "№   " << " | " << "Идентификатор" << " | " << "Тип данных" << " | " << "Тип идентификатора" << " | " << "Индекс в ТЛ" << " | " << " Значение   " << endl;
		*log.stream << setw(87) << endl;
		for (i = 0; i < it.size; i++)
		{
			*log.stream << setfill('0') << setw(4) << right << i << "    ";
			*log.stream << setfill(' ') << setw(13) << left << it.table[i].id;
			switch (it.table[i].iddatatype)
			{
			case IT::INT: *log.stream << setw(10) << left;
				if (it.table[i].idtype == IT::OP)*log.stream << "-";
				else *log.stream << "integer   "; break;
			case IT::STR: *log.stream << setw(10) << left;
				if (it.table[i].idtype == IT::OP) *log.stream << "-";
				else *log.stream << "string    "; break;
			default: *log.stream << setw(10) << std::left << "unknown"; break;
			}
			switch (it.table[i].idtype)
			{
			case IT::V: *log.stream << left << "\t\tпеременная\t"; break;
			case IT::F: *log.stream << left << "\t\tфункция\t\t"; break;
			case IT::P: *log.stream << left << "\t\tпараметр\t"; break;
			case IT::L: *log.stream << left << "\t\tлитерал\t\t"; break;
			case IT::OP: *log.stream << left << "\t\tоператор\t\t"; break;
				break;
			default: *log.stream << left << "\t\tunknown\t\t\t"; break;
			}
			*log.stream << setw(11) << left << it.table[i].idxfirstLE;
			if (it.table[i].iddatatype == IT::INT && (it.table[i].idtype == IT::V || it.table[i].idtype == IT::L))
				*log.stream << setw(18) << left << it.table[i].value.vint;
			else if (it.table[i].iddatatype == IT::STR && (it.table[i].idtype == IT::V || it.table[i].idtype == IT::L))
				*log.stream << "[" << it.table[i].value.vstr->len << "]" << " \"" << it.table[i].value.vstr->str << "\" " << "";
			else if (it.table[i].idtype == IT::OP)
				*log.stream << setw(18) << left << it.table[i].value.op;
			else
				*log.stream << "       -       ";
			*log.stream << endl;
		}

		*log.stream << "\nКоличество идентификаторов:\n " << i << endl;

	}


	void LTLog(LT::LexTable lt, Log::LOG log)
	{

		int line = 1;
		*log.stream << "\n";
		*log.stream << "Таблица лексем:\n\n";
		*log.stream << line << "  ";
		for (int i = 0; i < lt.size; i++)
		{
			if (lt.table[i].lexema[0] == ';' || lt.table[i].lexema[0] == '{')
			{
				*log.stream << lt.table[i].lexema[0];
				line++;
				*log.stream << '\n';
				*log.stream << line << "  ";
				continue;
			}

			*log.stream << lt.table[i].lexema[0] << "\t\t" << "idxTI" << lt.table[i].idxTI  << endl;
		}
	}

	void Add_Operator_To_LT(int line, char c, char _operatorName)
	{
		LT::Entry entry;
		entry.sn = line;
		entry.idxTI = __IdTable.size;
		memset(entry.lexema, '\0', strlen(entry.lexema) - 1);
		entry.lexema[0] = c;
		entry.priority = Get_Priority(_operatorName);
		LT::Add(__LexTable, entry);
	}

	void Add_To_LT(int line, char c)
	{
		LT::Entry entry;
		entry.sn = line;
		if (c != 'i')
			entry.idxTI = LT_TI_NULLIDX;
		else
			entry.idxTI = __IdTable.size;
		memset(entry.lexema, '\0', strlen(entry.lexema) - 1);
		entry.lexema[0] = c;
		LT::Add(__LexTable, entry);
	}

	void Add_To_LT_INDEF(int line, char c, int idx)
	{
		LT::Entry entry;
		entry.sn = line;
		entry.idxTI = idx;
		memset(entry.lexema, '\0', strlen(entry.lexema) - 1);
		entry.lexema[0] = c;
		LT::Add(__LexTable, entry);
	}

	void Add_To_LT_LITERAL(int line, char c, int idx)
	{
		LT::Entry entry;
		entry.sn = line;
		entry.idxTI = idx;
		memset(entry.lexema, '\0', strlen(entry.lexema) - 1);
		entry.lexema[0] = c;
		LT::Add(__LexTable, entry);
	}



	LEX Lexical_analysis(Parm::PARM parm, In::IN in)
	{
		LEX lex;
		LEX_FLAGS _LEX_FLAGS;

		IT::Entry _IT_entry;
		char tmp_char;
		char tmp_word[127] = "";
		int line = 0;
		short k = 0;
		char tmp_func[ID_MAXSIZE] = "";
		char tmp_func_external[ID_MAXSIZE * 2] = "";
		char tmp_func_standart_library[ID_MAXSIZE * 2] = "";
		unsigned short literal_count = 1;
		int idx;
		bool findSameLiteral = false;
		int litValue;
		bool findLibraryFunc = false;

		for (int i = 0; i < in.size; i++)
		{
			tmp_char = in.text[i];
			if (tmp_char == IN_CODE_ENDL)
			{
				line++;
				continue;
			}

			if (tmp_char != ' ')
			{
				tmp_word[k] = tmp_char;
				k++;
				continue;
			}
			else
			{
				tmp_word[k] = '\0';
				if ((tmp_word[0] == '\0') || (tmp_word[0] == ' ' && tmp_word[1] == '\0'))
					continue;
				k = 0;
				//для перевода через бубну
				std::string lit_value_str_ = "", lit_value_str2_ = "";
				char lit_value_str_buf_[TI_STR_MAXSIZE - 1] = "";
				switch (Proverka_FST(tmp_word))
				{
				case _TYPEINT:
					Add_To_LT(line, LEX_INTEGER);
					_LEX_FLAGS.__flag_int = true;
					break;
				case _TYPESTRING:
					Add_To_LT(line, LEX_STRING);
					_LEX_FLAGS.__flag_string = true;
					break;
				case _TYPEFUNCTION:
					Add_To_LT(line, LEX_FUNCTION);
					if ((_LEX_FLAGS.__flag_function && _LEX_FLAGS.__flag_declare) && (_LEX_FLAGS.__flag_int || _LEX_FLAGS.__flag_string))
						_LEX_FLAGS.__flag_function_external = true;
					_LEX_FLAGS.__flag_function = true;
					break;
				case _STRDECLARE:
					Add_To_LT(line, LEX_DECLARE);
					_LEX_FLAGS.__flag_declare = true;
					break;
				case _STRRETURN:
					Add_To_LT(line, LEX_RETURN);
					findLibraryFunc = false;
					break;
				case _STRMAIN:
					Add_To_LT(line, LEX_MAINFUNC);
					strcpy_s(tmp_func, tmp_word);
					_LEX_FLAGS.__flag_function = true;
					break;
				case _STRPRINT:
					Add_To_LT(line, LEX_PRINT);
					break;
				case _LEFTBRACE:
					Add_To_LT(line, LEX_LEFTBRACE);
					break;
				case _RIGHTBRACE:
					Add_To_LT(line, LEX_BRACELET);
					_LEX_FLAGS.__flag_function = false;
					break;
				case _LEFTTHESIS:
					Add_To_LT(line, LEX_LEFTHESIS);
					_LEX_FLAGS.__flag_int = false;
					_LEX_FLAGS.__flag_string = false;
					if (_LEX_FLAGS.__flag_function || _LEX_FLAGS.__flag_function_external || _LEX_FLAGS.__flag_standard_library)
						_LEX_FLAGS.__flag_parameter = true;
					break;
				case _RIGHTTHESIS:
					Add_To_LT(line, LEX_RIGHTHESIS);
					_LEX_FLAGS.__flag_int = false;
					_LEX_FLAGS.__flag_string = false;
					_LEX_FLAGS.__flag_parameter = false;
					break;
				case _SEMICOLON:
					Add_To_LT(line, LEX_SEMICOLON);
					_LEX_FLAGS.__flag_int = false;
					_LEX_FLAGS.__flag_string = false;
					_LEX_FLAGS.__flag_parameter = false;
					_LEX_FLAGS.__flag_declare = false;
					_LEX_FLAGS.__flag_function_external = false;
					_LEX_FLAGS.__flag_standard_library = false;
					break;
				case _COMMA:
					Add_To_LT(line, LEX_COMMA);
					_LEX_FLAGS.__flag_int = false;
					_LEX_FLAGS.__flag_string = false;
					break;
				case _PLUS:
					Add_Operator_To_LT(line, LEX_PLUS, '+');
					_LEX_FLAGS.__flag_operator = true;
					break;
				case _MINUS:
					Add_Operator_To_LT(line, LEX_MINUS, '-');
					_LEX_FLAGS.__flag_operator = true;
					break;
				case _DEL:
					Add_Operator_To_LT(line, LEX_DIRSLASH, '/');
					_LEX_FLAGS.__flag_operator = true;
					break;
				case _UMN:
					Add_Operator_To_LT(line, LEX_STAR, '*');
					_LEX_FLAGS.__flag_operator = true;
					break;
				case _PERCENT:
					Add_Operator_To_LT(line, LEX_PERCENT, '%');
					_LEX_FLAGS.__flag_operator = true;
					break;
				case _RAVNO:
					Add_To_LT(line, LEX_RAV);
					break;
				case _LITERAL_INT:
					litValue = atoi(tmp_word);
					for (int k = 0; k < __IdTable.size; k++)
					{
						if (__IdTable.table[k].value.vint == litValue &&
							__IdTable.table[k].idtype == IT::L &&
							__IdTable.table[k].iddatatype == IT::INT)
						{
							Add_To_LT_LITERAL(line, LEX_LITERAL, k);
							findSameLiteral = true;
							break;
						}
					}
					if(!findSameLiteral)
						Add_To_LT_LITERAL(line, LEX_LITERAL, __IdTable.size);
					break;
				case _LITERAL_STR:


					// перевод через бубну
					memset(lit_value_str_buf_, '\0', strlen(lit_value_str_buf_));
					lit_value_str_ = tmp_word;
					std::remove_copy(lit_value_str_.begin(), lit_value_str_.end(), std::back_inserter(lit_value_str2_), '\'');
					strcpy(lit_value_str_buf_, lit_value_str2_.c_str());

					for (int k = 0; k < __IdTable.size; k++)
					{
						if ((strcmp(__IdTable.table[k].value.vstr->str, lit_value_str_buf_) == 0) &&
							__IdTable.table[k].idtype == IT::L &&
							__IdTable.table[k].iddatatype == IT::STR)
						{
							Add_To_LT_LITERAL(line, LEX_LITERAL, k);
							findSameLiteral = true;
							break;
						}
					}
					if (!findSameLiteral)
						Add_To_LT_LITERAL(line, LEX_LITERAL, __IdTable.size);
					break;
				case _FSTIDENF:
					if (IT::IsId(__IdTable, tmp_word) != (int)TI_NULLIDX)
					{
						if (__IdTable.table[IT::IsId(__IdTable, tmp_word)].idtype == IT::F)
						{
							Add_To_LT_INDEF(line, LEX_ID, IT::IsId(__IdTable, tmp_word));
						}
					}
					else if (_LEX_FLAGS.__flag_function)
					{
						char buffer_parm2[ID_MAXSIZE * 2] = "";
						strcat(buffer_parm2, tmp_func);
						strcat(buffer_parm2, tmp_word);
						if (IT::IsId(__IdTable, buffer_parm2) == (int)TI_NULLIDX)
							Add_To_LT(line, LEX_ID);
						else
							Add_To_LT_INDEF(line, LEX_ID, IT::IsId(__IdTable, buffer_parm2));
					}
					break;
				case _STANDART_LIBRARY:
					if (findLibraryFunc)
						Add_To_LT_INDEF(line, LEX_ID, IT::IsId(__IdTable, tmp_word));
					else
					Add_To_LT(line, LEX_ID);
					_LEX_FLAGS.__flag_standard_library = true;
					findLibraryFunc = true;
					break;
				default:
					throw ERROR_THROW(117);
				}
			}

			// функция стандартной библиотеки ( F )
			if (_LEX_FLAGS.__flag_standard_library && !_LEX_FLAGS.__flag_parameter && Proverka_FST(tmp_word) == _STANDART_LIBRARY && _LEX_FLAGS.__flag_declare)
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				strcpy_s(_IT_entry.id, tmp_word);
				strcpy_s(tmp_func_standart_library, tmp_word);
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::F;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// переменная функции стандартной библиотеки ( P )
			if (_LEX_FLAGS.__flag_standard_library && _LEX_FLAGS.__flag_parameter && Proverka_FST(tmp_word) == _FSTIDENF && _LEX_FLAGS.__flag_declare)
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				if (strlen(tmp_word) > ID_MAXSIZE)
				{
					throw ERROR_THROW(118);
				}
				else
				{
					char buffer_parm[ID_MAXSIZE * 2] = "";
					strcat(buffer_parm, tmp_func_standart_library);
					strcat(buffer_parm, tmp_word);
					strcpy_s(_IT_entry.id, buffer_parm);
				}
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::P;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// переменная ( V )
			if ((_LEX_FLAGS.__flag_int || _LEX_FLAGS.__flag_string) && (!_LEX_FLAGS.__flag_parameter && Proverka_FST(tmp_word) == _FSTIDENF)
				&& (_LEX_FLAGS.__flag_declare) && (!_LEX_FLAGS.__flag_function_external))
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				if (_LEX_FLAGS.__flag_function)
				{
					char buffer_parm[ID_MAXSIZE * 2] = "";
					strcat(buffer_parm, tmp_func);
					strcat(buffer_parm, tmp_word);
					if (IT::IsId(__IdTable, buffer_parm) == (int)TI_NULLIDX)
					{
						strcpy_s(_IT_entry.id, buffer_parm);
					}
					else
					{
						//throw (уже данный идентификатор заняты)
					}
				}
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::V;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// функция ( F ) 
			if ((_LEX_FLAGS.__flag_int || _LEX_FLAGS.__flag_string) && (_LEX_FLAGS.__flag_function && Proverka_FST(tmp_word) == _FSTIDENF)
				&& (!_LEX_FLAGS.__flag_parameter) && (!_LEX_FLAGS.__flag_declare))
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				if (strlen(tmp_word) > ID_MAXSIZE)
				{
					throw ERROR_THROW(118);
				}
				else
				{
					strcpy_s(_IT_entry.id, tmp_word);
					strcpy_s(tmp_func, tmp_word);
				}
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::F;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// функция (ВНЕШНЯЯ) ( F ) 
			if (_LEX_FLAGS.__flag_function_external && Proverka_FST(tmp_word) == _FSTIDENF && !_LEX_FLAGS.__flag_parameter)
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				strcpy_s(_IT_entry.id, tmp_word);
				strcpy_s(tmp_func_external, tmp_word);
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::F;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// параметр ( P )
			if ((_LEX_FLAGS.__flag_int || _LEX_FLAGS.__flag_string) && (_LEX_FLAGS.__flag_function && Proverka_FST(tmp_word) == _FSTIDENF) && (_LEX_FLAGS.__flag_parameter))
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				//если функция внешняя
				if (_LEX_FLAGS.__flag_function_external)
				{
					if (strlen(tmp_word) > ID_MAXSIZE)
					{
						throw ERROR_THROW(118);
					}
					else
					{
						char buffer_parm[ID_MAXSIZE * 2] = "";
						strcat(buffer_parm, tmp_func_external);
						strcat(buffer_parm, tmp_word);
						strcpy_s(_IT_entry.id, buffer_parm);
					}
					if (_LEX_FLAGS.__flag_int)
					{
						_IT_entry.iddatatype = IT::INT;
						_IT_entry.value.vint = 0;
					}
					if (_LEX_FLAGS.__flag_string)
					{
						_IT_entry.iddatatype = IT::STR;
						strcpy_s(_IT_entry.value.vstr->str, "");
					}
					_IT_entry.idtype = IT::P;
					_IT_entry.idxfirstLE = __LexTable.size;

					IT::Add(__IdTable, _IT_entry);
					goto next;
				}
				//если функция не внешняя
				if (strlen(tmp_word) > ID_MAXSIZE)
				{
					throw ERROR_THROW(118);
				}
				else
				{
					char buffer_parm[ID_MAXSIZE * 2] = "";
					strcat(buffer_parm, tmp_func);
					strcat(buffer_parm, tmp_word);
					strcpy_s(_IT_entry.id, buffer_parm);
				}
				if (_LEX_FLAGS.__flag_int)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = 0;
				}
				if (_LEX_FLAGS.__flag_string)
				{
					_IT_entry.iddatatype = IT::STR;
					strcpy_s(_IT_entry.value.vstr->str, "");
				}
				_IT_entry.idtype = IT::P;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			// литерал ( L )
			if ((Proverka_FST(tmp_word) == _LITERAL_INT || Proverka_FST(tmp_word) == _LITERAL_STR) && !findSameLiteral)
			{
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				char lit_numb[4];
				if (Proverka_FST(tmp_word) == _LITERAL_INT)
				{
					_IT_entry.iddatatype = IT::INT;
					_IT_entry.value.vint = atoi(tmp_word);
				}
				if (Proverka_FST(tmp_word) == _LITERAL_STR)
				{
					_IT_entry.iddatatype = IT::STR;

					// перевод через бубну
					std::string lit_value_str = tmp_word, lit_value_str2;
					std::remove_copy(lit_value_str.begin(), lit_value_str.end(), std::back_inserter(lit_value_str2), '\'');
					if (lit_value_str2.length() > TI_STR_MAXSIZE - 1)
					{
						throw ERROR_THROW(119);
					}
					char lit_value_str_buf[TI_STR_MAXSIZE - 1];
					strcpy(lit_value_str_buf, lit_value_str2.c_str());
					strcpy_s(_IT_entry.value.vstr->str, lit_value_str_buf);
					_IT_entry.value.vstr->len = strlen(_IT_entry.value.vstr->str);
				}
				itoa(literal_count, lit_numb, 10);
				char buffer_parm[ID_MAXSIZE * 2] = "";
				strcat(buffer_parm, "L");
				strcat(buffer_parm, lit_numb);
				strcpy_s(_IT_entry.id, buffer_parm);

				_IT_entry.idtype = IT::L;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				literal_count++;
				goto next;
			}
			// оператор ( OP )
			if (_LEX_FLAGS.__flag_operator)
			{
				_LEX_FLAGS.__flag_operator = false;
				memset(_IT_entry.id, '\0', ID_MAXSIZE * 2);
				switch (Proverka_FST(tmp_word))
				{
				case _PLUS:
					_IT_entry.value.op = '+';
					break;
				case _MINUS:
					_IT_entry.value.op = '-';
					break;
				case _DEL:
					_IT_entry.value.op = '/';
					break;
				case _UMN:
					_IT_entry.value.op = '*';
					break;
				case _PERCENT:
					_IT_entry.value.op = '%';
					break;
				}
				_IT_entry.iddatatype = IT::INT;
				_IT_entry.idtype = IT::OP;
				_IT_entry.idxfirstLE = __LexTable.size;

				IT::Add(__IdTable, _IT_entry);
				goto next;
			}
			if (Proverka_FST(tmp_word) == _STANDART_LIBRARY)
			{
				idx = IsId(__IdTable, tmp_word);
				if (idx != (int)TI_NULLIDX)
					__LexTable.table[__LexTable.size].idxTI = idx;
				goto next;
			}
		next:

			findSameLiteral = false;
			if (!_LEX_FLAGS.__flag_standard_library)
				memset(tmp_func_standart_library, '\0', strlen(tmp_func_standart_library));
			if (!_LEX_FLAGS.__flag_function_external)
				memset(tmp_func_external, '\0', strlen(tmp_func_external));
			if (!_LEX_FLAGS.__flag_function)
				memset(tmp_func, '\0', strlen(tmp_func));
			memset(tmp_word, '\0', strlen(tmp_word));
		}

		std::fstream file;
		//std::ofstream Out(parm.out);
		//Out << in.text;
		//Out.close();
		file.open(parm.out);
		if (file.fail())
		{
			throw ERROR_THROW(112);
		}
		file << '0' << __LexTable.table[0].sn + 1 << ' ';


		for (int i = 0; i < __LexTable.size; i++)
		{
			if (__LexTable.table[i].lexema[0] == '|')
			{
				file << '\n';
				if (i != (__LexTable.size - 1))
				{
					if (__LexTable.table[i].sn + 1 < 10)
						file << '0' << __LexTable.table[i].sn + 1 << ' ';
					else
						file << __LexTable.table[i].sn + 1 << ' ';
				}
				continue;
			}
			file << __LexTable.table[i].lexema;

		}

		file << std::endl << std::endl;
		file << std::setw(15) << "|Name" << std::setw(2) << '|';
		file << std::setw(10) << "|Type" << std::setw(2) << '|';
		file << std::setw(15) << "|ID type" << std::setw(2) << '|';
		file << std::setw(15) << "|Line" << std::setw(2) << '|';
		file << std::setw(15) << "|Value" << std::setw(2) << '|';
		file << std::setw(15) << "|Length" << std::setw(2) << '|';
		//file << std::setw(15) << "|Area" << std::setw(2) << '|' << '\n';
		for (int i = 0; i < __IdTable.size; i++)
		{
			file << std::setw(15) << __IdTable.table[i].id << std::setw(2);

			if (__IdTable.table[i].iddatatype == IT::INT)
				file << std::setw(15) << "integer" << std::setw(2);
			if (__IdTable.table[i].iddatatype == IT::STR)
				file << std::setw(15) << "string" << std::setw(2);

			if (__IdTable.table[i].idtype == IT::F)
				file << std::setw(15) << "Функция" << std::setw(2);
			if (__IdTable.table[i].idtype == IT::P)
				file << std::setw(15) << "Параметр" << std::setw(2);
			if (__IdTable.table[i].idtype == IT::L)
				file << std::setw(15) << "Литерал" << std::setw(2);
			if (__IdTable.table[i].idtype == IT::V)
				file << std::setw(15) << "Переменная" << std::setw(2);

			file << std::setw(15) << __IdTable.table[i].idxfirstLE << std::setw(2);

			if (__IdTable.table[i].iddatatype == IT::INT)
				file << std::setw(15) << __IdTable.table[i].value.vint << std::setw(2) << std::setw(15) << " " << std::setw(2);
			else
			{
				file << std::setw(15) << "\"" << __IdTable.table[i].value.vstr->str << "\"" << std::setw(2);
				file << std::setw(15) << "[" << __IdTable.table[i].value.vstr->len << "]" << std::setw(2);

			}
			file << std::endl << std::endl;
		}
		file.close();

		lex.idtable = __IdTable;
		lex.lextable = __LexTable;
		return lex;
	}
	short Proverka_FST(char* c)
	{
		FST::FST typeint(c,
			8,
			FST::NODE(1, FST::RELATION('i', 1)),
			FST::NODE(1, FST::RELATION('n', 2)),
			FST::NODE(1, FST::RELATION('t', 3)),
			FST::NODE(1, FST::RELATION('e', 4)),
			FST::NODE(1, FST::RELATION('g', 5)),
			FST::NODE(1, FST::RELATION('e', 6)),
			FST::NODE(1, FST::RELATION('r', 7)),
			FST::NODE()
		);

		FST::FST typestring(c,
			7,
			FST::NODE(1, FST::RELATION('s', 1)),
			FST::NODE(1, FST::RELATION('t', 2)),
			FST::NODE(1, FST::RELATION('r', 3)),
			FST::NODE(1, FST::RELATION('i', 4)),
			FST::NODE(1, FST::RELATION('n', 5)),
			FST::NODE(1, FST::RELATION('g', 6)),
			FST::NODE()
		);

		FST::FST typefunction(c,
			9,
			FST::NODE(1, FST::RELATION('f', 1)),
			FST::NODE(1, FST::RELATION('u', 2)),
			FST::NODE(1, FST::RELATION('n', 3)),
			FST::NODE(1, FST::RELATION('c', 4)),
			FST::NODE(1, FST::RELATION('t', 5)),
			FST::NODE(1, FST::RELATION('i', 6)),
			FST::NODE(1, FST::RELATION('o', 7)),
			FST::NODE(1, FST::RELATION('n', 8)),
			FST::NODE()
		);

		FST::FST strdeclare(c,
			8,
			FST::NODE(1, FST::RELATION('d', 1)),
			FST::NODE(1, FST::RELATION('e', 2)),
			FST::NODE(1, FST::RELATION('c', 3)),
			FST::NODE(1, FST::RELATION('l', 4)),
			FST::NODE(1, FST::RELATION('a', 5)),
			FST::NODE(1, FST::RELATION('r', 6)),
			FST::NODE(1, FST::RELATION('e', 7)),
			FST::NODE()
		);

		FST::FST streturn(c,
			7,
			FST::NODE(1, FST::RELATION('r', 1)),
			FST::NODE(1, FST::RELATION('e', 2)),
			FST::NODE(1, FST::RELATION('t', 3)),
			FST::NODE(1, FST::RELATION('u', 4)),
			FST::NODE(1, FST::RELATION('r', 5)),
			FST::NODE(1, FST::RELATION('n', 6)),
			FST::NODE()
		);

		FST::FST strmain(c,
			5,
			FST::NODE(1, FST::RELATION('m', 1)),
			FST::NODE(1, FST::RELATION('a', 2)),
			FST::NODE(1, FST::RELATION('i', 3)),
			FST::NODE(1, FST::RELATION('n', 4)),
			FST::NODE()
		);

		FST::FST strprint(c,
			6,
			FST::NODE(1, FST::RELATION('p', 1)),
			FST::NODE(1, FST::RELATION('r', 2)),
			FST::NODE(1, FST::RELATION('i', 3)),
			FST::NODE(1, FST::RELATION('n', 4)),
			FST::NODE(1, FST::RELATION('t', 5)),
			FST::NODE()
		);

		FST::FST leftbrace(c,
			2,
			FST::NODE(1, FST::RELATION('{', 1)),
			FST::NODE()
		);

		FST::FST rightbrace(c,
			2,
			FST::NODE(1, FST::RELATION('}', 1)),
			FST::NODE()
		);

		FST::FST leftthesis(c,
			2,
			FST::NODE(1, FST::RELATION('(', 1)),
			FST::NODE()
		);

		FST::FST rightthesis(c,
			2,
			FST::NODE(1, FST::RELATION(')', 1)),
			FST::NODE()
		);

		FST::FST semicolon(c,
			2,
			FST::NODE(1, FST::RELATION(';', 1)),
			FST::NODE()
		);

		FST::FST comma(c,
			2,
			FST::NODE(1, FST::RELATION(',', 1)),
			FST::NODE()
		);

		FST::FST _plus(c,
			2,
			FST::NODE(1, FST::RELATION('+', 1)),
			FST::NODE());

		FST::FST _minus(c,
			2,
			FST::NODE(1, FST::RELATION('-', 1)),
			FST::NODE());

		FST::FST divider(c,
			2,
			FST::NODE(1, FST::RELATION('/', 1)),
			FST::NODE());

		FST::FST umn(c,
			2,
			FST::NODE(1, FST::RELATION('*', 1)),
			FST::NODE());

		FST::FST ravno(c,
			2,
			FST::NODE(1, FST::RELATION('=', 1)),
			FST::NODE());

		FST::FST Literal_int(c,
			2,
			FST::NODE(20,
				FST::RELATION('0', 0), FST::RELATION('1', 0), FST::RELATION('2', 0),
				FST::RELATION('3', 0), FST::RELATION('4', 0), FST::RELATION('5', 0),
				FST::RELATION('6', 0), FST::RELATION('7', 0), FST::RELATION('8', 0),
				FST::RELATION('9', 0), FST::RELATION('0', 1), FST::RELATION('1', 1),
				FST::RELATION('2', 1), FST::RELATION('3', 1), FST::RELATION('4', 1),
				FST::RELATION('5', 1), FST::RELATION('6', 1), FST::RELATION('7', 1),
				FST::RELATION('8', 1), FST::RELATION('9', 1)),
			FST::NODE()
		);

		FST::FST Literal_str(c,
			4,
			FST::NODE(1, FST::RELATION('\'', 1)),
			FST::NODE(20,
				FST::RELATION('0', 1), FST::RELATION('1', 1), FST::RELATION('2', 1),
				FST::RELATION('3', 1), FST::RELATION('4', 1), FST::RELATION('5', 1),
				FST::RELATION('6', 1), FST::RELATION('7', 1), FST::RELATION('8', 1),
				FST::RELATION('9', 1), FST::RELATION('0', 2), FST::RELATION('1', 2),
				FST::RELATION('2', 2), FST::RELATION('3', 2), FST::RELATION('4', 2),
				FST::RELATION('5', 2), FST::RELATION('6', 2), FST::RELATION('7', 2),
				FST::RELATION('8', 2), FST::RELATION('9', 2)),
			FST::NODE(1, FST::RELATION('\'', 3)),
			FST::NODE()
		);

		FST::FST fstidenf(c,
			2,
			FST::NODE(54,
				FST::RELATION('a', 1), FST::RELATION('a', 0), FST::RELATION('b', 1), FST::RELATION('b', 0),
				FST::RELATION('c', 1), FST::RELATION('c', 0), FST::RELATION('d', 1), FST::RELATION('d', 0), FST::RELATION('e', 1), FST::RELATION('e', 0),
				FST::RELATION('f', 1), FST::RELATION('f', 0), FST::RELATION('g', 1), FST::RELATION('g', 0), FST::RELATION('h', 0), FST::RELATION('h', 1), FST::RELATION('i', 0), FST::RELATION('i', 1),
				FST::RELATION('j', 0), FST::RELATION('j', 1), FST::RELATION('k', 0), FST::RELATION('k', 1), FST::RELATION('l', 0), FST::RELATION('l', 1),
				FST::RELATION('m', 0), FST::RELATION('m', 1), FST::RELATION('n', 0), FST::RELATION('n', 1), FST::RELATION('o', 0), FST::RELATION('o', 1),
				FST::RELATION('p', 0), FST::RELATION('p', 1), FST::RELATION('q', 0), FST::RELATION('q', 1), FST::RELATION('r', 0), FST::RELATION('r', 1),
				FST::RELATION('s', 0), FST::RELATION('s', 1), FST::RELATION('t', 0), FST::RELATION('t', 1), FST::RELATION('u', 0), FST::RELATION('u', 1),
				FST::RELATION('v', 0), FST::RELATION('v', 1), FST::RELATION('w', 0), FST::RELATION('w', 1), FST::RELATION('x', 0), FST::RELATION('x', 1),
				FST::RELATION('y', 0), FST::RELATION('y', 1), FST::RELATION('z', 0), FST::RELATION('z', 1)),
			FST::NODE()
		);

		FST::FST strlen_func(c,
			7,
			FST::NODE(1, FST::RELATION('s', 1)),
			FST::NODE(1, FST::RELATION('t', 2)),
			FST::NODE(1, FST::RELATION('r', 3)),
			FST::NODE(1, FST::RELATION('l', 4)),
			FST::NODE(1, FST::RELATION('e', 5)),
			FST::NODE(1, FST::RELATION('n', 6)),
			FST::NODE()
		);

		FST::FST substr_func(c,
			7,
			FST::NODE(1, FST::RELATION('s', 1)),
			FST::NODE(1, FST::RELATION('u', 2)),
			FST::NODE(1, FST::RELATION('b', 3)),
			FST::NODE(1, FST::RELATION('s', 4)),
			FST::NODE(1, FST::RELATION('t', 5)),
			FST::NODE(1, FST::RELATION('r', 6)),
			FST::NODE()
		);

		typeint.string = c;
		if (FST::execute(typeint))
			return _TYPEINT;
		typestring.string = c;
		if (FST::execute(typestring))
			return _TYPESTRING;
		typefunction.string = c;
		if (FST::execute(typefunction))
			return _TYPEFUNCTION;
		strdeclare.string = c;
		if (FST::execute(strdeclare))
			return _STRDECLARE;
		streturn.string = c;
		if (FST::execute(streturn))
			return _STRRETURN;
		strmain.string = c;
		if (FST::execute(strmain))
			return _STRMAIN;
		strprint.string = c;
		if (FST::execute(strprint))
			return _STRPRINT;

		leftbrace.string = c;
		if (FST::execute(leftbrace))
			return _LEFTBRACE;
		rightbrace.string = c;
		if (FST::execute(rightbrace))
			return _RIGHTBRACE;
		leftthesis.string = c;
		if (FST::execute(leftthesis))
			return _LEFTTHESIS;
		rightthesis.string = c;
		if (FST::execute(rightthesis))
			return _RIGHTTHESIS;
		rightthesis.string = c;
		if (FST::execute(rightthesis))
			return _RIGHTTHESIS;
		semicolon.string = c;
		if (FST::execute(semicolon))
			return _SEMICOLON;
		comma.string = c;
		if (FST::execute(comma))
			return _COMMA;
		_plus.string = c;
		if (FST::execute(_plus))
			return _PLUS;
		_minus.string = c;
		if (FST::execute(_minus))
			return _MINUS;
		divider.string = c;
		if (FST::execute(divider))
			return _DEL;
		umn.string = c;
		if (FST::execute(umn))
			return _UMN;
		ravno.string = c;
		if (FST::execute(ravno))
			return _RAVNO;
		ravno.string = c;
		if (FST::execute(ravno))
			return _RAVNO;
		// функции стандартной библиотеки
		strlen_func.string = c;
		if (FST::execute(strlen_func))
			return _STANDART_LIBRARY;
		substr_func.string = c;
		if (FST::execute(substr_func))
			return _STANDART_LIBRARY;

		Literal_int.string = c;
		if (FST::execute(Literal_int))
			return _LITERAL_INT;
		Literal_str.string = c;
		if (FST::execute(Literal_str))
			return _LITERAL_STR;

		fstidenf.string = c;
		if (FST::execute(fstidenf))
			return _FSTIDENF;

		return 0;
	}

	short Get_Priority(char c)
	{
		switch (c)
		{ 
		case LEX_LEFTHESIS:
			return PRIORITY_LEFTTHESIS;
		case LEX_RIGHTHESIS:
			return PRIORITY_RIGHTTHESIS;
		case LEX_COMMA:
			return PRIORITY_COMMA;
		case '+':
			return PRIORITY_PLUS;
		case '-':
			return PRIORITY_MINUS;
		case '*':
			return PRIORITY_STAR;
		case '/':
			return PRIORITY_DIRSLASH;
		case '%':
			return PRIORITY_PERCENT;
		}
	}
}