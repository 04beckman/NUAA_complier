/***********************************************
 * 词法分析器
 * 编译环境：Visual Studio 2017
 ***********************************************/
#include <iostream>
#include <string>
#include <Windows.h>
using namespace std;

/* 单词编码 */
enum TokenCode
{
    /*未定义*/
    TK_UNDEF = 0,

    /* 关键字 */
    KW_VOID,   // void关键字
    KW_MAIN,   // main关键字
    KW_INT,    // int关键字
    KW_DOUBLE, // double关键字
    KW_FOR,    // for关键字
    KW_WHILE,  // while关键字
    KW_SWITCH, // switch关键字
    KW_CASE,   // case关键字
    KW_IF,     // if关键字
    KW_ELSE,   // else关键字
    KW_RETURN, // return关键字

    /* 运算符 */
    TK_PLUS,   //+加号
    TK_MINUS,  //-减号
    TK_STAR,   //*乘号
    TK_DIVIDE, /// 除号
    TK_ASSIGN, //=赋值运算符
    TK_EQ,     //==等于号
    TK_LT,     //<小于号
    TK_LEQ,    //<=小于等于号
    TK_GT,     //>大于号
    TK_GEQ,    //>=大于等于号

    /* 分隔符 */
    TK_OPENPA,    //(左圆括号
    TK_CLOSEPA,   //)右圆括号
    TK_OPENBR,    //[左中括号
    TK_CLOSEBR,   //]右中括号
    TK_BEGIN,     //{左大括号
    TK_END,       //}右大括号
    TK_COMMA,     //,逗号
    TK_SEMOCOLOM, //;分号

    /* 常量 */
    TK_INT,    // 整型常量
    TK_DOUBLE, // 浮点型常量

    /* 标识符 */
    TK_IDENT
};

/******************************************全局变量*****************************************************/
TokenCode code = TK_UNDEF;                                                                                        // 记录单词的种别码
const int MAX = 11;                                                                                               // 关键字数量
int row = 1;                                                                                                      // 记录字符所在的行数
string token = "";                                                                                                // 用于存储单词
char keyWord[][10] = {"void", "main", "int", "double", "for", "while", "switch", "case", "if", "else", "return"}; // 存储关键词

/**********************************************函数*****************************************************/

/********************************************
 * 功能：打印词法分析的结果
 * code：单词对应的种别码
 * token：用于存储单词
 * row：单词所在的行数
 *********************************************/
void print(TokenCode code)
{

    switch (code)
    {
    /*未识别的符号*/
    case TK_UNDEF:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED); // 未识别的符号为红色
        cout << '(' << code << ',' << token << ")" << "未识别的符号在第" << row << "行。" << endl;
        return;
        break;
    /*关键字*/
    case KW_VOID:                                                                                         // void关键字
    case KW_MAIN:                                                                                         // main关键字
    case KW_INT:                                                                                          // int关键字
    case KW_DOUBLE:                                                                                       // double关键字
    case KW_FOR:                                                                                          // for关键字
    case KW_WHILE:                                                                                        // while关键字
    case KW_SWITCH:                                                                                       // switch关键字
    case KW_CASE:                                                                                         // case关键字
    case KW_IF:                                                                                           // if关键字
    case KW_ELSE:                                                                                         // else关键字
    case KW_RETURN:                                                                                       // return关键字
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE); // 关键字为蓝色
        break;

    /* 运算符 */
    case TK_PLUS:   //+加号
    case TK_MINUS:  //-减号
    case TK_STAR:   //*乘号
    case TK_DIVIDE: /// 除号
    case TK_ASSIGN: //=赋值运算符
    case TK_EQ:     //==等于号
    case TK_LT:     //<小于号
    case TK_LEQ:    //<=小于等于号
    case TK_GT:     //>大于号
    case TK_GEQ:    //>=大于等于号

    /* 分隔符 */
    case TK_OPENPA:                                                                                        //(左圆括号
    case TK_CLOSEPA:                                                                                       //)右圆括号
    case TK_OPENBR:                                                                                        //[左中括号
    case TK_CLOSEBR:                                                                                       //]右中括号
    case TK_BEGIN:                                                                                         //{左大括号
    case TK_END:                                                                                           //}右大括号
    case TK_COMMA:                                                                                         //,逗号
    case TK_SEMOCOLOM:                                                                                     //;分号
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN); // 运算符和分隔符为绿色
        break;

    /* 常量 */
    case TK_INT:                                                                                                            // 整型常量
    case TK_DOUBLE:                                                                                                         // 浮点型常量
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN); // 常量为黄色
        if (token.find('.') == token.npos)
            cout << '(' << code << ',' << atoi(token.c_str()) << ")" << endl; // 单词为整型
        else
            cout << '(' << code << ',' << atof(token.c_str()) << ")" << endl; // 单词为浮点型
        return;
        break;
    /* 标识符 */
    case TK_IDENT:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY); // 关键字为灰色
        break;
    default:
        break;
    }
    cout << '(' << code << ',' << token << ")" << endl;
}

/********************************************
 * 功能：判断是否是关键字
 * MAX：关键字数量
 * token：用于存储单词
 *********************************************/
bool isKey(string token)
{
    for (int i = 0; i < MAX; i++)
    {
        if (token.compare(keyWord[i]) == 0)
            return true;
    }
    return false;
}

/********************************************
 * 功能：返回关键字的内码值
 * MAX：关键字数量
 * token：用于存储单词
 *********************************************/
int getKeyID(string token)
{
    for (int i = 0; i < MAX; i++)
    { // 关键字的内码值为keyWord数组中对应的下标加1
        if (token.compare(keyWord[i]) == 0)
            return i + 1;
    }
    return -1;
}

/********************************************
 * 功能：判断一个字符是否是字母
 * letter：被判断的字符
 *********************************************/
bool isLetter(char letter)
{
    if ((letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z'))
        return true;
    return false;
}

/********************************************
 * 功能：判断一个字符是否是数字
 * digit：被判断的字符
 *********************************************/
bool isDigit(char digit)
{
    if (digit >= '0' && digit <= '9')
        return true;
    return false;
}

/********************************************
 * 功能：词法分析
 * fp：文件指针
 * code：单词对应的种别码
 * token：用于存储单词
 * row：单词所在的行数
 *********************************************/
void lexicalAnalysis(FILE *fp)
{
    char ch;                        // 用于存储从文件中获取的单个字符
    while ((ch = fgetc(fp)) != EOF) // 未读取到文件尾，从文件中获取一个字符
    {
        token = ch;                                // 将获取的字符存入token中
        if (ch == ' ' || ch == '\t' || ch == '\n') // 忽略空格、Tab和回车
        {
            if (ch == '\n') // 遇到换行符，记录行数的row加1
                row++;
            continue; // 继续执行循环
        }
        else if (isLetter(ch)) // 以字母开头，关键字或标识符
        {
            token = "";                         // token初始化
            while (isLetter(ch) || isDigit(ch)) // 非字母或数字时退出，将单词存储在token中
            {
                token.push_back(ch); // 将读取的字符ch存入token中
                ch = fgetc(fp);      // 获取下一个字符
            }
            // 文件指针后退一个字节，即重新读取上述单词后的第一个字符
            fseek(fp, -1L, SEEK_CUR);
            if (isKey(token)) // 关键字
                code = TokenCode(getKeyID(token));
            else                 // 标识符
                code = TK_IDENT; // 单词为标识符
        }
        else if (isDigit(ch)) // 无符号常数以数字开头
        {
            int isdouble = 0;   // 标记是否为浮点数
            token = "";         // token初始化
            while (isDigit(ch)) // 当前获取到的字符为数字
            {
                token.push_back(ch); // 读取数字，将其存入token中
                ch = fgetc(fp);      // 从文件中获取下一个字符
                // 该单词中第一次出现小数点
                if (ch == '.' && isdouble == 0)
                {
                    // 小数点下一位是数字
                    if (isDigit(fgetc(fp)))
                    {
                        isdouble = 1;             // 标记该常数中已经出现过小数点
                        fseek(fp, -1L, SEEK_CUR); // 将超前读取的小数点后一位重新读取
                        token.push_back(ch);      // 将小数点入token中
                        ch = fgetc(fp);           // 读取小数点后的下一位数字
                    }
                }
            }
            if (isdouble == 1)
                code = TK_DOUBLE; // 单词为浮点型
            else
                code = TK_INT; // 单词为整型
            // 文件指针后退一个字节，即重新读取常数后的第一个字符
            fseek(fp, -1L, SEEK_CUR);
        }
        else
            switch (ch)
            {
            /*运算符*/
            case '+':
                code = TK_PLUS; //+加号
                break;
            case '-':
                code = TK_MINUS; //-减号
                break;
            case '*':
                code = TK_STAR; //*乘号
                break;
            case '/':
                code = TK_DIVIDE; // 除号
                break;
            case '=':
            {
                ch = fgetc(fp); // 超前读取'='后面的字符
                if (ch == '=')  //==等于号
                {
                    token.push_back(ch); // 将'='后面的'='存入token中
                    code = TK_EQ;        // 单词为"=="
                }
                else
                {                             //=赋值运算符
                    code = TK_ASSIGN;         // 单词为"="
                    fseek(fp, -1L, SEEK_CUR); // 将超前读取的字符重新读取
                }
            }
            break;
            case '<':
            {
                ch = fgetc(fp); // 超前读取'<'后面的字符
                if (ch == '=')  //<=小于等于号
                {
                    token.push_back(ch); // 将'<'后面的'='存入token中
                    code = TK_LEQ;       // 单词为"<="
                }
                else
                {                             //<小于号
                    code = TK_LT;             // 单词为"<"
                    fseek(fp, -1L, SEEK_CUR); // 将超前读取的字符重新读取
                }
            }
            break;
            case '>':
            {
                ch = fgetc(fp); // 超前读取'>'后面的字符
                if (ch == '=')  //>=大于等于号
                {
                    token.push_back(ch); // 将'>'后面的'='存入token中
                    code = TK_GEQ;       // 单词为">="
                }
                else
                {                             //>大于号
                    code = TK_GT;             // 单词为">"
                    fseek(fp, -1L, SEEK_CUR); // 将超前读取的字符重新读取
                }
            }
            break;
            /*分界符*/
            case '(':
                code = TK_OPENPA; //(左圆括号
                break;
            case ')':
                code = TK_CLOSEPA; //)右圆括号
                break;
            case '[':
                code = TK_OPENBR; //[左中括号
                break;
            case ']':
                code = TK_CLOSEBR; //]右中括号
                break;
            case '{':
                code = TK_BEGIN; //{左大括号
                break;
            case '}':
                code = TK_END; //}右大括号
                break;
            case ',':
                code = TK_COMMA; //,逗号
                break;
            case ';':
                code = TK_SEMOCOLOM; //;分号
                break;
            // 未识别符号
            default:
                code = TK_UNDEF;
            }
        print(code); // 打印词法分析结果
    }
}

int main()
{
    string filename; // 文件路径
    FILE *fp;        // 文件指针
    cout << "请输入源文件名：" << endl;
    while (true)
    {
        cin >> filename;                                // 读取文件路径
        if ((fopen_s(&fp, filename.c_str(), "r")) == 0) // 打开文件
            break;
        else
            cout << "路径输入错误！" << endl; // 读取失败
    }
    cout << "/=***************************词法分析结果***************************=/" << endl;
    lexicalAnalysis(fp);                                                                                                                  // 词法分析
    fclose(fp);                                                                                                                           // 关闭文件
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // 字体恢复原来的颜色
    system("pause");
    return 0;
}
