#include <bits/stdc++.h>
#define maxnum 50
#define Terminator 1
#define NonTerminator 0
#define Table_width 20
#define bufsize 300
typedef std::pair<int, int> Pair2;

// 产生式存放数组（不含公共左因子、不含左递归）
char start_char;
int cnt_nonT = 0;
int cnt_term = 0;
int lines = 0;
int table[maxnum][maxnum];
int old_follow_size[maxnum];
char str[bufsize];
std::string grammar[maxnum];
std::vector<char> nonT;
std::vector<char> term;
std::vector<char> firstP[maxnum];
std::vector<char> firstN[maxnum];
std::vector<char> follow[maxnum];
int indexOf(const std::vector<char> &v, char x)
{
    std::vector<char>::const_iterator it = find(v.begin(), v.end(), x);
    return it == v.end() ? -1 : it - v.begin();
}
void add_unique_item(std::vector<char> &v, char c)
{
    if (indexOf(v, c) == -1)
        v.push_back(c);
}
int typeOf(char x)
{
    if (indexOf(nonT, x) != -1)
        return NonTerminator;
    if (indexOf(term, x) != -1)
        return Terminator;
    return -1;
}
int indexOfNonT(char x)
{
    return indexOf(nonT, x);
}
int indexOfTerm(char x)
{
    return indexOf(term, x);
}

void INPUT()
{
    lines = 0;
    while (std::cin >> grammar[lines])
        lines++;
    std::cin.clear();
}
// 提取非终结符 和 终结符, 保存到 nonT 和 term 中
void pick_up()
{
    bool vis[256] = {0};
    for (int i = 0; i < lines; i++)
    {
        char cur = grammar[i][0];
        if (!vis[(int)cur])
        {
            nonT.push_back(cur);
            vis[(int)cur] = true;
        }
    }
    // 不是非终结符的都是终结符
    for (int i = 0; i < lines; i++)
    {
        int n = grammar[i].length();
        for (int j = 3; j < n; j++)
        {
            char cur = grammar[i][j];
            if (!vis[(int)cur])
            {
                term.push_back(cur);
                vis[(int)cur] = true;
            }
        }
    }
    term.push_back('#');
    cnt_nonT = nonT.size();
    cnt_term = term.size();
    start_char = grammar[0][0];
}
// 提取非终结符 和 终结符, 保存到 nonT 和 term 中
void getfirst(char NonTer)
{
    int index_NonTer = indexOfNonT(NonTer); // 查询 字符 x 在 nonT 中下标
    for (int j = 0; j < lines; j++)         // 遍历产生式
    {
        if (grammar[j][0] != NonTer)
            continue;
        bool keep = true; // 是否继续向后检查
        for (char c : grammar[j].substr(3))
        {
            if (!keep)
                break;
            if (typeOf(c) == Terminator)
            { // 终结符
                add_unique_item(firstN[index_NonTer], c);
                keep = false;
            }
            else
            {
                // 非终结符的 first
                keep = false;
                getfirst(c); // 先递归求解一下
                for (char b : firstN[indexOfNonT(c)])
                {
                    if (b == '@')
                        keep = true;
                    else
                        add_unique_item(firstN[index_NonTer], b);
                }
            }
        }
        if (keep)
            add_unique_item(firstN[index_NonTer], '@');
    }
}
void GETFIRST()
{
    for_each(nonT.begin(), nonT.end(), getfirst);
}
void GETFIRSTP()
{
    for (int i = 0; i < lines; i++)
    {
        bool keep = true; // 是否继续向后检查
        for (char c : grammar[i].substr(3))
        {
            if (!keep)
                break;
            if (typeOf(c) == Terminator)
            {
                add_unique_item(firstP[i], c);
                keep = false;
            }
            else
            { // 非终结符
                // 把这个非终结符的 first 收了
                keep = false;
                for (char b : firstN[indexOfNonT(c)])
                {
                    if (b == '@')
                        keep = true;
                    else
                        add_unique_item(firstP[i], b);
                }
            }
        }
        // 如果最后 keep 仍为 true, 则说明 产生式 右部可空
        if (keep)
            add_unique_item(firstP[i], '@');
    }
}
std::vector<Pair2> to_do;
std::vector<char> firstbeta(int i, int pos)
{
    std::vector<char> res;
    bool keep = true; // 是否继续向后检查
    for (char c : grammar[i].substr(pos))
    {
        if (!keep)
            break;
        if (typeOf(c) == Terminator)
        {
            add_unique_item(res, c);
            keep = false;
        }
        else
        { // 非终结符
            // 把这个非终结符的 first 收了
            keep = false;
            for (char b : firstN[indexOfNonT(c)])
            {
                if (b == '@')
                    keep = true;
                else
                    add_unique_item(res, b);
            }
        }
    }
    // 如果最后 keep 仍为 true, 则说明 产生式 右部可空
    if (keep)
        add_unique_item(res, '@');
    return res;
}
void singleFollow(char ch, std::vector<char> &inner_follow)
{
    int index_ch = indexOfNonT(ch);
    for (int k = 0; k < lines; k++)
    {
        std::string &cur_grammar = grammar[k];
        int index_head = indexOfNonT(cur_grammar[0]);
        int len = cur_grammar.length();
        for (int i = 3; i < len; i++)
        {
            if (cur_grammar[i] != ch)
                continue;
            auto next_first = firstbeta(k, i + 1); // RULE2
            bool ok = false;
            for (auto c : next_first)
            {
                if (c == '@')
                    ok = true;
                else
                    add_unique_item(inner_follow, c);
            }
            if (ok && index_head != index_ch)
                to_do.push_back(std::make_pair(index_head, index_ch)); // RULE3
        }
    }
}
bool isSameSize()
{
    for (int i = 0; i < cnt_nonT; i++)
    {
        if (old_follow_size[i] != follow[i].size())
            return false;
    }
    return true;
}
void copy_from_to(std::vector<char> &from, std::vector<char> &to)
{
    for (char c : from)
    {
        add_unique_item(to, c);
    }
}
void GETFOLLOW()
{
    follow[indexOfNonT(start_char)].push_back('#'); // RULE1
    to_do.clear();
    for (int i = 0; i < cnt_nonT; i++)
    {
        singleFollow(nonT[i], follow[i]);
    }
    do
    {
        for (int i = 0; i < cnt_nonT; i++)
        {
            old_follow_size[i] = follow[i].size();
        }
        for (auto p : to_do)
        {
            copy_from_to(follow[p.first], follow[p.second]);
        }
    } while (!isSameSize()); // 只要不一致，就重复
}

void generateTable()
{
    memset(table, -1, sizeof table);
    for (int k = 0; k < lines; k++)
    {
        bool hasEmpty = false; // firstP[k] 中是否含有@
        int i = indexOfNonT(grammar[k][0]);
        for (char c : firstP[k])
        { // RULE(2)
            if (c == '@')
            {
                hasEmpty = true;
            }
            else
            {
                int j = indexOfTerm(c);
                table[i][j] = k; // 将产生式 在 grammar 中的下标 填入分析表中
            }
        }
        if (hasEmpty)
        { // RULE(3)
            for (char c : follow[i])
            {
                int j = indexOfTerm(c);
                table[i][j] = k; // 将产生式 在 grammar 中的下标 填入分析表中
            }
        }
    }
}

void SHOWFIRST()
{
    std::cout << "所有非终结符的 FIRST 集：" << std::endl;
    for (int i = 0; i < cnt_nonT; i++)
    {
        std::cout << "FIRST(" << nonT[i] << ") = {";
        for (char fi : firstN[i])
        {
            std::cout << " " << fi;
        }
        std::cout << " }" << std::endl;
    }
    std::cout << std::endl;
}
void SHOWFOLLOW()
{
    std::cout << "所有非终结符的 FOLLOW 集：" << std::endl;
    for (int i = 0; i < cnt_nonT; i++)
    {
        std::cout << "FOLLOW(" << nonT[i] << ") = {";
        for (char fi : follow[i])
        {
            std::cout << " " << fi;
        }
        std::cout << " }" << std::endl;
    }
    std::cout << std::endl;
}
void SHOWTABLE()
{
    int output_width = 10;
    std::cout << "预测分析表：" << std::endl;
    std::cout << std::setw(output_width) << " ";
    for (char char_term : term)
    {
        if (char_term == '@')
            continue;
        std::cout << std::setw(output_width) << char_term;
    }
    std::cout << std::endl;
    for (int i = 0; i < cnt_nonT; i++)
    {
        std::cout << std::setw(output_width) << nonT[i];
        for (int j = 0; j < cnt_term; j++)
        {
            if (term[j] == '@')
                continue;
            if (table[i][j] == -1)
                std::cout << std::setw(output_width) << " ";
            else
                std::cout << std::setw(output_width) << grammar[table[i][j]];
        }
        std::cout << std::endl;
    }
}
// 打印当前状态
void printCurrentStatus(const char *s1, const char *s2, const std::string &s3)
{
    std::cout << std::setw(Table_width) << s1 << std::setw(Table_width) << s2 << std::setw(Table_width) << s3 << std::endl;
}
// 分析函数
void analyze(const char *sentence, int len)
{
    char str[300];
    strcpy(str, sentence);
    if (len == -1)
        len = strlen(str);
    str[len++] = '#'; // 在末尾加入 '#'
    str[len] = 0;
    char stk[200]; // 数组模拟栈
    int pos = 0;
    stk[pos++] = '#';
    stk[pos++] = start_char;
    std::cout << std::setw(Table_width) << "栈" << std::setw(Table_width) << "输入" << std::setw(Table_width) << "动作" << std::endl;
    stk[pos] = 0;
    printCurrentStatus(stk, str, std::string("初始"));
    for (int i = 0; i < len;)
    {
        char top = stk[--pos];
        stk[pos] = 0;
        if (top == '#')
        {
            if (str[i] == '#')
            {
                printCurrentStatus("#", "#", std::string("接受"));
                std::cout << "分析成功！" << std::endl;
                break;
            }
            else
            {
                printCurrentStatus(stk, str + i, std::string("错误！"));
                std::cerr << "分析栈栈顶为'#'，但是输入尚未结束。" << std::endl;
                return;
            }
        }
        // 如果栈顶不是 '#'
        int current_type = typeOf(top);
        if (current_type == Terminator)
        {
            if (top == str[i])
            {
                i++; // 输入字符提取走
                std::string res("匹配 ");
                printCurrentStatus(stk, str + i, res + top);
            }
            else
            {
                printCurrentStatus(stk, str + i, std::string("错误！"));
                std::cerr << "分析栈栈顶为终结符 '" << top << "'，但是输入符号为 '" << str[i] << "'" << std::endl;
                return;
            }
        }
        else if (current_type == NonTerminator)
        {
            int g_index = table[indexOfNonT(top)][indexOfTerm(str[i])];
            if (g_index != -1)
            {
                if (grammar[g_index][3] != '@')
                { // 如果右边 是 @，那么不向栈中增加东西
                    // 将产生式中右部内容 倒序 入栈
                    for (int i = grammar[g_index].size() - 1; i >= 3; i--)
                    {
                        stk[pos++] = grammar[g_index][i];
                    }
                    stk[pos] = 0;
                }
                printCurrentStatus(stk, str + i, grammar[g_index]);
            }
            else
            {
                printCurrentStatus(stk, str + i, std::string("错误！"));
                std::cerr << "分析表 table[" << top << "][" << str[i] << "] 项为空。" << std::endl;
                return;
            }
        }
        else
        {
            std::cerr << "解析错误！" << std::endl;
            return;
        }
    }
}
int main(void)
{
    std::cout << "请输入文法:" << std::endl;
    INPUT();
    pick_up();
    GETFIRST();
    GETFIRSTP();
    GETFOLLOW();
    SHOWFIRST();
    SHOWFOLLOW();
    SHOWTABLE();
    std::cout << "--------测试--------" << std::endl;
    while (true)
    {
        std::cout << "请输入一个句子：(退出请输入 q）" << std::endl;
        std::cin >> str;
        if (strcmp("q", str) == 0)
            break;
        analyze(str, strlen(str));
    }
}
/*
E->TA
A->+TA
A->@
T->FB
B->*FB
B->@
F->(E)
F->i
/*
Ctrl+Z
/*
i*i+i
*/