#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "Generation.h"
#include "In.h"
#include "Polish.h"
#include "Log.h"
#include <fstream>
#pragma once


namespace CG
{
	void	CodeSegmentGeneration(CodeSegment& CS, LT::LexTable l, IT::IdTable& iT)
	{

		bool ifmain = false, error = false, until = false, func = false;
		int expression_type = IT::INT;
		std::vector<LT::Entry> tempEntries;
		std::string func_name;
		std::string var_name_temp;
		std::string var_name;
		std::string str = "";
		std::string str_temp = "";
		int idxTI, untilParm;
		char separators[9] = "(){},;=\n";
		std::cout << "\n\n\n";
		for (int i = 0; i < l.size; i++)
		{
			str_temp.clear();
			while (!strchr(separators, l.table[i].lexema[0]))
			{
				str_temp += l.table[i++].lexema[0];
				if (l.table[i - 1].lexema[0] == 'i')
				{
					idxTI = l.table[i - 1].idxTI;

				}
				else if (l.table[i - 1].lexema[0] == 'p' || l.table[i - 1].lexema[0] == 'r')
					break;
			}
			cout << "ебаный рот " << str_temp << endl;
			if (l.table[i].lexema[0] == 'v')
			{
				str += "+";
			}
			else if (str_temp == "tfi")
			{
				func_name = iT.table[idxTI].id;

				str += "\nfunction " + func_name;

			}
			else if (str_temp == "ti") {
				int j = func_name.size();
				var_name_temp = (string)iT.table[idxTI].id;
				cout << "ебань" << var_name_temp << endl;
				for (int i = 0; i < var_name_temp.size() - j; i++) {
					var_name += var_name_temp[j++];
				}
				expression_type = IT::INT;
				str += "int " + var_name;
				var_name.clear();
				var_name_temp.clear();
			}/*
			else if (str_temp == "ti" && iT.table[idxTI].iddatatype == IT::STR) {

				expression_type = IT::STR;
				str += "string " + (char)iT.table[idxTI].id;
			}*/
			else if (str_temp == "dti") {
				str += "let " + (string)iT.table[idxTI].id;
			}
			else if (str_temp == "dtfi") {
				str += "function " + (string)iT.table[idxTI].id;
				/*func_name = iT.table[idxTI].id;*/
			}
			else if (str_temp == "m") {
				ifmain = true;
				func_name = "main";
				str += "\n";
			}
			else if (str_temp == "r") {
				if (iT.table[l.table[i].idxTI].iddatatype == IT::STR)
				{
					str += "\nreturn " + (string)iT.table[l.table[i].idxTI].id;
					
				}
				else
				{
					str += "\nreturn " + (string)iT.table[l.table[i].idxTI].id;
				}

			}
			else if (str_temp == "p") {
				if (iT.table[l.table[i].idxTI].iddatatype == IT::STR)
				{
					str += "\nconsole.log(" + (string)iT.table[l.table[i].idxTI].id;
					str += ")";
				}
				else
				{
					str += "\nconsole.log(" + (string)iT.table[l.table[i].idxTI].id;
					str += ")";
				}
			}
			if (l.table[i].lexema[0] == '}')
			{
				str += "}\n";

			}
			else if (l.table[i].lexema[0] == '{') {
				str += "{\n";
			}
			else if (l.table[i].lexema[0] == '(') {
				str += "(";
			}
			else if (l.table[i].lexema[0] == ')') {
				str += ")";
			}
			else if (l.table[i].lexema[0] == ',') {
				str += ",";
			}
			else if (l.table[i].lexema[0] = ';') {
				str += ";\n";
			}
			else if (l.table[i].lexema[0] = '\n') {
				str += "\n";
			}
			else if (l.table[i].lexema[0] == '=') {
				str += "=";
			}
		}
					std::cout << str;
			
	}
	void print(string str)
	{
		for (int i = 0; i < str.size(); i++)
			std::cout << str;
	}

}