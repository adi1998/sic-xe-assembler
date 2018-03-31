#include <bits/stdc++.h>

#include "pass1.cpp"

using namespace std;

ofstream listing;
ofstream object_code;

int PASS2_E=0;
	
vector<string> parse_operand(string operand){
	char * temp;
	temp = strtok((char*)operand.c_str(),", \t");
	vector<string> ops;
	while (temp != NULL){
		ops.push_back(temp);
		temp = strtok(NULL,", \t");
	}	
	return ops;
}



bool is_num(string s){
	if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

	char * p ;
	strtol(s.c_str(), &p, 10) ;
	return (*p == 0) ;
}

void write_line(int i, int obc){
	listing << setw(4) << left << i*5;
	listing << setw(8) << left << code_table[i].label;
	listing << setw(8) << left << code_table[i].opcode;
	listing << setw(8) << left << code_table[i].operand;
	listing << setw(8) << left << hex << obc << dec << endl;
}


void write_line(int i){
	listing << setw(4) << left << i*5;
	listing << setw(8) << left << code_table[i].label;
	listing << setw(8) << left << code_table[i].opcode;
	listing << setw(8) << left << code_table[i].operand << endl;
}


void pass2(string asmfile){
	vector<int> mod_record;
	listing.open(asmfile+".list");
	object_code.open(asmfile+".out");
	int j=0;
	string label, opcode, operand;
	int loc;
	while (code_table[j].com_line[0]=='.'){
		j++;
	}
	int idx=0;
	if (code_table[j].opcode=="START"){
		idx=j;
		label = code_table[idx].label;
		opcode = code_table[idx].opcode;
		operand = code_table[idx].operand;
		loc = code_table[idx].loc;
		write_line(idx);
		object_code << left << "H" << setw(6) << label << right << hex << setfill('0') << setw(6) << stoi(operand,nullptr,16) << setfill('0') << setw(6) << LOCCTR << endl;
		idx=j+1;
	}
	int pc;
	pc = code_table[idx+1].loc;
	int base;
	for ( ; idx<code_table.size(); idx++){
		pc = code_table[idx+1].loc;
		label = code_table[idx].label;
		opcode = code_table[idx].opcode;
		operand = code_table[idx].operand;
		loc = code_table[idx].loc;
		int n,i,x,b,p,e;
		bool isBase;
		n=i=1;
		if ((code_table[idx].com_line=="")){
			if (OPTAB[opcode] > 2){
				vector<string> ops;
				ops = parse_operand(operand);
				if (opcode[0]=='+'){
					e=1;
					opcode = opcode.substr(1,opcode.length());
				}
				if (operand[0]=='@'){
					n=1;
					i=0;
					operand = operand.substr(1,operand.length());
				}
				if (operand[0]=='#'){
					n=0;
					i=1;
					operand = operand.substr(1,operand.length());
				}
				if (ops.size()==2 and ops[1][0]=='X'){
					n=1;
					i=1;
					x=1;
					operand = (string)(ops[0]);
					//cout << operand;
				}
				int opaddr;
				if (SYMTAB.count(operand)!=0){
					opaddr = SYMTAB[operand].loc;
				}
				else if (n==0 and i==1 and is_num(operand)){
					opaddr = stoi(operand);
				}
				else{
					opaddr=0;
					PASS2_E=1;
				}
				if (opaddr-pc<2048 and opaddr-pc>=-2048 ){
					b=0;
					p=1;
					opaddr = opaddr-pc;
				}
				else if (isBase){
					if (0<=opaddr-base<4096){
						b=1;
						p=0;
						opaddr = opaddr-base;
					}
					else{
						mod_record.push_back(loc+1<<8 + (3+e)*2-3);
					}
				}
				else{
					mod_record.push_back(loc+1<<8 + (3+e)*2-3);
				}
				int asm_code;
				if (e==0){
					asm_code = (OPCODETAB[opcode] << 16) + (n << 17) + (i << 16) + (x << 15) + (b << 14) + (p << 13) + (e << 12) + (opaddr&0xfff);
					asm_code = asm_code && 0xffffff;
				}
				else{
					asm_code = (((OPCODETAB[opcode] << 16) + (n << 17) + (i << 16) + (x << 15) + (b << 14) + (p << 13) + (e << 12)) << 8) + (opaddr&0xfffff);
				}
				code_table[idx].asm_code=asm_code;
			}
			else if (OPTAB[opcode] == 2){
				vector<string> ops;
				ops = parse_operand(operand);
				int asm_code;
				if (ops.size()==2 and (REGISTERS[ops[0]] or ops[0]=="A") and (REGISTERS[ops[1]] or ops[1]=="A")){
					asm_code = (OPCODETAB[opcode] << 8) + (REGISTERS[ops[0]] << 4) + (REGISTERS[ops[1]]);
				}
				else{
					//error
					asm_code = (OPCODETAB[opcode] << 8);
				}
				code_table[idx].asm_code=asm_code;
			}
			else if (OPTAB[opcode] == 1){
				int asm_code;
			}
		}
	}
	listing.close();
	object_code.close();
}

int main(int argc, char* argv[]){
	if (argc<2){
		cout << "Usage: sicasm <input-file>\n";
		return 1;
	}
	pass1(argv[1]);
	for (int i=0; i<code_table.size(); i++){
		printf("%04x| %20s| %8s| %8s| %8s| %s\n", code_table[i].loc,code_table[i].com_line.c_str(),code_table[i].label.c_str(), code_table[i].opcode.c_str(), code_table[i].operand.c_str(),code_table[i].comment.c_str());
	}
	cout << PASS1_E;
	pass2(argv[1]);
	return 0;
}