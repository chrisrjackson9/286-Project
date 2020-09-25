#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <bitset>
#include <sstream>
using namespace std;

string SIM_INSTR[1000];
int SIM_DATA[1000];
int REGISTERS[32];

void sim(ofstream& ofs);
bool isFileOpenForOutput(ofstream& ofs, const string& filename);

void add(int rd, int rs, int rt);
void sub(int rd, int rs, int rt);
void mul(int rd, int rs, int rt);
void addi(int rt, int rs, int immediate);
void sw(int rt, int offset, int base);
void sh(int rt, int offset, int base);
void lw(int rt, int offset, int base);
void movz(int rd, int rs, int rt);
void OR(int rd, int rs, int rt);
void AND(int rd, int rs, int rt);
void beq(int rs, int rt, int offset);
void bltz(int rs, int offset);
void sll(int rd, int rt, int sa);
void jr(int rs);
void j(int target);

int main(int argc, char **argv) {
    ofstream ofs; 
    ostringstream oss;
    ostringstream result;
    string binSpace;
    string text;
    int addr = 96;
    int type = 1;

    for (int i = 0; i < 32; i++) {
        REGISTERS[i] = 0;
    }

    isFileOpenForOutput(ofs, "testDis.txt");
    
    char buffer[4];
    int i;
    char * iPtr;
    iPtr = (char*)(void*) &i;

    int FD;
    if (argc >=2) {
    FD = open(argv[1], O_RDONLY); 
    } else FD = open("test1.bin", O_RDONLY);

    int amt = 4;

    while(amt != 0) {
        amt = read(FD, buffer, 4);
        if(amt == 4)
        {
            iPtr[0] = buffer[3];
            iPtr[1] = buffer[2];
            iPtr[2] = buffer[1];
            iPtr[3] = buffer[0];

            //store bits into x
            bitset<32> x(i);

            if (text.compare("BREAK")) {
                //put x into a string stream
                oss << x;
                //convert oss into string
                binSpace = oss.str();
                //clear stream
                oss.str("");
                //insert spaces into instruction 
                binSpace.insert(1, " ");
                binSpace.insert(7, " ");
                binSpace.insert(13, " ");
                binSpace.insert(19, " ");
                binSpace.insert(24, " ");
                binSpace.insert(31, " ");

                //decode instruction 
                int valid = i >> 31;
                int opcode = (((unsigned int)i)>>26);
                int final = ((((unsigned int)i)<<26)>>26);
                int immediate = ((((signed int)i)<<16)>>16);
                int rs = ((((unsigned int)i)<<6)>>27);
                int rt = ((((unsigned int)i)<<11)>>27);
                int rd = ((((unsigned int)i)<<16)>>27);
                int sa = ((((unsigned int)i)<<21)>>27);
                int instr_index = (4*((((unsigned int)i)<<6)>>6));

                    //analyze opcode
                    //CONVERT TO SWITCH STATEMENTS
                    if (valid == 0) {
                        result << "Invalid instruction";
                    } else if (opcode == 32) {
                        if (final == 34) {
                            result << "SUB\t" << "R" << rd << ", R" << rs << ", R" << rt;
                        } else if (final ==37) {
                            result << "OR\t" << "R" << rd << ", R" << rs << ", R" << rt;
                        } else if (final == 6) {
                            result << "MOVZ\t" << "R" << rd << ", R" << rs << ", R" << rt;
                        } else if (final == 36) {
                            result << "AND\t" << "R" << rd << ", R" << rs << ", R" << rt;
                        } else if (final == 32) {
                            result << "ADD\t" << "R" << rd << ", R" << rs << ", R" << rt;
                        } else if (final == 0) {
                            result << "SLL\t" << "R" << rd << ", R" << rt << ", #" << sa;
                        } else if (final == 13) {
                            result << "BREAK";
                        }
                    } else if (opcode == 45) {
                        result << "MUL\t" << "R" << rd << ", R" << rs << ", R" << rt;
                    } else if (opcode == 33) {
                        result << "BLTZ\t" << "R" << rs << ", #" << immediate*4;
                    } else if (opcode == 40) {
                        result << "ADDI\t" << "R" << rt << ", R" << rs << ", #" << immediate;
                    } else if (opcode == 43) {
                        result << "SW\t" << "R" << rt << ", " << immediate << "(" << rs << ")";
                    } else if (opcode == 36) {
                        result << "BEQ\t" << "R" << rs << ", R" << rt << ", #" << immediate;
                    } else if (opcode == 34) {
                        result << "J\t#" << instr_index;
                    } else if (opcode == 35) {
                        result << "LW\t" << "R" << rt << ", " << immediate << "(" << rs << ")";
                    } else if (opcode == 73) {
                        result << "SH\t" << "R" << rt << ", " << immediate << ", (" << rs << ")";
                    } else if (x == 0) {
                        text = "NOP";
                    }else text = "";

                //Put results into text
                text = result.str();
                //Put instruction into array
                SIM_INSTR[addr] = text;
                //Clear stream
                result.str("");
                //Output results before BREAK Instruction
                ofs << binSpace << " " << addr << " " << text << endl;
            } else {
                //Put data into array
                SIM_DATA[addr] = i;
                //Output results after BREAK Instruction
                ofs << x << "\t" << addr << " " << i << endl;
            }
            addr += amt;
        }     
    }
    sim(ofs);
    ofs.close();
    return 0;
}

bool isFileOpenForOutput(ofstream& ofs, const string& filename) {
   //Attempt to open file 
    ofs.open(filename);
    //check if file is open
    if (!ofs.is_open()) {
        return false;
   }
   else {
       return true;
   }
}

void sim(ofstream& ofs) {
    int cycleNum = 1;
    int mod = 8;
    
    for (int i = 0; i < 1000; i+=4) {
        if (SIM_INSTR[i].size() != 0) {
        cout << "====================" << endl;
        cout << "Cycle: " << cycleNum << "\t" << i << "\t" << SIM_INSTR[i] << endl << endl;
        cout << "registers:";
        for (int i = 0; i < 32; i++) {
            if (i%8 == 0) {
            cout << endl << "r" << i << ": \t" << REGISTERS[i] << "\t";
            } else cout << REGISTERS[i] << "\t ";
        }
        cout << endl;
        for (int j = 172; j <= 264; j+=4) {
            if (mod%8 == 0) {
            cout << endl << j << ": \t" << SIM_DATA[j] << "\t";
            } else cout << SIM_DATA[j] << "\t ";
            mod++;
        }
        cout << endl;
        cycleNum++;
        }
    }
}

void addi(int rt, int rs, int immediate) {
    REGISTERS[rt] = REGISTERS[rs] + immediate;
}