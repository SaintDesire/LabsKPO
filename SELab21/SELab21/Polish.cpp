#include "stdafx.h"
#include <queue>
#include <stack>
#include "Polish.h"

namespace Polish {
	bool PolishNotation(int i, LT::LexTable& lextable, IT::IdTable& idtable)
	{
		std::stack<LT::Entry> stack;	// В стек будем заносить операции
		std::queue<LT::Entry> queue;	// В очередь будем заносить операнды

		LT::Entry placeholder_symbol;
		placeholder_symbol.idxTI = -1;
		placeholder_symbol.lexema[0] = ' ';
		placeholder_symbol.lexema[1] = '\0';
		placeholder_symbol.sn = lextable.table[i].sn;

		LT::Entry function_symbol;				// Лексема, обозначающая конец функции
		function_symbol.idxTI = LT_TI_NULLIDX;	// Признак начала и окончания функции
		function_symbol.lexema[0] = '@';
		function_symbol.lexema[1] = '\0';
		function_symbol.sn = lextable.table[i].sn;
		int idx;

		int lexem_counter = 0;
		int parm_counter = 0;			// Количество параметров в функции
		int lexem_position = i;			// Запоминаем номер лексемы перед преобразованием
		char* buf = new char[i];

		bool findFunc = false;			// флаг на нахождение функции

		for (i; lextable.table[i].lexema[0] != LEX_SEMICOLON; i++, lexem_counter++)
		{
			switch (lextable.table[i].lexema[0])
			{
			case LEX_ID:
			case LEX_LITERAL:
				if (idtable.table[lextable.table[i].idxTI].idtype == IT::F)
				{
					findFunc = true;
					idx = lextable.table[i].idxTI;
				}
				else
				{
					if (findFunc)
						parm_counter++;
					queue.push(lextable.table[i]);
				}
				continue;

			case LEX_LEFTHESIS:
				stack.push(lextable.table[i]);
				continue;

			case LEX_RIGHTHESIS:
				while (stack.top().lexema[0] != LEX_LEFTHESIS)
				{
					queue.push(stack.top());
					stack.pop();
					if (stack.empty())
						return false;
				}

				if (!findFunc)
					stack.pop();
				else {
					function_symbol.idxTI = idx;
					idx = LT_TI_NULLIDX;
					lextable.table[i] = function_symbol;
					queue.push(lextable.table[i]);
					_itoa_s(parm_counter, buf, 2, 10);
					stack.top().lexema[0] = buf[0];
					stack.top().idxTI = LT_TI_NULLIDX;
					stack.top().sn = function_symbol.sn;
					queue.push(stack.top());
					stack.pop();
					parm_counter = 0;
					findFunc = false;
				}
				continue;

			case LEX_OPERATOR:
				while (!stack.empty() && lextable.table[i].priority <= stack.top().priority)
				{
					queue.push(stack.top());
					stack.pop();
				}
				stack.push(lextable.table[i]);
				continue;
			}
		}

		while (!stack.empty())
		{
			if (stack.top().lexema[0] == LEX_LEFTHESIS || stack.top().lexema[0] == LEX_RIGHTHESIS)
				return false;

			queue.push(stack.top());
			stack.pop();
		}

		while (lexem_counter != 0)
		{
			if (!queue.empty())
			{
				lextable.table[lexem_position++] = queue.front();
				queue.pop();
			}
			else
				lextable.table[lexem_position++] = placeholder_symbol;

			lexem_counter--;
		}

		for (int i = 0; i < lexem_position; i++)
		{
			if (lextable.table[i].lexema[0] == LEX_OPERATOR || lextable.table[i].lexema[0] == LEX_LITERAL)
				idtable.table[lextable.table[i].idxTI].idxfirstLE = i;
		}

		return true;
	}

	void PolishStart(Lex_analysis::LEX lex)
	{
		for (int i = 0; i < lex.lextable.size; i++)
		{
			if (lex.lextable.table[i].lexema[0] == '=')
			{
				PolishNotation(i + 1, lex.lextable, lex.idtable);
			}
		}
	}
}