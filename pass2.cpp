#include <bits/stdc++.h>

#include "pass1.cpp"

using namespace std;

ofstream listing;

ofstream object_code;

int PASS2_E=0;
	
string upper_case(string name ){
	//string name = "john doe"; //or just get string from user...
	for(int i = 0; i < name.size(); i++) {
    	name.at(i) = toupper(name.at(i));
	}
	return name;
}

string string_to_hex(string input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

	string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}


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

string int_hex(int n, int width){
	stringstream ss;
	ss  << setfill('0') << setw(width) << hex << n;
	return ss.str();
}	


void write_line(int i, string obc){
	listing << setw(4) << left << i*5;
	listing << setw(4) << setfill('0') << right << hex << code_table[i].loc << " " << setfill(' ')  << dec;
	listing << setw(8) << left << code_table[i].label;
	listing << setw(8) << left << code_table[i].opcode;
	listing << setw(10) << left << code_table[i].operand;
	listing << setw(8) << left << upper_case(obc) << endl;
}


void write_line(int i){
	listing << setw(9) << left << i*5;
	listing << setw(8) << left << code_table[i].label;
	listing << setw(8) << left << code_table[i].opcode;
	listing << setw(8) << left << code_table[i].operand << endl;
}

void write_comment(int i){
	listing  << left << setw(4) << i*5 << "     " << left << code_table[i].com_line << endl;
}


void pass2(string asmfile){
	vector<int> mod_record;
	listing.open(asmfile+".list");
	object_code.open(asmfile+".out");
	error_file.open(asmfile+".err");
	int j=0;
	string label, opcode, operand;
	int loc;
	while (code_table[j].com_line[0]=='.'){
		write_comment(j);
		j++;
	}
	int idx=0;
	idx=j;
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
	bool isBase=false;
	for ( ; idx<code_table.size(); idx++){
		pc = code_table[idx+1].loc;
		label = code_table[idx].label;
		opcode = code_table[idx].opcode;
		operand = code_table[idx].operand;
		loc = code_table[idx].loc;
		int n,i,x,b,p,e;
		string asm_final;
		n=i=1;
		e=0;
		b=p=0;
		x=0;
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
				else if (operand[0]=='#'){
					n=0;
					i=1;
					operand = operand.substr(1,operand.length());
				}
				else if (ops.size()==2){
					if (ops[1][0]=='X'){
						n=1;
						i=1;
						x=1;
						operand = (string)(ops[0]);
					}
				}
				int opaddr;
				int isConst=0;
				if (SYMTAB[operand].y==1){
					opaddr = SYMTAB[operand].loc;
				}
				else if (LITTAB[operand].found==1){
					opaddr = LITTAB[operand].loc;
				}
				else if (n==0 and i==1 and is_num(operand)){
					opaddr = stoi(operand);
					b=p=0;
					isConst=1;
				}
				else{
					opaddr=0;
					PASS2_E=1;
				}
				if ((e==0) and (!isConst )and (opaddr-pc<2048 and opaddr-pc>=-2048) ){
					b=0;	
					p=1;
					opaddr = opaddr-pc;
				}
				else if ((e==0) and isBase and !isConst){
					if (0<=opaddr-base<4096){
						b=1;
						p=0;
						opaddr = opaddr-base;
					}
				}
				if (!isConst and p==0 and b==0){
					mod_record.push_back((loc+1<<8) + (3+e)*2-3);
				}
				if (operand==""){
					opaddr=0;
				}
				if (opcode=="RSUB"){
					n=i=1;
					x=p=b=0;
				}
				int asm_code;
				if (e==0){
					asm_code = (OPCODETAB[opcode] << 16) + (n << 17) + (i << 16) + (x << 15) + (b << 14) + (p << 13) + (e << 12) + (opaddr&0xfff);
					asm_code = asm_code & 0xffffff;
				}
				else{
					asm_code = (((OPCODETAB[opcode] << 16) + (n << 17) + (i << 16) + (x << 15) + (b << 14) + (p << 13) + (e << 12)) << 8) + (opaddr&0xfffff);
					asm_code = asm_code & 0xffffffff;
				}
				code_table[idx].asm_code=int_hex(asm_code,(3+e)*2);
			}
			else if (OPTAB[opcode] == 2){
				vector<string> ops;
				ops = parse_operand(operand);
				int asm_code;
				if (ops.size()==2 and (REGISTERS[ops[0]] or ops[0]=="A") and (REGISTERS[ops[1]] or ops[1]=="A")){
					asm_code = (OPCODETAB[opcode] << 8) + (REGISTERS[ops[0]] << 4) + (REGISTERS[ops[1]]);
				}
				else if (ops.size()==1 and (REGISTERS[ops[0]] or ops[0]=="A")){
					//error
					asm_code = (OPCODETAB[opcode] << 8) + (REGISTERS[ops[0]] << 4);
				}
				else{
					asm_code = (OPCODETAB[opcode] << 8) ;	
				}
				if (ops.size()==2 and (REGISTERS[ops[0]] or ops[0]=="A") and (is_num(ops[1]) and opcode.substr(5)=="SHIFT")){
					asm_code = (OPCODETAB[opcode] << 8) + (REGISTERS[ops[0]] << 4) + stoi(ops[1])-1;
				}
				else if (ops.size()==1 and (REGISTERS[ops[0]] or ops[0]=="A")){
					//error
					asm_code = (OPCODETAB[opcode] << 8) + (REGISTERS[ops[0]] << 4);
				}
				else {
					asm_code = (OPCODETAB[opcode] << 8) ;	
				}
				code_table[idx].asm_code=int_hex(asm_code,2*2);
			}
			else if (OPTAB[opcode] == 1){
				int asm_code;
				code_table[idx].asm_code=int_hex(OPCODETAB[opcode],2);
			}
			else if (opcode[0]=='=' and opcode[2]=='\'' and opcode[opcode.length()-1] == '\'' and (opcode[1] == 'X' or opcode[1] == 'C')){
				int asm_code;
				if (opcode[1]=='C'){
					code_table[idx].asm_code=string_to_hex(opcode.substr(3,opcode.length()-4));
				}
				else{
					code_table[idx].asm_code=(opcode.substr(3,opcode.length()-4));
				}
			}
			else if (opcode=="WORD"){
				int asm_code;
				asm_code = stoi(operand);
				code_table[idx].asm_code=int_hex(asm_code,6);
			}
			else if (opcode=="BYTE"){
				string temp;
				if (operand[0]=='C'){
					temp=string_to_hex(operand.substr(2,operand.length()-1-2));
				}
				else if (operand[0]=='X'){
					temp = operand.substr(2,operand.length()-2-1);
				}
				else{
					temp = "00";
					error_file << "Error at line " << idx+1 << ": " << "Constant with invalid format \'" << operand << '\'' << endl;
				}
				code_table[idx].asm_code=temp;
			}
			else if (opcode=="BASE"){
				base = SYMTAB[operand].loc;
				isBase=true;
			}
			else if (opcode=="NOBASE"){
				base = 0;
				isBase = false;
			}

			write_line(idx,code_table[idx].asm_code);

		}
		else{
			write_comment(idx);
		}

	}
	string T="";
	int Tl;
	for (int i=j; i<code_table.size(); i++){
		if ((code_table[i].asm_code+T).length()<=60 and code_table[i].asm_code != ""){
			if (T==""){
				Tl = code_table[i].loc;
			}
			T = T+code_table[i].asm_code;
		}
		else if (T!=""){
			object_code << "T" << setw(6) << setfill('0') << uppercase << right << hex <<(Tl);
			object_code << setw(2) << setfill('0') << T.length()/2;
			object_code << setw(60) << setfill(' ') << left << upper_case(T) << endl; 
			T="";
		}
	}

	for (int q=0; q<mod_record.size(); q++){
		object_code << 'M' << setfill('0') << right << setw(8)<< hex <<mod_record[q] << endl;
	}
	object_code << "E" << setfill('0') << setw(6) << right << hex << code_table[j].loc << endl;
	listing.close();
	object_code.close();
}

int main(int argc, char* argv[]){
	if (argc<2){
		cout << "Usage: sicasm <input-file>\n";
		return 1;
	}
	error_file.open(string(argv[1])+".err");
	pass1(argv[1]);
	pass2(argv[1]);

	cout << "Intermidiate file : " << argv[1] << ".int" << endl;
	cout << "Listing file      : " << argv[1] << ".list" << endl;
	cout << "Object Code file  : " << argv[1] << ".out" << endl;
	cout << "Error file        : " << argv[1] << ".err" << endl;
	error_file.close();
	return 0;
}