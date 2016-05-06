// dependent_file_analyzer.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>


/*
 *
 * 1st arg : analyze file name
 * 2nd arg : output temporary file name
 *
 */

//using namespace std;
typedef std::vector<std::string> StringVector;

StringVector reserved;
bool comment_region;

bool command_region;
bool extend_command;  //not ; on line end

// if(str.find_first_of(";") != str.find_end_of(";")){
//     command_line <- not assert 
// }

//make list of verilog reserved words
void init_reserved_word(StringVector *v){
	v->push_back("always");
	v->push_back("assign");
	v->push_back("automatic");
	v->push_back("base");
	v->push_back("begin");
	v->push_back("case");
	v->push_back("casex");
	v->push_back("casez");
	v->push_back("cell");
	v->push_back("config");
	v->push_back("deassign");
	v->push_back("default");
	v->push_back("defparam");
	v->push_back("design");
	v->push_back("disable");
	v->push_back("edge");
	v->push_back("else");
	v->push_back("end");
	v->push_back("endcase");
	v->push_back("endconfig");
	v->push_back("endfunction");
	v->push_back("endgenerate");
	v->push_back("endprimitive");
	v->push_back("endspecify");
	v->push_back("endtable");
	v->push_back("endtask");
	v->push_back("event");
	v->push_back("for");
	v->push_back("force");
	v->push_back("forever");
	v->push_back("fork");
	v->push_back("function");
	v->push_back("generate");
	v->push_back("genvar");
	v->push_back("if");
	v->push_back("ifnone");
	v->push_back("incdir");
	v->push_back("include");
	v->push_back("initial");
	v->push_back("inout");
	v->push_back("input");
	v->push_back("instance");
	v->push_back("join");
	v->push_back("liblist");
	v->push_back("library");
	v->push_back("localparam");
	v->push_back("macromodule");
	v->push_back("module");
	v->push_back("negedge");
	v->push_back("noshowcancelled");
	v->push_back("output");
	v->push_back("parameter");
	v->push_back("posedge");
	v->push_back("primitive");
	v->push_back("pulsestyle_ondetect");
	v->push_back("pulsestyle_onevent");
	v->push_back("reg");
	v->push_back("release");
	v->push_back("repeat");
	v->push_back("scalared");
	v->push_back("showcancelled");
	v->push_back("signed");
	v->push_back("specify");
	v->push_back("specparam");
	v->push_back("strength");
	v->push_back("table");
	v->push_back("task");
	v->push_back("tri");
	v->push_back("tri0");
	v->push_back("tri1");
	v->push_back("triend");
	v->push_back("trior");
	v->push_back("wand");
	v->push_back("wor");
	v->push_back("trireg");
	v->push_back("unsigned");
	v->push_back("use");
	v->push_back("vectored");
	v->push_back("wait");
	v->push_back("while");
	v->push_back("wire");
}

inline void delete_head_space(std::string &buf){
	size_t pos;
	while ((pos = buf.find_first_of(" 　\t")) == 0){
		buf.erase(buf.begin());
		if (buf.empty()) break;
	}
}

inline void delete_double_quotes(std::string &buf){
	size_t pos;
	while ((pos = buf.find_first_of("\"")) != std::string::npos){
		buf.erase(pos, 1);
	}
}

inline size_t match_start_region(std::string &str){
	if (str.find("begin") != std::string::npos) return str.find("begin");
	else if (str.find("case") != std::string::npos) return str.find("case");
	else if (str.find("config") != std::string::npos) return str.find("config");
	else if (str.find("function") != std::string::npos) return str.find("function");
	else if (str.find("generate") != std::string::npos) return str.find("generate");
	else if (str.find("module") != std::string::npos) return str.find("module");
	else if (str.find("primitive") != std::string::npos) return str.find("primitive");
	else if (str.find("specify") != std::string::npos) return str.find("specify");
	else if (str.find("table") != std::string::npos) return str.find("table");
	else if (str.find("task") != std::string::npos) return str.find("task");
}

inline size_t match_end_region(std::string &str){
	if (str.find("endbegin") != std::string::npos) return str.find("endbegin");
	else if (str.find("endcase") != std::string::npos) return str.find("endcase");
	else if (str.find("endconfig") != std::string::npos) return str.find("endconfig");
	else if (str.find("endfunction") != std::string::npos) return str.find("endfunction");
	else if (str.find("endgenerate") != std::string::npos) return str.find("endgenerate");
	else if (str.find("endmodule") != std::string::npos) return str.find("endmodule");
	else if (str.find("endprimitive") != std::string::npos) return str.find("endprimitive");
	else if (str.find("endspecify") != std::string::npos) return str.find("endspecify");
	else if (str.find("endtable") != std::string::npos) return str.find("endtable");
	else if (str.find("endtask") != std::string::npos) return str.find("endtask");
}

size_t match_reserved(std::string str){
	for (int i = 0; i < reserved.size(); i++){
		if (str.find(reserved[i]) != std::string::npos)
			return str.find(reserved[i]);
	}
	return std::string::npos;
}

// 依存ファイルが出現するのは
// `includeとインスタンシエートの時だけ
// `include は簡単
// シンスタンシエートの判別は !in_section && 最初の単語が予約語でない

void line_analysis(std::string str, std::ofstream &out){

	size_t pos;
	bool comment_flag = false;

	try{
		//one line comment
		if (pos = str.find_first_of("//") != std::string::npos){
			str = str.substr(pos + 2, str.size());
		}
		//any line comment
		else if (pos = str.find_first_of("/*") != std::string::npos){
			comment_flag = true;
			str = str.substr(pos + 2, str.size());
		}
		//end comment
		else if (pos = str.find("*/") != std::string::npos){
			comment_flag = false;
			str = str.substr(pos + 2, str.size());
		}
	}
	catch (const std::exception e){ //when str size < 0
		return;
	}

	//command over lines
	if (extend_command){
		if (str.find(";") != std::string::npos){
			extend_command = false;
		}
		return;
	}

	//`include file match (rule: include file is only verilog header file)
	if (comment_region != 0 || extend_command != 0){ // || command_region != 0){

		if (str.find("`include") == std::string::npos){
			out << str.substr(str.find("`include") + 1, str.size()) << ".vh" << std::endl;
		}
		// match instanciate (rule: file name must be same of module name)
		else if (match_reserved(str) != std::string::npos){
			if (str.find("=") == std::string::npos){
				out << str.substr(0, str.find_first_of(" ")) << ".v" << std::endl;
			}
		}
	}

	//any other commands

	//else if (match_reserved(str) != std::string::npos){
	//	if (match_start_region(str) != std::string::npos){
	//		if (match_end_region(str) == std::string::npos){
	//			extend_command = true;
	//		}
	//		return;
	//	}
	//	else if (match_end_region(str) != std::string::npos){
	//		extend_command = false;
	//		return;
	//	}
	//	else if (str.find(";") != std::string::npos){
	//		command_line = false;
	//		return;
	//	}
	//	else{
	//		command_line = true;
	//		return;
	//	}
	//}

	//command over lines
	if (str.find(";") == std::string::npos){
		extend_command = true;
	}
	comment_region = comment_flag;
}

int _tmain(int argc, char* argv[])
{
	if (argc != 3)
		return -1;

	std::ifstream in_file;
	std::ofstream tmp_outfile;
	std::string line;

	init_reserved_word(&reserved);

	in_file.open(argv[1]);
	tmp_outfile.open(argv[2]);

	while (getline(in_file, line)){
		delete_head_space(line);
		line_analysis(line, tmp_outfile);
	}

	in_file.close();
	tmp_outfile.close();

	return 0;
}

