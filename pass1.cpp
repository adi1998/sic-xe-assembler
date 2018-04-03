#include <bits/stdc++.h>

#include "tables.cpp"
#include "infix.cpp"

using namespace std;

ofstream error_file;
ofstream int_file;

string int_hex(int n, int width);


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
	int value;
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


bool is_num(string s){
	if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

	char * p ;
	strtol(s.c_str(), &p, 10) ;
	return (*p == 0) ;
}

map<string,symtab_struct> SYMTAB;
map<string,littab_struct> LITTAB;
int LOCCTR;
int PASS1_E=0;

vector<string> codeLines;

vector<table_struct> code_table;


pair<int,int> eval_expression(string exp){
	char* temp;
	string expcopy;
	expcopy = exp+"";
	temp = strtok((char*)exp.c_str(), " \t+-*/");
	vector<string> v;
	while (temp != NULL){
		v.push_back(temp);
		temp = strtok(NULL," \t+-*/");
	}
	for (auto it:v){
		cout << it << endl;
	}
	vector<string> symbols;
	int i=0;
	int x=0;
	while (i<v.size()-1 and x<exp.length()){
		if (expcopy[x]=='*' or expcopy[x]=='/' or expcopy[x]=='+' or expcopy[x]=='-'){
			if (!(SYMTAB[v[i]].y==1 or is_num(v[i]))){
				return make_pair(0,1);
			}
			if (!is_num(v[i])){
				symbols.push_back(to_string(SYMTAB[v[i]].loc));
			}
			else{
				symbols.push_back(v[i]);	
			}
			symbols.push_back(expcopy.substr(x,1));
			//cout << "LEL" << expcopy.substr(x,1) << endl;
			i++;
		}
		cout << i<< " " << x << ":" << expcopy[x] << " " << v[i] << endl;
		x++;
	}
	if (!is_num(v[i])){
		symbols.push_back(to_string(SYMTAB[v[i]].loc));
	}
	else{
		symbols.push_back(v[i]);	
	}
	string final_exp;
	for (string it:symbols){
		final_exp+=it;
	}
	cout << final_exp << "=";
	int result;
	result = EvaluateString(final_exp).getResult();
	cout << result << endl;
	return make_pair(result,0);
}

void write_line_int(int i){
	int_file << setw(4) << left << i*5 << " ";
	int_file << setw(4) << setfill('0') << right << hex << code_table[i].loc << " " << setfill(' ')  << dec;
	int_file << setw(8) << left << code_table[i].label;
	int_file << setw(8) << left << code_table[i].opcode;
	int_file << setw(10) << left << code_table[i].operand << endl;
	//listing << setw(8) << left << upper_case(obc) << endl;
}

void write_comment_int(int i){
	int_file  << left << setw(4) << i*5 << "      " << left << code_table[i].com_line << endl;
}

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
		else if (all_of(line.begin(),line.end(),::isspace)){
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
	int_file.open(asmfile+".int");
	LOCCTR=0;
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
		write_line_int(j);
	}

	for ( ; i<code_table.size(); i++){
		if (code_table[i].opcode=="END"){
			code_table[i].loc=LOCCTR;
			int li=0;
			for (pair<string,littab_struct> temp : LITTAB){
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
			}
			write_line_int(i);
			break;
		}
		code_table[i].loc=LOCCTR;
		int isLit=0;
		if (code_table[i].operand=="*"){
			SYMTAB["*"+int_hex(LOCCTR,6)].loc=LOCCTR;
			SYMTAB["*"+int_hex(LOCCTR,6)].y=1;
			code_table[i].operand = "*"+int_hex(LOCCTR,6);
		}
		if (code_table[i].operand[0] == '='){
			string temp;
			temp = code_table[i].operand;
			if (temp[2]=='\'' and temp[temp.length()-1] == '\'' and (temp[1] == 'X' or temp[1] == 'C')){
				if (!LITTAB[temp].found){
					LITTAB[temp].found=1;
					if (temp[1]=='X'){
						LITTAB[temp].length = (temp.length()-4+1)/2;
					}
					else{
						LITTAB[temp].length = (temp.length()-4);	
					}
					isLit=1;
				}
			}
			else if (temp[1]=='*' and temp.length()==2){

				temp = temp + int_hex(LOCCTR,6);
				code_table[i].operand=temp;
				LITTAB[temp].found=1;
				LITTAB[temp].length = 3;
				LITTAB[temp].value=LOCCTR;
				isLit=1;
			}
		}
		if (code_table[i].label != ""){
			
			if (SYMTAB.count(code_table[i].label) > 0 ){
				SYMTAB[code_table[i].label].e=1;
				error_file << "Error at line " << i+1 << ": " << "Symbol redefinition \'" << code_table[i].label << '\'' << endl;
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
				
				LOCCTR+=OPTAB[code_table[i].opcode];
			}
			else if (code_table[i].opcode == "WORD"){
				LOCCTR+=3;
			}
			else if (code_table[i].opcode == "RESW"){
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
			else if (code_table[i].opcode == "EQU"){
				pair<int,int> exp_val=eval_expression(code_table[i].operand);
				//cout << "EQU " <<  
				if(code_table[i].operand[0]=='*'){
					SYMTAB[code_table[i].label].loc = LOCCTR;
					code_table[i].loc=LOCCTR;
				}
				else if (exp_val.second==0){
					SYMTAB[code_table[i].label].loc = exp_val.first;
					code_table[i].loc=exp_val.first;
				}	
				
				else{
					SYMTAB[code_table[i].label].loc = exp_val.first;
					error_file << "Error at line " << i+1 << ": " << "Invalid expression \'" << code_table[i].operand << '\'' << endl;
				}
				
			}
			else if (code_table[i].opcode == "BASE"){

			}
			else if (code_table[i].opcode == "NOBASE"){

			}
			else if (code_table[i].opcode == "LTORG"){
				int li=0;
				for (pair<string,littab_struct> temp : LITTAB){
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
				error_file << "Error at line " << i+1 << ": " << "Invalid opcode \'" << code_table[i].opcode << '\'' << endl;
				PASS1_E = 1;
			}
			write_line_int(i);
		}
		else{
			write_comment_int(i);
		}	
	}
}