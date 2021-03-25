#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <map>

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

void pushSomething(std::ofstream &ofs)
{
    ofs << "@SP" << std::endl;
    ofs << "A=M" << std::endl;
    ofs << "M=D" << std::endl;
    ofs << "@SP" << std::endl;
    ofs << "M=M+1" << std::endl;
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
        pushSomething(ofs);
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
        pushSomething(ofs);
    }
}

void memoryCmd(std::vector<std::string> &args, std::ofstream &ofs, std::string &function_name)
{
    int value = 0;
    for (char c : args[2])
    {
        if (c < '0' || c > '9')
            continue;
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
        ofs << "@" << function_name << "static" << value << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "A=M" << std::endl;
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
        pushSomething(ofs);
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

void flowCmd(std::vector<std::string> &args, std::ofstream &ofs)
{
    if (args[0] == "label")
    {
        ofs << "(" << args[1] << ")" << std::endl;
    }
    else if (args[0] == "goto")
    {
        ofs << "@" << args[1] << std::endl;
        ofs << "0;JMP" << std::endl;
    }
    else
    {
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@" << args[1] << "END" << std::endl;
        ofs << "D;JEQ" << std::endl;
        ofs << "@" << args[1] << std::endl;
        ofs << "0;JMP" << std::endl;
        ofs << "(" << args[1] << "END)" << std::endl;
    }
}

void calcFrame(int minus, std::ofstream &ofs)
{
    ofs << "@" << minus << std::endl;
    ofs << "D=A" << std::endl;
    ofs << "@R13" << std::endl;
    ofs << "D=M-D" << std::endl;
    ofs << "A=D" << std::endl;
    ofs << "D=M" << std::endl;
}

void funcCmd(std::vector<std::string> &args, std::ofstream &ofs, int &c_function)
{
    if (args[0] == "function")
    {
        std::string func_name = args[1];
        ofs << "(" << func_name << ")" << std::endl;
        int value = 0;
        for (char c : args[2])
        {
            if (c < '0' || c > '9')
                continue;
            value = value * 10 + (c - '0');
        }
        for (int i = 0; i < value; ++i)
        {
            // push 0
            ofs << "@0" << std::endl;
            ofs << "D=A" << std::endl;
            pushSomething(ofs);
        }
    }
    else if (args[0] == "return")
    {
        // R13 = LCL
        ofs << "@LCL" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R13" << std::endl;
        ofs << "M=D" << std::endl;

        // R14 = LCL-5
        ofs << "@R13" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@5" << std::endl;
        ofs << "D=D-A" << std::endl;
        ofs << "A=D" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@R14" << std::endl; // return-address
        ofs << "M=D" << std::endl;

        // *ARG = pop()
        ofs << "@SP" << std::endl;
        ofs << "M=M-1" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@ARG" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "M=D" << std::endl;

        // SP = ARG + 1
        ofs << "@1" << std::endl;
        ofs << "D=A" << std::endl;
        ofs << "@ARG" << std::endl;
        ofs << "D=D+M" << std::endl;
        ofs << "@SP" << std::endl;
        ofs << "M=D" << std::endl;

        // THAT = * (R13 - 1)
        calcFrame(1, ofs);
        ofs << "@THAT" << std::endl;
        ofs << "M=D" << std::endl;

        // THIS = * (R13 - 2)
        calcFrame(2, ofs);
        ofs << "@THIS" << std::endl;
        ofs << "M=D" << std::endl;

        // ARG = * (R13 - 3)
        calcFrame(3, ofs);
        ofs << "@ARG" << std::endl;
        ofs << "M=D" << std::endl;

        // LCL = * (R13 - 4)
        calcFrame(4, ofs);
        ofs << "@LCL" << std::endl;
        ofs << "M=D" << std::endl;

        // go to R14
        ofs << "@R14" << std::endl;
        ofs << "A=M" << std::endl;
        ofs << "0;JMP" << std::endl;
    }
    else if (args[0] == "call")
    {
        int value = 0;
        for (char c : args[2])
        {
            if (c < '0' || c > '9')
                continue;
            value = value * 10 + (c - '0');
        }
        // push return-address
        c_function++;
        ofs << "@FUNC" << c_function << std::endl;
        ofs << "D=A" << std::endl;
        pushSomething(ofs);

        //push LCL
        ofs << "@LCL" << std::endl;
        ofs << "D=M" << std::endl;
        pushSomething(ofs);

        // push ARG
        ofs << "@ARG" << std::endl;
        ofs << "D=M" << std::endl;
        pushSomething(ofs);

        // push THIS
        ofs << "@THIS" << std::endl;
        ofs << "D=M" << std::endl;
        pushSomething(ofs);

        // push THAT
        ofs << "@THAT" << std::endl;
        ofs << "D=M" << std::endl;
        pushSomething(ofs);

        // ARG = SP-n-5
        ofs << "@SP" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@" << value << std::endl;
        ofs << "D=D-A" << std::endl;
        ofs << "@5" << std::endl;
        ofs << "D=D-A" << std::endl;
        ofs << "@ARG" << std::endl;
        ofs << "M=D" << std::endl;

        // LCL = SP
        ofs << "@SP" << std::endl;
        ofs << "D=M" << std::endl;
        ofs << "@LCL" << std::endl;
        ofs << "M=D" << std::endl;

        // goto f
        ofs << "@" << args[1] << std::endl;
        ofs << "0;JMP" << std::endl;

        //(return-address)
        ofs << "(FUNC" << c_function << ")" << std::endl;
    }
}

void processFile(std::string pathname, std::ifstream &ifs, std::ofstream &ofs, int &c_arithmetic, int &c_function, std::string &function_name)
{
    std::cout << "Trying to load the file " << pathname << "\n\n";
    // check whether if the file is .vm or not

    if (!ifs.is_open())
    {
        std::cout << "Cannot open the file " << pathname << "\n";
        return;
    }
    std::string str;
    std::vector<std::vector<std::string>> listCmd;

    // first loop to find out the label map

    while (getline(ifs, str))
    {
        // pre-process the command (remove redundant spaces)
        preProcess(str);
        if (str == "")
            continue;

        // divide into list of args
        std::vector<std::string> args;
        processArgs(str, args);
        listCmd.push_back(args);
    }

    for (int i = 0; i < (int)listCmd.size(); ++i)
    {
        std::vector<std::string> args = listCmd[i];

        if ((int)args.size() == 1 && args[0] != "return")
        { // arithmetics command
            arithmeticCmd(args, ofs, c_arithmetic);
        }
        else if (args[0] == "push" || args[0] == "pop")
        {
            memoryCmd(args, ofs, function_name);
        }
        else if (args[0] == "goto" || args[0] == "if-goto" || args[0] == "label")
        {
            flowCmd(args, ofs);
        }
        else
        {
            funcCmd(args, ofs, c_function);
        }
    }
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string pathname(argv[i]);
        int c_arithmetic = 0;
        int c_function = 0;
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(argv[i])) != NULL)
        {
            /* print all the files and directories within directory */
            std::vector<std::string> listVmFile;
            std::vector<std::string> listFileName;
            while ((ent = readdir(dir)) != NULL)
            {
                std::string file(ent->d_name);
                if ((int)file.size() >= 4)
                {
                    std::string type = file.substr((int)file.size() - 3, 3);
                    if (type == ".vm")
                    {
                        std::string fullpath = pathname + "/" + file;
                        listVmFile.push_back(fullpath);
                        listFileName.push_back(file);
                    }
                }
            }
            i ++;
            std::string out_file = pathname + "/" + argv[i];
            std::ofstream ofs(out_file);
            ofs << "@Sys.init" << std::endl;
            ofs << "0;JMP" << std::endl;
            for (int i = 0; i < (int)listVmFile.size(); ++i)
            {
                std::string function_name = listFileName[i];
                std::string in_file = listVmFile[i];
                std::ifstream ifs(in_file);
                processFile(in_file, ifs, ofs, c_arithmetic, c_function, function_name);
            }
            closedir(dir);
        }
        else
        {
            std::string function_name = "only";
            std::string in_file = pathname;
            std::string out_file = in_file.substr(0, (int)in_file.size() - 2) + "asm";
            std::ifstream ifs(in_file);
            std::ofstream ofs(out_file);
            processFile(pathname, ifs, ofs, c_arithmetic, c_function, function_name);
        }
    }

    return 0;
}