#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip>
#include <bitset>
#include <sstream>
using namespace std;

string SIM_INSTR[1000];
string DECODE[1000];
int SIM_DATA[1000];
int REGISTERS[32];
int addr = 96;
int dataStart = 0;

void sim(ofstream &ofs);
bool isFileOpenForOutput(ofstream &ofs, const string &filename);

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
void beq(int rs, int rt, int offset, int &i);
void bltz(int rs, int offset, int &i);
void sll(int rd, int rt, int sa);
void srl(int rd, int rt, int sa);
void jr(int rs, int &i);
void j(int target, int &i);

int main(int argc, char **argv)
{
    string ofile;
    ofstream assembly;
    ofstream simulator;
    ostringstream oss;
    ostringstream result;
    ostringstream simple;
    string binSpace;
    string text;
    string stext;
    string disFile = "";
    string simFile = "";
    int type = 1;

    for (int i = 0; i < 32; i++)
    {
        REGISTERS[i] = 0;
    }

    

    char buffer[4];
    int i;
    char *iPtr;
    iPtr = (char *)(void *)&i;

    int FD;

    if (argc >= 2)
    {
        FD = open(argv[2], O_RDONLY);
        ofile = argv[4];
        disFile = ofile + "_dis.txt";
        simFile = ofile + "_sim.txt";
        isFileOpenForOutput(assembly, disFile);
        isFileOpenForOutput(simulator, simFile);
    }
    else {
        FD = open("test1.bin", O_RDONLY);
        isFileOpenForOutput(assembly, "testDis.txt");
        isFileOpenForOutput(simulator, "testSim.txt");
    }

    int amt = 4;

    while (amt != 0)
    {
        amt = read(FD, buffer, 4);
        if (amt == 4)
        {
            iPtr[0] = buffer[3];
            iPtr[1] = buffer[2];
            iPtr[2] = buffer[1];
            iPtr[3] = buffer[0];

            //store bits into x
            bitset<32> x(i);

            if (text.compare("BREAK") != 0)
            {
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
                binSpace.insert(25, " ");
                binSpace.insert(31, " ");

                //decode instruction
                int valid = i >> 31;
                int nop = (((unsigned int)i) << 1);
                int opcode = (((unsigned int)i) >> 26);
                int final = ((((unsigned int)i) << 26) >> 26);
                int immediate = ((((signed int)i) << 16) >> 16);
                int rs = ((((unsigned int)i) << 6) >> 27);
                int rt = ((((unsigned int)i) << 11) >> 27);
                int rd = ((((unsigned int)i) << 16) >> 27);
                int sa = ((((unsigned int)i) << 21) >> 27);
                int instr_index = (4 * ((((unsigned int)i) << 6) >> 6));

                //analyze opcode
                if (nop == 0)
                {
                    result << "NOP";
                    simple << "NOP";
                }
                if (valid == 0)
                {
                    result << "Invalid Instruction";
                }

                switch (opcode)
                {
                case 32:
                    switch (final)
                    {
                    case 8:
                        result << "JR\tR" << rs;
                        simple << "JR " << rs;
                        break;
                    case 34:
                        result << "SUB\t"
                               << "R" << rd << ", R" << rs << ", R" << rt;
                        simple << "SUB " << rd << " " << rs << " " << rt;
                        break;
                    case 37:
                        result << "OR\t"
                               << "R" << rd << ", R" << rs << ", R" << rt;
                        simple << "OR " << rd << " " << rs << " " << rt;
                        break;
                    case 10:
                        result << "MOVZ\t"
                               << "R" << rd << ", R" << rs << ", R" << rt;
                        simple << "MOVZ " << rd << " " << rs << " " << rt;
                        break;
                    case 36:
                        result << "AND\t"
                               << "R" << rd << ", R" << rs << ", R" << rt;
                        simple << "AND " << rd << " " << rs << " " << rt;
                        break;
                    case 32:
                        result << "ADD\t"
                               << "R" << rd << ", R" << rs << ", R" << rt;
                        simple << "ADD " << rd << " " << rs << " " << rt;
                        break;
                    case 0:
                        if (rt != 0 || rd != 0 || sa != 0)
                        {
                            result << "SLL\t"
                                   << "R" << rd << ", R" << rt << ", #" << sa;
                            simple << "SLL " << rd << " " << rt << " " << sa;
                        }
                        break;
                    case 2:
                        result << "SRL\t"
                               << "R" << rd << ", R" << rt << ", #" << sa;
                        simple << "SRL " << rd << " " << rt << " " << sa;
                        break;
                    case 13:
                        result << "BREAK";
                        simple << "BREAK";
                        break;
                    default:
                        break;
                    }
                    break;
                case 60:
                    result << "MUL\t"
                           << "R" << rd << ", R" << rs << ", R" << rt;
                    simple << "MUL " << rd << " " << rs << " " << rt;
                    break;
                case 33:
                    result << "BLTZ\t"
                           << "R" << rs << ", #" << immediate * 4;
                    simple << "BLTZ " << rs << " " << immediate * 4;
                    break;
                case 40:
                    result << "ADDI\t"
                           << "R" << rt << ", R" << rs << ", #" << immediate;
                    simple << "ADDI " << rt << " " << rs << " " << immediate;
                    break;
                case 43:
                    result << "SW\t"
                           << "R" << rt << ", " << immediate << "(R" << rs << ")";
                    simple << "SW " << rt << " " << immediate << " " << rs;
                    break;
                case 36:
                    result << "BEQ\t"
                           << "R" << rs << ", R" << rt << ", #" << immediate;
                    simple << "BEQ " << rs << " " << rt << " " << immediate;
                    break;
                case 34:
                    result << "J\t#" << instr_index;
                    simple << "J " << instr_index;
                    break;
                case 35:
                    result << "LW\t"
                           << "R" << rt << ", " << immediate << "(R" << rs << ")";
                    simple << "LW " << rt << " " << immediate << " " << rs;
                    break;
                case 73:
                    result << "SH\t"
                           << "R" << rt << ", " << immediate << ", (R" << rs << ")";
                    simple << "SH " << rt << " " << immediate << " " << rs;
                    break;
                default:
                    text = "";
                    stext = "";
                    break;
                }

                //Put results into text
                text = result.str();
                stext = simple.str();
                //Put instruction into array
                SIM_INSTR[addr] = text;
                DECODE[addr] = stext;
                //Clear stream
                result.str("");
                simple.str("");
                //Output results before BREAK Instruction
                assembly << binSpace << " " << addr << " " << text << endl;
            } //end of if (text.comapare("BREAK") != 0)
            else
            {
                //find first line after
                if (dataStart == 0)
                {
                    dataStart = addr;
                }
                //Put data into array
                SIM_DATA[addr] = i;
                //Output results after BREAK Instruction
                assembly << x << "\t" << addr << " " << i << endl;
            }
            addr += amt;
        } //end of if (amt ==4)
    }     //end of while (amt != 0)
    sim(simulator);
    assembly.close();
    return 0;
} //end of main

bool isFileOpenForOutput(ofstream &ofs, const string &filename)
{
    //Attempt to open file
    ofs.open(filename);
    //check if file is open
    if (!ofs.is_open())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void sim(ofstream &ofs)
{
    int cycleNum = 1;
    int mod;
    int rd, rt, rs, sa, immediate;
    string instruction;
    istringstream iss;

    for (int i = 0; i < 1000; i += 4)
    {
        if (SIM_INSTR[i].size() != 0 && SIM_INSTR[i].compare("Invalid Instruction") != 0)
        {
            iss.str(DECODE[i]);
            iss >> instruction;
            if (instruction.compare("ADDI") == 0)
            {
                iss >> rt >> rs >> immediate;
                addi(rt, rs, immediate);
            }
            else if (instruction.compare("SW") == 0)
            {
                iss >> rt >> immediate >> rs;
                sw(rt, immediate, rs);
            }
            else if (instruction.compare("LW") == 0)
            {
                iss >> rt >> immediate >> rs;
                lw(rt, immediate, rs);
            }
            else if (instruction.compare("SLL") == 0)
            {
                iss >> rd >> rt >> sa;
                sll(rd, rt, sa);
            }
            else if (instruction.compare("SRL") == 0)
            {
                iss >> rd >> rt >> sa;
                srl(rd, rt, sa);
            }
            else if (instruction.compare("SUB") == 0)
            {
                iss >> rd >> rs >> rt;
                sub(rd, rs, rt);
            }
            else if (instruction.compare("ADD") == 0)
            {
                iss >> rd >> rs >> rt;
                add(rd, rs, rt);
            }
            else if (instruction.compare("MUL") == 0)
            {
                iss >> rd >> rs >> rt;
                mul(rd, rs, rt);
            }
            else if (instruction.compare("SH") == 0)
            {
                iss >> rt >> immediate >> rs;
                sh(rt, immediate, rs);
            }
            else if (instruction.compare("MOVZ") == 0)
            {
                iss >> rd >> rs >> rt;
                movz(rd, rs, rt);
            }
            else if (instruction.compare("OR") == 0)
            {
                iss >> rd >> rs >> rt;
                OR(rd, rs, rt);
            } 
            else if (instruction.compare("AND") == 0)
            {
                iss >> rd >> rs >> rt;
                AND(rd, rs, rt);
            }

            ofs << "====================" << endl;
            ofs << "cycle: " << cycleNum << "\t" << i << "\t" << SIM_INSTR[i] << endl
                << endl;
            ofs << "registers:";
            for (int i = 0; i < 32; i++)
            {
                if (i % 8 == 0)
                {
                    ofs << endl
                        << "r" << setw(2) << setfill('0') << i << ": \t" << REGISTERS[i] << "\t";
                }
                else
                    ofs << REGISTERS[i] << "\t ";
            }

            ofs << endl
                << endl
                << "data:";
            
            mod =8;

            for (int j = dataStart; j <= addr - 4; j += 4)
            {
                if (mod % 8 == 0)
                {
                    ofs << endl
                        << j << ": \t" << SIM_DATA[j] << "\t";
                }
                else
                    ofs << SIM_DATA[j] << "\t ";
                mod++;
            }

            if (instruction.compare("J") == 0)
            {
                iss >> immediate;
                j(immediate, i);
            }
            else if (instruction.compare("BLTZ") == 0)
            {
                iss >> rs >> immediate;
                bltz(rs, immediate, i);
            }
            else if (instruction.compare("BEQ") == 0)
            {
                iss >> rs >> rt >> immediate;
                beq(rs, rt, immediate, i);
            }
            else if (instruction.compare("JR") == 0)
            {
            }

            ofs << endl
                << endl;
            iss.clear();
            cycleNum++;
        }
    }
}

void add(int rd, int rs, int rt)
{
    REGISTERS[rd] = REGISTERS[rs] + REGISTERS[rt];
}

void sub(int rd, int rs, int rt)
{
    REGISTERS[rd] = REGISTERS[rs] - REGISTERS[rt];
}

void mul(int rd, int rs, int rt)
{
    REGISTERS[rd] = REGISTERS[rs] * REGISTERS[rt];
}

void addi(int rt, int rs, int immediate)
{
    REGISTERS[rt] = REGISTERS[rs] + immediate;
}

void sw(int rt, int offset, int base)
{
    SIM_DATA[REGISTERS[base] + offset] = REGISTERS[rt];
}

void sh(int rt, int offset, int base)
{
    SIM_DATA[REGISTERS[base] + offset] = REGISTERS[rt];
}

void lw(int rt, int offset, int base)
{
    REGISTERS[rt] = SIM_DATA[REGISTERS[base] + offset];
}

void movz(int rd, int rs, int rt)
{
    if (REGISTERS[rt] == 0)
    {
        REGISTERS[rd] = REGISTERS[rs];
    }
}

void OR(int rd, int rs, int rt)
{
    REGISTERS[rd] = (REGISTERS[rs] | REGISTERS[rt]);
}

void AND(int rd, int rs, int rt)
{
    REGISTERS[rd] = (REGISTERS[rs] & REGISTERS[rt]);
}

void beq(int rs, int rt, int offset, int &i)
{
    if (REGISTERS[rs] == REGISTERS[rt])
    {
        i = offset;
    }
}

void bltz(int rs, int offset, int &i)
{
    if (REGISTERS[rs] < 0)
    {
        i += (offset);
    }
}

void sll(int rd, int rt, int sa)
{
    REGISTERS[rd] = REGISTERS[rt] * (sa * 2);
}

void srl(int rd, int rt, int sa)
{
    REGISTERS[rd] = REGISTERS[rt] / (sa * 2);
}

void jr(int rs, int &i)
{
    i = rs;
}

void j(int target, int &i)
{
    i = (target - 4);
}