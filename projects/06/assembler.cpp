#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <vector>

std::map<std::string, int> variables;
std::vector<std::string> listCmd;
std::vector<std::string> listDest{"M", "D", "MD", "A", "AM", "AD", "AMD"};
std::vector<std::string> listJump{"JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP"};

std::string processComp(std::string comp)
{
    if (comp == "0")
        return "0101010";
    if (comp == "1")
        return "0111111";
    if (comp == "-1")
        return "0111010";
    if (comp == "D")
        return "0001100";
    if (comp == "A")
        return "0110000";
    if (comp == "M")
        return "1110000";
    if (comp == "!D")
        return "1110000";
    if (comp == "!A")
        return "0110001";
    if (comp == "!M")
        return "1110001";
    if (comp == "-D")
        return "0001111";
    if (comp == "-A")
        return "0110011";
    if (comp == "-M")
        return "1110011";
    if (comp == "D+1")
        return "0011111";
    if (comp == "A+1")
        return "0110111";
    if (comp == "M+1")
        return "1110111";
    if (comp == "D-1")
        return "0001110";
    if (comp == "A-1")
        return "0110010";
    if (comp == "M-1")
        return "1110010";
    if (comp == "D+A")
        return "0000010";
    if (comp == "D+M")
        return "1000010";
    if (comp == "D-A")
        return "0010011";
    if (comp == "D-M")
        return "1010011";
    if (comp == "A-D")
        return "0000111";
    if (comp == "M-D")
        return "1000111";
    if (comp == "D&A")
        return "0000000";
    if (comp == "D&M")
        return "1000000";
    if (comp == "D|A")
        return "0010101";
    if (comp == "D|M")
        return "1010101";
}

std::string process(std::string command, int &var_count)
{
    std::string value = "";
    std::string result = "";
    if (command[0] == '@')
    {
        value = command.substr(1, (int)command.size() - 1);
        int dec = 0;
        if (value[0] >= '0' && value[0] <= '9')
        {
            for (char c : value)
            {
                dec = dec * 10 + (c - '0');
            }
        }
        else
        {
            if (variables.find(value) == variables.end())
            {
                dec = var_count;
                variables[value] = var_count;
                var_count++;
            }
            else
            {
                dec = variables[value];
            }
        }
        for (int i = 15; i >= 0; --i)
        {
            int c = dec / (1 << i);
            dec %= (1 << i);
            result = result + char(c + '0');
        }
        return result;
    }
    else
    {
        int pos = -1;
        std::string comp = "";
        int dest = 0;
        int jump = 0;
        for (int i = 0; i < (int)command.size(); ++i)
        {
            if (command[i] == '=' || command[i] == ';')
            {
                pos = i;
                break;
            }
        }
        std::string prefix = command.substr(0, pos);
        std::string suffix = command.substr(pos + 1, (int)command.size() - pos - 1);
        if (command[pos] == '=')
        {
            for (int i = 0; i < 7; ++i)
            {
                if (listDest[i] == prefix)
                    dest = i + 1;
            }
            comp = processComp(suffix);
        }
        else
        {
            for (int i = 0; i < 7; ++i)
            {
                if (listJump[i] == suffix)
                    jump = i + 1;
            }
            comp = processComp(prefix);
        }
        std::string result = "111";
        result = result + comp;
        for (int i = 2; i >= 0; --i)
        {
            int c = dest / (1 << i);
            dest %= (1 << i);
            result = result + char(c + '0');
        }
        for (int i = 2; i >= 0; --i)
        {
            int c = jump / (1 << i);
            jump %= (1 << i);
            result = result + char(c + '0');
        }
        return result;
    }
}

int main(int argv, char **argc)
{

    for (int i = 1; i < argv; ++i)
    {
        variables["SP"] = 0;
        variables["LCL"] = 1;
        variables["ARG"] = 2;
        variables["THIS"] = 3;
        variables["THAT"] = 4;
        variables["SCREEN"] = 16384;
        variables["KBD"] = 24576;
        for (int i = 0; i <= 15; ++i)
        {
            std::string c = std::to_string(i);
            c = "R" + c;
            variables[c] = i;
        }
        int len = strlen(argc[i]);

        if (len < 5)
        {
            std::cout << "File " << argc[i] << " is invalid" << std::endl;
            continue;
        }

        std::string file_in(argc[i]);
        std::ifstream ifs(file_in);
        std::string file_out = file_in.substr(0, len - 3);
        file_out = file_out + "hack";
        std::ofstream ofs(file_out);
        std::string line;
        int line_count = 0;

        while (getline(ifs, line))
        {
            line = line.substr(0, (int)line.size() - 1);
            std::string line_trim = "";
            for (char c : line)
            {
                if (c == ' ')
                    continue;
                if (c == '/')
                    break;
                line_trim = line_trim + c;
            }
            line = line_trim;
            if ((int)line.size() == 0)
            {
                continue;
            }
            if (line[0] == '(')
            {
                std::string funcName = line.substr(1, (int)line.size() - 2);
                variables[funcName] = line_count;
                continue;
            }
            listCmd.push_back(line);
            line_count++;
        }
        int var_count = 16;
        for (std::string cmd : listCmd)
        {
            std::string x = process(cmd, var_count);
            ofs << x << '\n';
        }
        variables.clear();
        listCmd.clear();
        ifs.close();
        ofs.close();
    }

    return 0;
}