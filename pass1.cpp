#include <bits/stdc++.h>

#include "tables.cpp"

using namespace std;

struct symtab_struct{
	bool e;
	int loc;
	int y;
};

struct littab_struct{
	int loc;
	int found;
	int length;
	int written;
	littab_struct(){
		length=found=written=0;

	}
};

struct table_struct{
	string label;
	string opcode;
	string operand;
	string comment;
	string com_line;
	string asm_code;
	int loc;
};

map<string,symtab_struct> SYMTAB;
map<string,littab_struct> LITTAB;
int LOCCTR;
int PASS1_E=0;

vector<string> codeLines;

vector<table_struct> code_table;

void get_lines(string asmfile){
	int i=0;
	string temp;
	ifstream codeFile;
	codeFile.open(asmfile);
	while (getline(codeFile,temp)){
		codeLines.push_back(temp);
		i++;
	}
}

void make_table(string asmfile){
	get_lines(asmfile);
	for (int i=0; i<codeLines.size(); i++){
		string line;
		line = codeLines[i];
		stringstream ss(line);
		string label;
		string opcode;
		string operand;
		string comment;
		string com_line;
		com_line="";
		if (line[0]=='.'){
			com_line=line;
			label="";
			opcode="";
			operand="";
			comment="";
			//com_line="";
		}
		else{
			if (!isspace(line[0])){
				//label present
				ss >> label;
			}
			else if (isspace(line[0])){
				//label not present
				label="";
			}
			ss >> opcode;
			if (OPTAB[opcode]==1 or opcode=="RSUB" or opcode=="+RSUB" or opcode=="LTORG"){
				operand="";
			}
			else{
				ss >> operand;
				string temp;
				if (operand[0]=='C' and operand[1]=='\''){
					while (operand[operand.length()-1]!='\''){
						ss >> temp;
						operand+=" "+temp;
						if (operand[operand.length()-1] == '\'')
							break;
						if (!(ss>>temp)){
							PASS1_E=1;
							break;
						}
					}
				} 
			}
			string temp;
			while (ss >> temp){
				comment+= " "+temp;
			}
		}
		//printf("%s\n", com_line.c_str());
		table_struct temp;
		temp.label=label;
		temp.opcode=opcode;
		temp.operand=operand;
		temp.comment=comment;
		temp.com_line=com_line;
		code_table.push_back(temp);
	}
}


int pass1(string asmfile){
	LOCCTR=0; // for good measure
	make_table(asmfile);
	//return 0;
	int i=0;
	int j=0;
	while (code_table[j].com_line[0]=='.'){
		code_table[j].loc=0;
		j++;

	}
	if (code_table[j].opcode=="START"){
		LOCCTR=stoi(code_table[j].operand,nullptr,16);
		code_table[j].loc=LOCCTR;
		i=j+1;
	}

	for ( ; i<code_table.size(); i++){
		if (code_table[i].opcode=="END"){
			code_table[i].loc=LOCCTR;
			int li=0;
			for (auto temp : LITTAB){
				string lit;
				lit = temp.first;
				
				if (LITTAB[lit].found and !LITTAB[lit].written){
					table_struct litemp;
					litemp.loc = LOCCTR;
					litemp.opcode = lit;
					code_table.insert(code_table.begin()+i+li+1,litemp);
					li++;
					LITTAB[lit].loc=LOCCTR;
					LITTAB[lit].written=1;
					LOCCTR+=LITTAB[lit].length;
					i++;
				}
				cout << lit << " " << hex << LITTAB[lit].loc<< endl ;
			}
			break;
		}
		code_table[i].loc=LOCCTR;
		int isLit=0;
		if (code_table[i].operand[0] == '='){
			string temp;
			cout << "LIT\n";
			temp = code_table[i].operand;
			if (temp[2]=='\'' and temp[temp.length()-1] == '\'' and (temp[1] == 'X' or temp[1] == 'C')){
				if (!LITTAB[temp].found){
					LITTAB[temp].found=1;
					cout << "LIT\n";
					if (temp[1]=='X'){
						LITTAB[temp].length = (temp.length()-4+1)/2;
						cout << LITTAB[temp].length;
					}
					else{
						LITTAB[temp].length = (temp.length()-4);	
					}
					isLit=1;
				}
			}
		}
		if (code_table[i].label != ""){
			
			if (SYMTAB.count(code_table[i].label) > 0 ){
				SYMTAB[code_table[i].label].e=1;
			}
			else {
				symtab_struct temp;
				temp.e=0;
				temp.loc=LOCCTR;
				temp.y=1;
				SYMTAB[code_table[i].label] = temp;
			}
		}
		if (code_table[i].opcode != ""){
			int f=0;
			if (OPTAB[code_table[i].opcode] != 0 and DIRECTIVES[code_table[i].opcode] == 0){
				//printf("%s %d\n", code_table[i].opcode.c_str(),OPTAB.count(code_table[i].opcode));
				LOCCTR+=OPTAB[code_table[i].opcode];
			}
			else if (code_table[i].opcode == "WORD"){
				//printf("%d\n", LOCCTR);
				LOCCTR+=3;
			}
			else if (code_table[i].opcode == "RESW"){
				//printf("%d\n", LOCCTR);
				LOCCTR+=stoi(code_table[i].operand)*3;
			}
			else if (code_table[i].opcode == "RESB"){
				LOCCTR+=stoi(code_table[i].operand);
			}
			else if (code_table[i].opcode == "BYTE"){
				string temp;
				temp = code_table[i].operand;
				if (temp[0] == 'C'){
					LOCCTR+=temp.length()-3;
				}
				else if (temp[0] == 'X'){
					LOCCTR+=(temp.length()-2)/2;
				}
			}
			else if (code_table[i].opcode == "BASE"){

			}
			else if (code_table[i].opcode == "LTORG"){
				int li=0;
				for (auto temp : LITTAB){
					string lit;
					lit = temp.first;
					if (LITTAB[lit].found and !LITTAB[lit].written){
						table_struct litemp;
						litemp.loc = LOCCTR;
						litemp.opcode = lit;
						code_table.insert(code_table.begin()+i+1+li,litemp);
						LITTAB[lit].loc=LOCCTR;
						LITTAB[lit].written=1;
						LOCCTR+=LITTAB[lit].length;
						i+=1;
					}
				}
			}
			else{
				printf("%s\n",code_table[i].opcode.c_str());
				PASS1_E = 1;
			}
		}

	}
}