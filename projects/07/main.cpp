#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <vector>

struct stat info;

void preProcess(std::string &str)
{
    str = str.substr(0, (int)str.size() - 1);
    std::string x = "";
    for (char c : str)
    {
        if (c == '/')
            break;
        x = x + c;
    }
    str = x;
}

void processArgs(std::string &str, std::vector<std::string> &args)
{
    std::string cur_str = "";
    for (char c : str)
    {
        if (c == ' ')
        {
            if (cur_str == "")
                continue;
            args.push_back(cur_str);
            cur_str = "";
        }
        else
        {
            cur_str = cur_str + c;
        }
    }
    if (cur_str != "")
        args.push_back(cur_str);
}

std::string arithmetic(std::string type, int &c_arithmetic)
{
    if (type == "add")
        return "D=D+M";
    if (type == "sub")
        return "D=D-M";
    if (type == "and")
        return "D=D&M";
    if (type == "or")
        return "D=D|M";
    c_arithmetic++;
    if (type == "eq")
    {
        std::string result = "";
        result = result +
                 "D=D-M" + "\n" +
                 "@EQ" + std::to_string(c_arithmetic) + "\n" +
                 "D;JEQ" + "\n" +
                 "@0" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(EQ" + std::to_string(c_arithmetic) + ")" + "\n" +
                 "@SP" + "\n" +
                 "A=A-1" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(END" + std::to_string(c_arithmetic) + ")";
        return result;
    }
    if (type == "gt")
    {
        std::string result = "";
        result = result +
                 "D=D-M" + "\n" +
                 "@GT" + std::to_string(c_arithmetic) + "\n" +
                 "D;JGT" + "\n" +
                 "@0" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(GT" + std::to_string(c_arithmetic) + ")" + "\n" +
                 "@SP" + "\n" +
                 "A=A-1" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(END" + std::to_string(c_arithmetic) + ")";
        return result;
    }
    if (type == "lt")
    {
        std::string result = "";
        result = result +
                 "D=D-M" + "\n" +
                 "@LT" + std::to_string(c_arithmetic) + "\n" +
                 "D;JLT" + "\n" +
                 "@0" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(LT" + std::to_string(c_arithmetic) + ")" + "\n" +
                 "@SP" + "\n" +
                 "A=A-1" + "\n" +
                 "D=A" + "\n" +
                 "@END" + std::to_string(c_arithmetic) + "\n" +
                 "0;JMP" + "\n" +
                 "(END" + std::to_string(c_arithmetic) + ")";
        return result;
    }
}

void arithmeticCmd(std::vector<std::string> &args, std::ofstream &ofs, int &c_arithmetic)
{
    if (args[0] == "not" || args[0] == "neg")
    {
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        if (args[0] == "not")
            ofs << "D=!D" << std::endl;
        else
            ofs << "D=-D" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "M=M+1" << std::endl;
    }
    else
    {
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << arithmetic(args[0], c_arithmetic) << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "M=M+1" << std::endl;
    }
}

void memoryCmd(std::vector<std::string> &args, std::ofstream &ofs)
{
    int value = 0;
    for (char c : args[2])
    {
        value = value * 10 + (c - '0');
    }
    if (args[1] == "pointer")
    {
        ofs << "@" << value + 3 << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@R" << value + 3 << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R14" << std::endl;
        ofs << "M=D" << std::endl;
    }
    else if (args[1] == "constant")
    {
        ofs << "@" << value << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@R14" << std::endl;
        ofs << "M=D" << std::endl;
    }
    else if (args[1] == "temp")
    {
        ofs << "@" << value + 5 << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@R" << value + 5 << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R14" << std::endl;
        ofs << "M=D" << std::endl;
    }
    else if (args[1] == "static")
    {
        ofs << "@" << value + 16 << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@" << value + 16 << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R14" << std::endl;
        ofs << "M=D" << std::endl;
    }
    else
    {
        if (args[1] == "argument")
        {
            ofs << "@ARG" << std::endl;
        }
        else if (args[1] == "local")
        {
            ofs << "@LCL" << std::endl;
        }
        else if (args[1] == "this")
        {
            ofs << "@THIS" << std::endl;
        }
        else if (args[1] == "that")
        {
            ofs << "@THAT" << std::endl;
        }
        ofs << "D=M" << std::endl;
        ofs << "@" << value << std::endl;
        ofs << "D=D+A" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "A=D" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R14" << std::endl;
        ofs << "M=D" << std::endl;
    }
    if (args[0] == "push")
    { // push
        ofs << "@R14" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "M=M+1" << std::endl;
    }
    else
    { // pop
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "M=D" << std::endl;
    }
}

int main(int argc, char **argv)
{

    for (int i = 1; i < argc; ++i)
    {
        char *pathname = argv[i];
        std::cout << "Trying to load the file " << pathname << "\n\n";
        // check whether if the file is .vm or not

        std::string in_file = pathname;
        std::string out_file = in_file.substr(0, (int)in_file.size() - 2) + "asm";
        std::ifstream ifs(in_file);
        std::ofstream ofs(out_file);

        if (!ifs.is_open())
        {
            std::cout << "Cannot open the file " << pathname << "\n";
            continue;
        }
        std::string str;
        int c_arithmetic = 0;
        while (getline(ifs, str))
        {
            // pre-process the command (remove redundant spaces)
            preProcess(str);
            if (str == "")
                continue;

            // divide into list of args
            std::vector<std::string> args;
            processArgs(str, args);

            if ((int)args.size() == 1)
            { // arithmetics command
                arithmeticCmd(args, ofs, c_arithmetic);
            }
            else if (args[0] == "push" || args[0] == "pop")
            {
                memoryCmd(args, ofs);
            }
        }
    }

    return 0;
}