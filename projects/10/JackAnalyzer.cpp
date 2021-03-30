#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <map>

bool checkPrefix(std::string foo, std::string bar) //  check if foo starts with bar or not
{
    if ((int)foo.size() < (int)bar.size())
        return false;
    std::string barCmp = foo.substr(0, (int)bar.size());
    return (bar == barCmp);
}

bool checkSuffix(std::string foo, std::string bar) //  check if foo ends with bar or not
{
    if ((int)foo.size() < (int)bar.size())
        return false;
    std::string barCmp = foo.substr((int)foo.size() - (int)bar.size(), (int)bar.size());
    return (bar == barCmp);
}

int type(char c)
{
    if (c >= 'a' && c <= 'z')
        return 0;
    if (c >= 'A' && c <= 'Z')
        return 0;
    if (c == '_')
        return 0;
    if (c == '@')
        return 0;
    if (c >= '0' && c <= '9')
        return 1;
    if (c == '{')
        return 2;
    if (c == '}')
        return 3;
    if (c == '(')
        return 4;
    if (c == ')')
        return 5;
    if (c == ',')
        return 6;
    if (c == ';')
        return 7;
    if (c == '"')
        return 8;
    if (c == '.')
        return 9;
    if (c == '[')
        return 10;
    if (c == ']')
        return 11;
    return 12;
}

std::string typeStr(std::string &str)
{
    std::string keyword[] = {"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true",
                             "false", "null", "this", "let", "do", "if", "else", "while", "return"};
    std::string symbol[] = {"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"};
    for (auto x : keyword)
    {
        if (str == x)
            return "keyword";
    }
    for (auto x : symbol)
    {
        if (str == x)
            return "symbol";
    }
    if (str[0] >= '0' && str[0] <= '9')
        return "integer";
    if (str[0] == '"')
        return "string";
    return "identifier";
}

bool isType(std::string str)
{
    std::vector<std::string> exact{"int", "boolean", "char"};
    for (auto x : exact)
    {
        if (str == x)
            return true;
    }
    return typeStr(str) == "identifier";
}

class JackAnalyzer
{
private:
    std::string pathname;
    std::vector<std::string> listTokenized;
    bool ready;

    void Tokenize(std::string &pathname)
    {
        std::ifstream ifs(pathname);

        std::string cur_string = "";

        bool commentLine = false;
        bool commentBlock = false;
        bool inQuote = false;

        std::string thisline = "";

        while (std::getline(ifs, thisline))
        {
            while (!thisline.empty() && (int)thisline[(int)thisline.size() - 1] < 32)
                thisline.pop_back();
            if (thisline == "")
                continue;
            for (char c : thisline)
            {
                if (inQuote)
                {
                    cur_string = cur_string + c;
                    if (c == '"')
                    {
                        listTokenized.push_back(cur_string);
                        cur_string = "";
                        inQuote = false;
                    }
                    continue;
                }
                if (c == ' ' || c == '\t')
                {
                    if (commentBlock || commentLine)
                        continue;
                    if (cur_string != "")
                    {
                        listTokenized.push_back(cur_string);
                        cur_string = "";
                    }
                    continue;
                }
                cur_string = cur_string + c;
                if (checkSuffix(cur_string, "//"))
                {
                    if (commentBlock || commentLine)
                        continue;
                    commentLine = true;
                    std::string x = cur_string.substr(0, (int)cur_string.size() - 2);
                    if (x != "")
                    {
                        listTokenized.push_back(x);
                    }
                    cur_string = "";
                    continue;
                }
                if (checkSuffix(cur_string, "/*"))
                {
                    if (commentLine || commentBlock)
                        continue;
                    commentBlock = true;
                    std::string x = cur_string.substr(0, (int)cur_string.size() - 2);
                    if (x != "")
                    {
                        listTokenized.push_back(x);
                    }
                    cur_string = "";
                    continue;
                }
                if (checkSuffix(cur_string, "*/"))
                {
                    if (commentLine)
                        continue;
                    if (commentBlock)
                    {
                        commentBlock = false;
                        cur_string = "";
                        continue;
                    }
                }
                if (!commentBlock && !commentLine)
                {
                    if ((int)cur_string.size() >= 2)
                    {
                        int x = type(c), y = type(cur_string[0]);
                        if ((x == y & x <= 1) || (x == 1 && y == 0))
                            continue;
                        std::string foo = cur_string.substr(0, (int)cur_string.size() - 1);
                        listTokenized.push_back(foo);
                        cur_string = "";
                        cur_string = cur_string + c;
                    }
                    if (cur_string[0] == '"')
                    {
                        inQuote = true;
                        // std::string x = ""; x = x + cur_string[0];
                        // cur_string = "";
                        // listTokenized.push_back(x);
                        continue;
                    }
                }
            }
            if (commentLine)
            {
                commentLine = false;
                cur_string = "";
                continue;
            }
            else
            {
                if (commentBlock)
                    continue;
                else
                {
                    if (cur_string != "")
                    {
                        listTokenized.push_back(cur_string);
                        cur_string = "";
                    }
                }
            }
        }
    }

    void print(std::ofstream &ofs, int pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        std::string y = listTokenized[pos];
        std::string x = typeStr(y);
        if (x == "integer")
            x = "integerConstant";
        if (x == "string")
        {
            x = "stringConstant";
            y = y.substr(1, (int)y.size() - 2);
        }
        ofs << "<" << x << "> ";
        ofs << y << " ";
        ofs << "</" << x << ">" << std::endl;
    }

    bool checkType(std::ofstream &ofs, int &pos, int nested, std::vector<std::string> &exact, std::vector<std::string> &type)
    {
        std::string str = listTokenized[pos];
        std::string str_type = typeStr(str);
        bool ok = false;
        for (auto x : exact)
        {
            if (x == str)
                ok = true;
        }
        for (auto x : type)
        {
            if (str_type == x)
                ok = true;
        }
        if (!ok)
        {
            std::cout << "Expected: ";
            for (auto x : exact)
                std::cout << x << " ";
            for (auto x : type)
                std::cout << x << " ";
            std::cout << ", but got " << str << std::endl;
            return false;
        }
        print(ofs, pos, nested);
        pos++;
        return true;
    }

    bool classVarDec(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<classVarDec>" << std::endl;
        nested++;
        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{};
        print(ofs, pos, nested);
        pos++;

        //type
        exact.clear();
        exact.push_back("int");
        exact.push_back("char");
        exact.push_back("boolean");
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // varName
        exact.clear();
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // (, varName)*
        while (1)
        {
            if (listTokenized[pos] != ",")
                break;
            // symbol ,
            exact.clear();
            exact.push_back(",");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
                return false;

            // varName
            exact.clear();
            type_statement.clear();
            type_statement.push_back("identifier");
            if (!checkType(ofs, pos, nested, exact, type_statement))
                return false;
        }

        // symbol ;
        exact.clear();
        exact.push_back(";");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;
        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</classVarDec>" << std::endl;
        return true;
    }

    bool parameterList(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<parameterList>" << std::endl;
        nested++;
        std::vector<std::string> exact{"int", "char", "boolean"};
        std::vector<std::string> type_statement{};

        while (1)
        {
            // type
            if (!isType(listTokenized[pos]))
                break;
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            // varName
            exact.clear();
            type_statement.clear();
            type_statement.push_back("identifier");
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            while (1)
            {
                if (listTokenized[pos] != ",")
                    break;
                // symbol ,
                exact.clear();
                exact.push_back(",");
                type_statement.clear();
                if (!checkType(ofs, pos, nested, exact, type_statement))
                {
                    return false;
                }

                // type
                exact.clear();
                exact.push_back("int");
                exact.push_back("char");
                exact.push_back("boolean");
                type_statement.clear();
                type_statement.push_back("identifier");
                if (!checkType(ofs, pos, nested, exact, type_statement))
                {
                    return false;
                }

                // varName
                exact.clear();
                type_statement.clear();
                type_statement.push_back("identifier");
                if (!checkType(ofs, pos, nested, exact, type_statement))
                {
                    return false;
                }
            }

            break;
        }
        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</parameterList>" << std::endl;
        return true;
    }

    bool varDec(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<varDec>" << std::endl;
        nested++;

        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{};

        // var
        exact.clear();
        exact.push_back("var");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // type
        exact.clear();
        exact.push_back("void");
        exact.push_back("int");
        exact.push_back("char");
        exact.push_back("boolean");
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // varName
        exact.clear();
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        while (1)
        {
            if (listTokenized[pos] != ",")
                break;

            // symbol ,
            exact.clear();
            exact.push_back(",");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
                return false;

            // varName
            exact.clear();
            type_statement.clear();
            type_statement.push_back("identifier");
            if (!checkType(ofs, pos, nested, exact, type_statement))
                return false;
        }

        // symbol ;
        exact.clear();
        exact.push_back(";");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</varDec>" << std::endl;
        return true;
    }

    bool term(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<term>" << std::endl;
        nested++;
        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{"integer", "string", "keyword"};
        std::string x = listTokenized[pos];
        std::string type_x = typeStr(x);

        // integer, string, keyword
        if (type_x == "integer" || type_x == "string" || type_x == "keyword")
        {
            print(ofs, pos, nested);
            pos++;
        }
        else if (type_x == "identifier")
        {
            if (listTokenized[pos + 1] == "." || listTokenized[pos + 1] == "(")
            {
                if (!subroutineCall(ofs, pos, nested))
                {
                    return false;
                }
            }
            else
            {
                // varName
                print(ofs, pos, nested);
                pos++;

                if (listTokenized[pos] == "[")
                {
                    // symbol [
                    print(ofs, pos, nested);
                    pos++;

                    // expression
                    if (!expression(ofs, pos, nested))
                    {
                        return false;
                    }
                    exact.clear();

                    // symbol ]
                    exact.push_back("]");
                    type_statement.clear();
                    if (!checkType(ofs, pos, nested, exact, type_statement))
                    {
                        return false;
                    }
                }
            }
        }
        else if (x == "(")
        {
            // symbol (
            print(ofs, pos, nested);
            pos++;

            // expression
            if (!expression(ofs, pos, nested))
            {
                return false;
            }
            exact.clear();

            // symbol )
            exact.push_back(")");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }
        }
        // -|~ term
        else if (x == "-" || x == "~")
        {
            // symbol -, ~
            print(ofs, pos, nested);
            pos++;

            if (!term(ofs, pos, nested))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</term>" << std::endl;
        return true;
    }

    bool subroutineCall(std::ofstream &ofs, int &pos, int nested)
    {
        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{"identifier"};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        if (listTokenized[pos] == "(")
        {
            print(ofs, pos, nested);
            pos++;

            if (!expressionList(ofs, pos, nested))
            {
                return false;
            }
            exact.clear();
            exact.push_back(")");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }
        }
        else if (listTokenized[pos] == ".")
        {
            print(ofs, pos, nested);
            pos++;

            // subroutineName
            exact.clear();
            type_statement.clear();
            type_statement.push_back("identifier");
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            // symbol (
            exact.clear();
            exact.push_back("(");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            if (!expressionList(ofs, pos, nested))
            {
                return false;
            }
            exact.clear();
            exact.push_back(")");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    bool expression(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<expression>" << std::endl;
        nested++;
        if (!term(ofs, pos, nested))
        {
            return false;
        }
        std::vector<std::string> op{"+", "-", "*", "/", "&", "|", "<", ">", "="};
        bool still = false;

        while (1)
        {
            still = false;
            for (auto x : op)
            {
                if (x == listTokenized[pos])
                    still = true;
            }
            if (!still)
                break;

            // symbol op
            print(ofs, pos, nested);
            pos++;

            // term
            if (!term(ofs, pos, nested))
            {
                return false;
            }
        }
        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</expression>" << std::endl;
        return true;
    }

    bool expressionList(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<expressionList>" << std::endl;
        nested++;

        if (listTokenized[pos] != ")")
        {
            if (!expression(ofs, pos, nested))
            {
                return false;
            }
            std::vector<std::string> op{","};
            while (1)
            {
                bool still = false;
                for (auto x : op)
                {
                    if (x == listTokenized[pos])
                        still = true;
                }
                if (!still)
                    break;

                // symbol ,
                print(ofs, pos, nested);
                pos++;

                // expression
                if (!expression(ofs, pos, nested))
                {
                    return false;
                }
            }
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</expressionList>" << std::endl;
        return true;
    }

    bool letStatement(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<letStatement>" << std::endl;
        nested++;

        std::vector<std::string> exact{"let"};
        std::vector<std::string> type_statement{};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        exact.clear();
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        while (1)
        {
            if (listTokenized[pos] != "[")
                break;

            // symbol [
            exact.clear();
            exact.push_back("[");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            // expression
            if (!expression(ofs, pos, nested))
            {
                return false;
            }

            // symbol ]
            exact.clear();
            exact.push_back("]");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }
        }

        // symbol =
        exact.clear();
        exact.push_back("=");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // expression
        if (!expression(ofs, pos, nested))
        {
            return false;
        }

        // symbol ;
        exact.clear();
        exact.push_back(";");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</letStatement>" << std::endl;
        return true;
    }

    bool ifStatement(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<ifStatement>" << std::endl;
        nested++;

        // if
        std::vector<std::string> exact{"if"};
        std::vector<std::string> type_statement{};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // (
        exact.clear();
        exact.push_back("(");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // expression
        if (!expression(ofs, pos, nested))
        {
            return false;
        }

        // )
        exact.clear();
        exact.push_back(")");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // {
        exact.clear();
        exact.push_back("{");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // statements
        if (!statements(ofs, pos, nested))
        {
            return false;
        }

        // }
        exact.clear();
        exact.push_back("}");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        while (1)
        {
            // else
            if (listTokenized[pos] != "else")
                break;
            print(ofs, pos, nested);
            pos++;

            // {
            exact.clear();
            exact.push_back("{");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }

            // statements
            if (!statements(ofs, pos, nested))
            {
                return false;
            }

            // }
            exact.clear();
            exact.push_back("}");
            type_statement.clear();
            if (!checkType(ofs, pos, nested, exact, type_statement))
            {
                return false;
            }
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</ifStatement>" << std::endl;
        return true;
    }

    bool whileStatement(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<whileStatement>" << std::endl;
        nested++;

        // while
        std::vector<std::string> exact{"while"};
        std::vector<std::string> type_statement{};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // symbol (
        exact.clear();
        exact.push_back("(");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // expression
        if (!expression(ofs, pos, nested))
        {
            return false;
        }

        // symbol )
        exact.clear();
        exact.push_back(")");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // symbol {
        exact.clear();
        exact.push_back("{");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // statements
        if (!statements(ofs, pos, nested))
        {
            return false;
        }

        // symbol }
        exact.clear();
        exact.push_back("}");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</whileStatement>" << std::endl;
        return true;
    }

    bool doStatement(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<doStatement>" << std::endl;
        nested++;

        // do
        std::vector<std::string> exact{"do"};
        std::vector<std::string> type_statement{};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // subroutineCall
        if (!subroutineCall(ofs, pos, nested))
        {
            return false;
        }

        // symbol ;
        exact.clear();
        exact.push_back(";");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</doStatement>" << std::endl;
        return true;
    }

    bool returnStatement(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<returnStatement>" << std::endl;
        nested++;

        // return
        std::vector<std::string> exact{"return"};
        std::vector<std::string> type_statement{};
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // subrountineCall
        while (1)
        {
            if (listTokenized[pos] == ";")
                break;
            if (!expression(ofs, pos, nested))
            {
                return false;
            }
        }

        print(ofs, pos, nested);
        pos++;

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</returnStatement>" << std::endl;

        return true;
    }

    bool statements(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<statements>" << std::endl;
        nested++;
        // statements
        std::vector<std::string> exact{"let", "if", "while", "do", "return"};
        std::vector<std::string> type_statement{};

        bool firstTime = true;
        bool still = false;

        while (1)
        {
            // "let", "if", "while", "do", "return"
            exact = std::vector<std::string>{"let", "if", "while", "do", "return"};
            type_statement.clear();
            still = false;
            for (auto x : exact)
            {
                if (listTokenized[pos] == x)
                {
                    still = true;
                }
            }
            if (!still)
            {
                break;
            }

            std::string type = listTokenized[pos];
            if (type == "let")
            {
                if (!letStatement(ofs, pos, nested))
                {
                    return false;
                }
            }
            if (type == "if")
            {
                if (!ifStatement(ofs, pos, nested))
                {
                    return false;
                }
            }
            if (type == "while")
            {
                if (!whileStatement(ofs, pos, nested))
                {
                    return false;
                }
            }
            if (type == "do")
            {
                if (!doStatement(ofs, pos, nested))
                {
                    return false;
                }
            }
            if (type == "return")
            {
                if (!returnStatement(ofs, pos, nested))
                {
                    return false;
                }
            }
        }
        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</statements>" << std::endl;
        return true;
    }

    bool subroutineBody(std::ofstream &ofs, int &pos, int nested)
    {
        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{};
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<subroutineBody>" << std::endl;
        nested++;

        exact.push_back("{");
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        // varDec
        while (1)
        {
            if (listTokenized[pos] != "var")
            {
                break;
            }
            if (!varDec(ofs, pos, nested))
            {
                return false;
            }
        }

        // statements
        if (!statements(ofs, pos, nested))
        {
            return false;
        }
        exact.clear();
        type_statement.clear();
        exact.push_back("}");
        if (!checkType(ofs, pos, nested, exact, type_statement))
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</subroutineBody>" << std::endl;
        return true;
    }

    bool subroutineDec(std::ofstream &ofs, int &pos, int nested)
    {
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "<subroutineDec>" << std::endl;
        nested++;
        print(ofs, pos, nested);
        pos++;
        std::vector<std::string> exact{};
        std::vector<std::string> type_statement{};

        // void | type
        exact.clear();
        exact.push_back("void");
        exact.push_back("int");
        exact.push_back("char");
        exact.push_back("boolean");
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // subroutineName
        exact.clear();
        type_statement.clear();
        type_statement.push_back("identifier");
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // symbol (
        exact.clear();
        exact.push_back("(");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // parameter list
        if (!parameterList(ofs, pos, nested))
            return false;

        // symbol )
        exact.clear();
        exact.push_back(")");
        type_statement.clear();
        if (!checkType(ofs, pos, nested, exact, type_statement))
            return false;

        // subroutine body
        if (!subroutineBody(ofs, pos, nested))
        {
            return false;
        }

        nested--;
        for (int i = 0; i < nested; ++i)
            ofs << "  ";
        ofs << "</subroutineDec>" << std::endl;
        return true;
    }

    bool CompileClass(std::ofstream &ofs, int &pos, int nested)
    {
        ofs << "<class>" << std::endl;
        nested++;

        print(ofs, pos, nested);
        pos++;

        if (typeStr(listTokenized[pos]) != "identifier")
        {
            std::cout << "Expected: identifier but it is " << listTokenized[pos] << std::endl;
            return false;
        }

        print(ofs, pos, nested);
        pos++;

        if (listTokenized[pos] != "{")
        {
            std::cout << "Expected: { but it is " << listTokenized[pos] << std::endl;
        }
        print(ofs, pos, nested);
        pos++;

        while (1)
        {
            std::string x = listTokenized[pos];
            if (x != "static" && x != "field")
                break;
            if (!classVarDec(ofs, pos, nested))
            {
                return false;
            }
        }

        while (1)
        {
            std::string x = listTokenized[pos];

            if (x != "constructor" && x != "function" && x != "method")
            {
                break;
            }

            if (!subroutineDec(ofs, pos, nested))
            {
                return false;
            }
        }

        if (listTokenized[pos] != "}")
        {
            std::cout << "Expected: } but it is " << listTokenized[pos] << std::endl;
        }
        print(ofs, pos, nested);
        pos++;
        ofs << "</class>" << std::endl;
        return true;
    }

    void CompilerEngine()
    {
        int pos = 0;
        std::string out_file = pathname.substr(0, (int)pathname.size() - 5);
        out_file = out_file + "KYM.xml";
        std::ofstream ofs(out_file);
        while (pos < (int)listTokenized.size())
        {
            if (listTokenized[pos] == "class")
            {
                bool ok = CompileClass(ofs, pos, 0);
                if (!ok)
                {
                    std::cout << "There's must be something wrong" << std::endl;
                    break;
                }
            }
            else
            {
                pos++;
            }
        }
    }

public:
    JackAnalyzer(std::string _pathname)
    {
        std::cout << "Trying to open the file " << _pathname << std::endl;
        pathname = _pathname;
        ready = true;

        // Check readability of the file
        std::ifstream ifs(_pathname);
        if (!ifs.good())
        {
            ready = false;
        }
        ifs.close();

        // Can't read the file
        if (!ready)
        {
            std::cout << "Can't read file: " << _pathname << std::endl;
            return;
        }

        // file is ok
        std::cout << "Successfully" << std::endl;
        ready = true;
        listTokenized.clear();
    }

    void execute()
    {
        if (!ready)
        {
            std::cout << "Jack Analyzer is not ready to execute" << std::endl;
            return;
        }
        Tokenize(pathname);
        CompilerEngine();
    }
};

int main(int argc, char **argv)
{

    for (int i = 1; i < argc; ++i)
    {
        std::string pathname(argv[i]);
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(argv[i])) != NULL)
        {
            /* print all the files and directories within directory */
            std::vector<std::string> listJackFile;
            while ((ent = readdir(dir)) != NULL)
            {
                std::string file(ent->d_name);
                if (checkSuffix(file, ".jack"))
                {
                    std::string fullpath = pathname + "/" + file;
                    listJackFile.push_back(fullpath);
                }
            }

            for (std::string jackFile : listJackFile)
            {
                JackAnalyzer jackAnalyzer(jackFile);
                jackAnalyzer.execute();
            }

            closedir(dir);
        }
        else
        {
            if ((int)pathname.size() >= 6)
            {
                if (checkSuffix(pathname, ".jack"))
                {
                    JackAnalyzer jackAnalyzer(pathname);
                    jackAnalyzer.execute();
                }
                else
                {
                    std::cout << "File " << pathname << " is not a .jack file" << std::endl;
                }
            }
        }
    }

    return 0;
}