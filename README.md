# verilog_make
analyze verilog-hdl source and Makefile generate sub Makefile and make files

File Tree

Makefile - normal make file. this file generate sub makefile through dependent_file_analyzer
dependent_file_analyzer.cpp - analyze dependence files of top verilog-hdl code 
Makefile.sub - sub make file generate from Makefile. it generate to make Verilog-HDL code and after compile delete this file
dependent_**** - dependent file list. it generate to make Verilog-HDL code and after compile delete this file

dir --- Makefile --- Makefile.sub
     |            |- sub(directory) --- dependent_1000 --- dependent_1100 --- depentent_1110
     |                               |                  |                  |- dependent_1120
     |                               |                  |                  |- dependent_1130
     |                               |                  |- dependent_1200 --- dependent_1210
     |                               |- dependent_2000
     |                               |- dependent_3000 --- dependent_3100    ...
     |-dependent_file_analyzer.cpp
