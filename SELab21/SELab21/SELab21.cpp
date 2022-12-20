#include "stdafx.h"
#include <iostream>
#include <locale>
#include <cwchar>

#include "Error.h"	//обработка ошибок
#include "Parm.h"	//обработка параметров
#include "Log.h"	//работа с протоколом
#include "In.h"		//ввод исходного файла
#include "Lexical_analysis.h"
#include "MFST.h"
#include "Polish.h"
#include "Generation.h"

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "rus");

	#define FILE "C:\\Users\\Admin\\Desktop\\лабы\\лабы 2_1\\КПО\\SELab21\\AsmCode\\AsmCode.asm"
	std::cout << " --- ТЕСТ IN::getin --- " << std::endl << std::endl;
	Log::LOG	log = Log::INITLOG;
	try
	{
		Parm::PARM parm = Parm::getparm(argc, argv);
		log = Log::getlog(parm.log);
		In::IN in = In::getin(parm.in);
		std::cout << in.text << std::endl;
		std::cout << "Всего символов: " << in.size << std::endl;
		std::cout << "Всего строк: " << in.lines << std::endl;
		std::cout << "Пропущенно: " << in.ignor << std::endl;
		//std::ofstream Out(parm.out);
		//Out << in.text;
		//Out.close();
		Lex_analysis::LEX lex = Lex_analysis::Lexical_analysis(parm, in);

		Lex_analysis::LTLog(lex.lextable, log);
		Lex_analysis::ITLog(lex.idtable, log);
		MFST::SyntaxCheck(lex);

		for (int i = 0; i < lex.lextable.size; i++)
		{

			if (lex.lextable.table[i].idxTI != -1)
				cout << i << ": " << lex.lextable.table[i].lexema << "\t+\t" << lex.idtable.table[lex.lextable.table[i].idxTI].id << endl;
			else
				cout << i << ": " << lex.lextable.table[i].lexema << endl;
		}
		Polish::PolishStart(lex);
		cout << "----------------------------PolishTest----------------------------" << endl;
		for (int i = 0; i < lex.lextable.size; i++)
		{
			if (lex.lextable.table[i].lexema[0] == ';' || lex.lextable.table[i].lexema[0] == '{' || lex.lextable.table[i].lexema[0] == '}')
				cout << lex.lextable.table[i].lexema << endl;
			else
				cout << lex.lextable.table[i].lexema;
		}

		CG::ConstSegment CnstS(lex.idtable.size);
		CG::DataSegment Data(lex.idtable.size);
		CG::CodeSegment Code;
		//CG::add(lex.idtable, CnstS);
		//CG::add(lex.idtable, Data);
		CG::CodeSegmentGeneration(Code, lex.lextable, lex.idtable);

		std::cout << "Генерация кода выполнена успешно." << std::endl;
		Log::Close(log);
	}
	catch (Error::ERROR e) {
		std::cout << "Ошибка " << e.id << ": " << e.message << std::endl << std::endl;
	};
	/*
	try
	{
		Parm::PARM parm = Parm::getparm(argc, argv);
		log = Log::getlog(parm.log);
		Log::WriteLine(log, (char*)"Тест:", (char*)" без ошибок \n", "");
		Log::WriteLine(log, (char*)"Тест:", (char*)" без ошибок \n", "");
		Log::WriteLog(log);
		Log::WriteParm(log, parm);
		In::IN in = In::getin(parm.in);
		Log::WriteIn(log, in);
		Log::Close(log);
	}
	catch (Error::ERROR e)
	{
		Log::WriteError(log, e);
	};*/

	//system("pause");
	return 0;
}

