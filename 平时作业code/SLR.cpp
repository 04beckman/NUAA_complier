#include <bits/stdc++.h>
#include <Windows.h>

#define DEBUG
#define WHITE SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) // 白色
#define GREEN SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
#define RED SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_RED);
#define BLUE SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_BLUE);
#define MAX_F 1005

class PRODUCTION
{
public:
    std::string left, right;
    int from;
    int id;

    PRODUCTION(char s1[], char s2[], int x, int y)
    {
        left = s1;
        right = s2;
        from = x;
        id = y;
    }
    PRODUCTION(const std::string &s1, const std::string &s2, int x, int y)
    {
        left = s1;
        right = s2;
        from = x;
        id = y;
    }
    bool operator<(const PRODUCTION &a) const
    {
        if (left == a.left)
            return right < a.right;
        return left < a.left;
    }
    bool operator==(const PRODUCTION &a) const
    {
        return (left == a.left) && (right == a.right);
    }
    void show()
    {
        printf("%s->%s\n", left.c_str(), right.c_str());
    }
};

class CLOSURE
{
public:
    std::vector<PRODUCTION> C_PS;
    void show()
    {
        GREEN;
        for (int i = 0; i < C_PS.size(); i++)
            C_PS[i].show();
    }
    bool operator==(const CLOSURE &a) const
    {
        if (a.C_PS.size() != C_PS.size())
            return false;
        for (int i = 0; i < a.C_PS.size(); i++)
            if (C_PS[i] == a.C_PS[i])
                continue;
            else
                return false;
        return true;
    }
};

// Content(0, x);type=0表示移入，x表示转移到的状态
struct Content
{
    int type;
    int num;
    std::string out;
    Content() { type = -1; }
    Content(int a, int b)
        : type(a), num(b) {}
};

std::string steptostr(int x) // 将数字转为字符串
{
    std::stringstream sin;
    sin << x;
    std::string ret;
    sin >> ret;
    return ret;
}

template <class T>
std::string stktostr(std::vector<T> stk) // 将输入的所有元素拼接成一个字符串并返回

{
    std::stringstream sin;
    for (int i = 0; i < stk.size(); i++)
        sin << stk[i];
    std::string ret;
    sin >> ret;
    return ret;
}
std::vector<PRODUCTION> produtions;
std::map<std::string, std::vector<int>> dic;      // 存储文法符号与项目索引的映射  //字典将左部与当前项目索引关联
std::map<std::string, std::vector<int>> NTOP_set; // 存储文法符号与产生式索引的映射 //将文法的左部（非终结符）与文法规则的索引进行关联
std::map<std::string, bool> vis;                  // 文法符号是否已经访问//用于记录哪些文法的 FIRST 集已经计算过
std::map<std::string, std::set<char>> FIRST;
std::map<std::string, std::set<char>> FOLLOW;
std::vector<CLOSURE> Closures; // 包含了一个文法的项目簇
std::vector<PRODUCTION> items; // items中包含所有项目
std::string start = "S";
std::vector<char> T; // T 用来存放文法中的所有符号，包括终结符和非终结符。这些符号会用于构建 LR分析表
Content action[MAX_F][MAX_F];
char mid = '@';       //'.'
int go[MAX_F][MAX_F]; // go[i][ch]=j 表示在状态i下，当接收到输入符号ch时，状态机转移到的下一个状态j
int to[MAX_F];
bool used[MAX_F];
int GOTO[MAX_F][MAX_F];

// 构建项目表
void MAKE_ITEM()
{
    memset(to, -1, sizeof(-1)); // 初始化 to 数组，将所有元素设置为 -1（用于表示状态转换）

    // 将文法左部与当前文法规则的索引i填入VTOP_set中 //建立文法符号与产生式索引的映射
    for (int i = 0; i < produtions.size(); i++)
        NTOP_set[produtions[i].left].push_back(i);

    // 构建项目表，在每个文法的右部逐步插入ch=@
    for (int i = 0; i < produtions.size(); i++)
    {
        for (int j = 0; j <= produtions[i].right.size(); j++)
        {
            std::string temp = produtions[i].right;
            temp.insert(temp.begin() + j, mid);              // 在每个文法的右部逐步插入ch=@
            dic[produtions[i].left].push_back(items.size()); // 将当前项的索引加入到 dic 中，字典将左部与当前项目索引关联

            // 将当前状态与下一个状态通过to连接起来
            if (j) // 如果 j 不为 0，表示插入的位置不是在开头
                to[items.size() - 1] = items.size();

            // 将当前项目（左部、右部和回退状态）添加到项目集合 items 中
            items.push_back(PRODUCTION(produtions[i].left, temp, i, items.size()));
        }
    }
}

// 对于求First集，详见书上P78页算法
void dfs_first(const std::string &x) // 深度优先搜索（DFS）算法，用于计算一个文法符号的 FIRST 集
{
    if (vis[x]) // 如果当前符号已经被访问过，直接返回
        return;
    vis[x] = true;                     // 标记当前符号为已访问
    std::vector<int> PS = NTOP_set[x]; // 获取与当前文法符号 x 相关的所有产生式的索引
    for (int i = 0; i < PS.size(); i++)
    {
        std::string left = produtions[PS[i]].left;   // 当前产生式的左部
        std::string right = produtions[PS[i]].right; // 当前产生式的右部

        // 遍历产生式右部的每个字符
        for (int j = 0; j < left.size(); j++)
        {
            // 如果遇到非终结符号（大写字母），递归计算其 FIRST 集
            if ('A' <= right[j] && 'Z' >= right[j])
            {
                // subtri（j，1）的作用：从起始位置为j处，截取一个字符并返回
                dfs_first(right.substr(j, 1));                  // 对于右部的每个非终结符，递归调用 dfs 计算其 FIRST 集
                std::set<char> tmp = FIRST[right.substr(j, 1)]; // 获取该非终结符的 FIRST 集
                bool flag = true;                               // 用于标记是否遇到了空串 '~'，表示该非终结符可以推导出空串，继续递归判断，直至没有空串退出循环
                for (auto &it : tmp)
                {
                    if (it == '$') // 如果遇到空串 '~'，则标记 flag 为 false，表示右部可以推导为空串
                        flag = false;
                    FIRST[left].insert(it); // 将 FIRST 集中的元素添加到当前文法符号的 FIRST 集
                }
                // 如果没有遇到空串，表示可以停止向下递归，结束当前文法符号的 FIRST 集计算
                if (flag)
                    break;
            }
            // 如果是终结符号（小写字母），直接将其加入当前文法符号的 FIRST 集
            else
            {
                FIRST[left].insert(right[j]);
                break;
            }
        }
    }
}

// 构造first集
void MAKE_FIRST()
{
    for (auto iter : dic)
        // vis->first表示文法的左部
        if (vis[iter.first]) // 如果该文法符号的 FIRST 集已经计算过了，跳过
            continue;
        else
            dfs_first(iter.first); // 对未计算过的文法符号调用 dfs 计算其 FIRST 集
}

// 合并两个 FOLLOW 集
void ADDLTR(const std::string &str1, const std::string &str2)
{
    std::set<char> &left = FOLLOW[str1];  // 获取 str1 的 FOLLOW 集
    std::set<char> &right = FOLLOW[str2]; // 获取 str2 的 FOLLOW 集
    for (auto it : left)                  // 将 str1 的 FOLLOW 集中的元素加入 str2 的 FOLLOW 集
        right.insert(it);
}

// bool _check(const std::vector<int> &PS, const std::string str)
// {
//     for (int i = 0; i < PS.size(); i++)
//     {
//         if (produtions[PS[i]].right == str)
//             return true;
//     }
//     return false;
// }

// 计算所有非终结符的 FOLLOW 集
// follow集判断结束的标准是:不再增大为止。
// make_follow() 是一个计算文法中非终结符 FOLLOW 集的函数，使用迭代的方式不断更新 FOLLOW 集，直到没有更多的变化为止。
// 通过检查每个产生式右部的符号，确定哪些符号应当加入 FOLLOW 集。
void MAKE_FOLLOW() // 未处理空的情况
{
    FOLLOW[start].insert('#'); //! R1
    while (true)
    {
        bool ok = false;          // 标志 FOLLOW 集是否发生更新
        for (auto it2 : NTOP_set) // 遍历每个非终结符
        {
            std::vector<int> PS = it2.second;   // 获取该非终结符的产生式列表
            for (int i = 0; i < PS.size(); i++) // 遍历该非终结符的每个产生式
            {
                PRODUCTION P = produtions[PS[i]]; // 获取当前产生式
                std::string left = P.left;
                std::string right = P.right;
                for (int j = right.size() - 1; j >= 0; j--) // 从右部的最后一个符号开始处理
                {
                    if ('A' <= right[j] && 'Z' >= right[j])
                    {
                        if (j == right.size() - 1) //! R3
                        {
                            // subtri（j，1）的作用：从起始位置为j处，截取一个字符并返回
                            //  获取该非终结符的 FOLLOW 集大小，用于判断是否有更新
                            int size_before = FOLLOW[right.substr(j, 1)].size();
                            ADDLTR(left, right.substr(j, 1)); // 例如：A->aB,则把FOLLOW(A)加入FOLLOW(B)
                            int size_after = FOLLOW[right.substr(j, 1)].size();
                            if (size_after > size_before) // 如果 FOLLOW 集有变化
                                ok = true;                // 设置 goon 标志，表示需要继续迭代
                        }
                        for (int k = j + 1; k < right.size(); k++) //! R2  //A -> ... X Y Z ...,这里我们正在处理右部的符号 X
                        {
                            if ('A' <= right[k] && 'Z' >= right[k]) // 如果遇到的是非终结符
                            {
                                std::string NT = right.substr(k, 1);            // 获取该符号
                                std::set<char> from = FIRST[NT];                // 获取该符号的 FIRST 集
                                std::set<char> to = FOLLOW[right.substr(j, 1)]; // 获取当前符号的 FOLLOW 集
                                int size_before = FOLLOW[right.substr(j, 1)].size();
                                for (auto it1 : from) // 将 FIRST 集中的符号除了空串外加入到 FOLLOW 集
                                    if (it1 != '$')   // 跳过空串 '~'
                                        to.insert(it1);
                                int size_after = FOLLOW[right.substr(j, 1)].size();
                                if (size_after > size_before) // 比较是否增大
                                    ok = true;                // 继续迭代
                            }
                            else // 如果遇到的是终结符
                            {
                                int size_before = FOLLOW[right.substr(j, 1)].size();
                                FOLLOW[right.substr(j, 1)].insert(right[k]); // 将当前终结符加入到 FOLLOW 集
                                int size_after = FOLLOW[right.substr(j, 1)].size();
                                if (size_after > size_before)
                                    ok = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!ok) // 如果没有 FOLLOW 集更新，则退出循环
            break;
    }
}

// 生成所有的闭包集（Closure）。每个闭包集表示在某个状态下，可能的所有产生式（项目）。
void MAKE_CLOURSE()
{
    bool vis[MAX_F];                       // 初始化标记数组
    for (int i = 0; i < items.size(); i++) // 遍历所有项目，找到起始项目
    {
        // 检查当前项目是否符合条件（即 S -> CH），作为闭包集的起点
        if (items[i].left[0] == 'S' && items[i].right[0] == mid)
        {
            CLOSURE temp;
            std::string &str = items[i].right;         // 当前项目的右侧部分
            std::vector<PRODUCTION> &C_PS = temp.C_PS; // 存放项目元素的容器
            C_PS.push_back(items[i]);                  // 将当前项目添加到闭包中
            int pos = 0;
            for (pos = 0; pos < str.size(); pos++)
                if (str[pos] == mid)
                    break;
            // 通过广度优先搜索（BFS），扩展闭包集。
            // 对于点CH后面的每个符号，如果是非终结符，则将相关的产生式加入闭包集
            // 如果项目的右侧没有 CH，继续处理其后续部分
            memset(vis, 0, sizeof(vis)); // 清空 has 数组
            vis[i] = true;               // 标记当前项目已经被处理
            if (pos != str.size() - 1)   // 如果点不是在末尾
            {
                std::queue<std::string> q;
                q.push(str.substr(pos + 1, 1)); // 将 mid 后的第一个符号加入队列
                while (!q.empty())
                {
                    std::string u = q.front(); // 获取队首元素
                    q.pop();
                    std::vector<int> PS = dic[u]; // 获取符号 u 对应的项目ID集合
                    for (int j = 0; j < PS.size(); j++)
                    {
                        int tx = PS[j];
                        if (items[tx].right[0] == mid) // 如果项目的右侧是 mid
                        {
                            if (vis[tx]) // 如果该项目已经处理过，跳过
                                continue;
                            vis[tx] = 1; // 标记该项目已处理
                            // 如果下一个符号是大写字母
                            if ('A' <= items[tx].right[1] && 'Z' >= items[tx].right[1])
                                q.push(items[tx].right.substr(1, 1)); // 将下一个符号加入队列
                            C_PS.push_back(items[tx]);                // 将项目添加到当前闭包中
                        }
                    }
                }
            }
            Closures.push_back(temp); // 将当前闭包添加到项目集规范族中
        }
    }
    // 对每个闭包进行进一步处理，构建项目集规范族
    // 生成所有闭包的组合并去除重复。
    // 通过遍历已生成的闭包集，进一步生成新的闭包集，并确保所有闭包集都是唯一的，避免重复。
    for (int i = 0; i < Closures.size(); i++)
    {
        std::map<int, CLOSURE> temp; // 临时存储新的闭包
        for (int j = 0; j < Closures[i].C_PS.size(); j++)
        {
            std::string str = Closures[i].C_PS[j].right;
            int pos = 0;
            for (; pos < str.size(); pos++)
                if (str[pos] == mid) // 找到点的位置
                    break;
            if (pos == str.size() - 1) // 如果点在末尾，跳过
                continue;
            int y = str[pos + 1]; // 获取点后面的符号
            // 将点向后移动一位
            str.erase(str.begin() + pos);
            str.insert(str.begin() + pos + 1, mid);
            int PR_id;
            // 创建一个新的项目对象
            PRODUCTION cmp = PRODUCTION(Closures[i].C_PS[j].left, str, -1, -1);
            // 在项目集合中找到与新项目匹配的项目编号
            for (int k = 0; k < items.size(); k++)
            {
                if (items[k] == cmp)
                {
                    PR_id = k;
                    break;
                }
            }
            // 初始化标记数组
            memset(vis, false, sizeof(vis));
            std::vector<PRODUCTION> &C_PS = temp[y].C_PS;
            C_PS.push_back(items[PR_id]); // 将新项目加入闭包集合
            vis[PR_id] = true;
            pos++;
            if (pos != str.size() - 1) // 如果点后面还有符号，继续扩展
            {
                std::queue<std::string> q;
                q.push(str.substr(pos + 1, 1));
                while (!q.empty())
                {
                    std::string u = q.front();
                    q.pop();
                    std::vector<int> &id = dic[u];
                    for (int j = 0; j < id.size(); j++)
                    {
                        int tx = id[j];
                        if (items[tx].right[0] == mid)
                        {
                            if (vis[tx])
                                continue;
                            vis[tx] = true;
                            if ('A' <= items[tx].right[1] && 'Z' >= items[tx].right[1])
                                q.push(items[tx].right.substr(1, 1));
                            C_PS.push_back(items[tx]);
                        }
                    }
                }
            }
        }
        // 将临时闭包存入Closures中
        for (auto &it : temp)
            Closures.push_back(it.second);
        // 去除重复的项目集
        for (int i = 0; i < Closures.size(); i++)
            for (int j = i + 1; j < Closures.size(); j++)
                if (Closures[i] == Closures[j])
                    Closures.erase(Closures.begin() + j);
    }
}

// 构建一个符号集T，其中包含了所有文法中的终结符和非终结符
void GET_T()
{
    // bool used[MAX];
    // used 是一个用于标记字符是否已添加到符号集 V 中的数组
    memset(used, false, sizeof(used));          // 初始化 used 数组，将所有元素设置为 0
    for (int i = 0; i < produtions.size(); i++) // 遍历文法的每一条产生式
    {
        std::string str = produtions[i].left; // 获取当前产生式的左侧部分（非终结符）
        for (int j = 0; j < str.size(); j++)  // 遍历左部字符串的每个字符
        {
            if (used[str[j]]) // 如果字符已经被处理过，则跳过
                continue;
            if (str[j] == 'S')
                continue;
            used[str[j]] = true;
            T.push_back(str[j]); // 将字符添加到符号集 V 中
        }
        std::string str1 = produtions[i].right; // 获取当前产生式的右侧部分（可以是终结符或非终结符）
        for (int j = 0; j < str1.size(); j++)
        {
            if (used[str1[j]])
                continue;
            if (str1[j] == 'S')
                continue;
            used[str1[j]] = true;
            T.push_back(str1[j]);
        }
    }
    sort(T.begin(), T.end());
    // sort(T.begin(), T.end());
    T.push_back('#');
}

// 读入ch后产生式什么样子
void make_cmp(std::vector<PRODUCTION> &cmp1, int i, char ch)
{
    // char mid = '$';
    //  遍历 collection 中第 i 个元素的产生式
    for (int j = 0; j < Closures[i].C_PS.size(); j++)
    {
        std::string str = Closures[i].C_PS[j].right; // 获取产生式的右部
        int k;
        for (k = 0; k < str.size(); k++) // 遍历右部的字符串，寻找特殊符号 mid
            if (str[k] == mid)
                break;
        if (k != str.size() - 1 && str[k + 1] == ch) // 如果 mid=$ 后面的位置存在字符 ch，则修改右部
        {
            // 将CH=$移动到ch后
            str.erase(str.begin() + k);                                        // 删除 mid
            str.insert(str.begin() + k + 1, mid);                              // 在原位置的下一个位置插入 mid
            cmp1.push_back(PRODUCTION(Closures[i].C_PS[j].left, str, -1, -1)); // 将修改后的产生式添加到 cmp1 向量中
        }
    }
    sort(cmp1.begin(), cmp1.end()); // 对 cmp1 进行排序
}

// make_go函数用于构建转移表go，表示在某个状态下遇到某个符号后，转移到哪个状态
void MAKE_GO()
{
    memset(go, -1, sizeof(go)); // 将 go 数组中的所有元素初始化为 -1，表示未定义的转移
    int m = Closures.size();    // 闭包的数量
    // 外层循环遍历 T 进行处理
    for (int t = 0; t < T.size(); t++)
    {
        char ch = T[t];             // 当前处理的符号
        for (int i = 0; i < m; i++) // 遍历所有闭包集i
        {
            std::vector<PRODUCTION> cmp1;
            make_cmp(cmp1, i, ch); // 调用 make_cmp 函数获取从状态 i 在符号 ch 下的转移产生式
            if (cmp1.size() == 0)  // 如果没有匹配的产生式，跳过此符号的处理
                continue;
            // 对于每一个状态 j，遍历其产生式，寻找与 cmp1 中的产生式相同的规则，并将其存储在 cmp2 中。
            for (int j = 0; j < m; j++)
            {
                std::vector<PRODUCTION> cmp2;
                // 遍历闭包 j 中的所有产生式
                for (int k = 0; k < Closures[j].C_PS.size(); k++)
                {
                    std::string &str = Closures[j].C_PS[k].right; // 获取当前产生式的右侧部分
                    int pos;
                    // 查找产生式右侧中点的位置（即符号·的位置）
                    for (pos = 0; pos < str.size(); pos++)
                        if (str[pos] == mid)
                            break;
                    // 如果点前一个符号为 ch，则将该产生式加入 cmp2
                    if (pos && str[pos - 1] == ch)
                        cmp2.push_back(PRODUCTION(Closures[j].C_PS[k].left, str, -1, -1));
                }
                // 如果 cmp2 的大小与 cmp1 不一致，则跳过
                if (cmp2.size() != cmp1.size())
                    continue;
                sort(cmp2.begin(), cmp2.end());
                // 如果 cmp1 和 cmp2 完全相同，说明状态 i 在符号 ch 下的转移应该指向状态 j，因此更新 go[i][ch] = j。
                bool flag = true;
                for (int k = 0; k < cmp1.size(); k++)
                    if (cmp1[k] == cmp2[k])
                        continue;
                    else
                        flag = false;
                if (flag)
                    go[i][ch] = j;
            }
        }
    }
#ifdef DEBUG
    puts("---------------EDGE----------------------");
    std::stringstream sin;
    std::string out;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            for (int k = 0; k < MAX_F; k++)
                if (go[i][k] == j)
                {
                    sin.clear();
                    sin << "I" << i << "--" << (char)(k) << "--I" << j;
                    sin >> out;
                    printf("%s\n", out.c_str());
                }
#endif
}

// 构造LR分析表
void make_table()
{
    memset(GOTO, -1, sizeof(GOTO)); // 初始化Goto表，所有值设置为-1
    // 遍历所有集合中的项目，用于填充LR分析表中的Goto和action部分
    for (int i = 0; i < Closures.size(); i++)
        for (int j = 0; j < T.size(); j++)
        {
            char ch = T[j];
            int x = go[i][ch]; // 计算状态i和字符ch的转移，go[i][ch]表示从状态i经过字符ch的转移结果
            if (x == -1)
                continue;                      // 如果没有转移，跳过
            if ('A' >= ch || 'Z' <= ch)        // 如果ch是终结符，设置action表为移入操作
                action[i][ch] = Content(0, x); // type=0表示移入，x表示转移到的状态
            else                               // 如果ch是非终结符，设置Goto表
                GOTO[i][ch] = x;
        }
    // 遍历每个集合中的项目，用于填充action表的规约部分和接受部分
    for (int i = 0; i < Closures.size(); i++)
    {
        for (int j = 0; j < Closures[i].C_PS.size(); j++)
        {
            PRODUCTION &tt = Closures[i].C_PS[j];
            if (tt.right[tt.right.size() - 1] == mid) //$已经到最后，例如A->a$
            {
                if (tt.left[0] == 'S') // 如果产生式的左部是S，表示接受操作
                    action[i]['#'] = Content(2, -1);
                else // 如果是规约操作，遍历T集合的终结符，确定哪些终结符需要进行规约
                    for (int k = 0; k < T.size(); k++)
                    {
                        int y = T[k];
                        //.count()用来检查某个元素是否存在于集合中
                        if (!FOLLOW[tt.left].count(T[k])) // 如果终结符y不在follow集内，跳过
                            continue;
                        // type=1表示规约，tt.back是规约的产生式的编号
                        action[i][y] = Content(1, tt.from);
                    }
            }
        }
    }
}

void PRINT_ONE_STEP(std::string s1, std::string s2, std::string s3, std::string s4, std::string s5, std::string s6)
{
    printf("%-15s%-15s%-15s%-15s%-15s%-15s\n", s1.c_str(), s2.c_str(), s3.c_str(), s4.c_str(),
           s5.c_str(), s6.c_str());
}

void BottomUpAnalyse(std::string src)
{
    PRINT_ONE_STEP("步骤", "符号栈", "输入串", "状态栈", "动作", "GOTO");
    std::vector<char> op_stack;
    std::vector<int> st_stack;
    src += "#"; // 字符串末尾加上#作为结束标志
    op_stack.push_back('#');
    st_stack.push_back(0);
    int step = 1;
    for (int i = 0; i < src.size(); i++)
    {
        char u = src[i];
        int top = st_stack[st_stack.size() - 1]; // 获取栈顶状态
        Content &act = action[top][u];           // 栈顶遇到当前字符会进行下一步操作
        if (act.type == 0)                       // 移进
        {
            PRINT_ONE_STEP(steptostr(step), stktostr(op_stack), src.substr(i), stktostr(st_stack), act.out, "");
            step++;
            op_stack.push_back(u);
            st_stack.push_back(act.num);
        }
        else if (act.type == 1) // 归约
        {
            PRODUCTION &P = produtions[act.num];                    // 当前要归约的产生式 P
            int y = st_stack[st_stack.size() - P.right.size() - 1]; // 计算出归约后的栈顶状态 y
            int x = GOTO[y][P.left[0]];                             // 获取新的状态 x，即根据 Goto 表找到当前归约后的左部符号 tt.left[0] 对应的新状态
            PRINT_ONE_STEP(steptostr(step), stktostr(op_stack), src.substr(i), stktostr(st_stack), act.out, steptostr(x));
            step++;
            for (int j = 0; j < P.right.size(); j++)
            {
                st_stack.pop_back();
                op_stack.pop_back();
            }
            // 将产生式的左部符号 tt.left[0] 和新状态 x 入栈
            op_stack.push_back(P.left[0]);
            st_stack.push_back(x);
            i--; // 使得当前输入符号u重新分析，因为归约后可能需要重新查看这个符号
        }
        else if (act.type == 2) // ACCEPT
        {
            PRINT_ONE_STEP(steptostr(step), stktostr(op_stack), src.substr(i), stktostr(st_stack), act.out, "");
            step++;
        }
        else
            continue;
    }
}
void menu()
{
    std::cout << "************文法分析完毕**************" << std::endl;
    std::cout << "**    [1] 显示FIRST集                    " << std::endl;
    std::cout << "**    [2] 显示FOLLOW集                   " << std::endl;
    std::cout << "**    [3] 显示项目表                     " << std::endl;
    std::cout << "**    [4] 显示项目集规范族               " << std::endl;
    std::cout << "**    [5] SLR分析表和自下而上分析过程    " << std::endl;
    std::cout << "**    [6] 退出                           " << std::endl;
    std::cout << "**************************************" << std::endl;
}
void show_first()
{
    RED;
    std::cout << "--------------------FIRST集-----------------------" << std::endl;
    WHITE;
    for (auto &it : FIRST)
    {
        printf("FIRST(%s)={", it.first.c_str());
        bool flag = false;
        for (auto &it1 : it.second)
        {
            if (flag)
                printf(",");
            printf("%c", it1);
            flag = true;
        }
        puts("}");
    }
    RED;
    // 打印分隔线
    std::cout << "---------------------------------------------------" << std::endl;
    WHITE;
    system("pause");
}
void show_follow()
{
    RED;
    std::cout << "--------------------FOLLOW集-----------------" << std::endl;
    WHITE;
    for (auto &it : FOLLOW)
    {
        printf("FOLLOW(%s)={", it.first.c_str());
        std::set<char> &temp = it.second;
        // // if ( it->first[0] == 'S' )
        // temp.insert('#');
        bool flag = false;
        for (auto &it1 : temp)
        {
            if (flag)
                printf(",");
            printf("%c", it1);
            flag = true;
        }
        puts("}");
    }
    RED;
    std::cout << "----------------------------------------------" << std::endl;
    WHITE;
    system("pause");
}
void show_items()
{
    // //项目表
    RED;
    std::cout << "-------------------------项目表-------------------------" << std::endl;
    WHITE;
    printf("项目表形式：（看到）已输入串 @ （期待）输入串\n");
    for (int i = 0; i < items.size(); i++)
        printf("%d:%s->%s\n", items[i].id, items[i].left.c_str(), items[i].right.c_str());
    RED;
    puts("--------------------------------------------------------");
    WHITE;
    system("pause");
    // //
}
void show_closures()
{
    RED;
    std::cout << "--------------CLOSURES---------------------" << std::endl;
    for (int i = 0; i < Closures.size(); i++)
    {
        BLUE;
        std::cout << "CLOSURE-" << i << std::endl;
        Closures[i].show();
    }
    RED;
    puts("-------------------------------------------");
    WHITE;
    system("pause");
}
void show_slr_and_analyse()
{
    RED;
    std::cout << "------------------------------------------SLR分析表--------------------------------------------------------" << std::endl;
    WHITE;
    // 打印表头（包括每个终结符和非终结符的列）
    printf("%10s%5c%5s", "|", T[0], "|");
    for (int i = 1; i < T.size(); i++)
        printf("%5c%5s", T[i], "|");
    std::cout << std::endl;
    // 打印分隔线
    for (int i = 0; i < (T.size() + 1) * 10; i++)
        printf("-");
    std::cout << std::endl;
    std::stringstream sin; // 创建sin对象，使得它可以使用>>和<<

    // 打印每一行，即每个状态的Goto和action表内容
    for (int i = 0; i < Closures.size(); i++)
    {
        printf("%5d%5s", i, "|");
        for (int j = 0; j < T.size(); j++)
        {
            char ch = T[j];
            if (ch >= 'A' && ch <= 'Z') // 如果ch是非终结符，打印Goto表内容
            {
                if (GOTO[i][ch] == -1)
                    printf("%10s", "|");
                else
                    printf("%5d%5s", GOTO[i][ch], "|");
            }
            else // 如果ch是终结符，打印action表内容
            {
                sin.clear(); // 清除。可以确保之前的错误或标志不会影响后续操作。
                if (action[i][ch].type == -1)
                    printf("%10s", "|"); // 如果类型为 -1，表示没有有效操作
                else
                {
                    Content &temp = action[i][ch];
                    if (temp.type == 0) // 如果操作类型是 0，表示是移进
                        sin << "s";
                    if (temp.type == 1) // 如果操作类型是 1，表示是归约
                        sin << "r";
                    if (temp.type == 2) // 如果操作类型是 2，表示接受
                        sin << "acc";
                    if (temp.num != -1) // 如果 num 不为 -1，表示操作包含一个编号
                        sin << temp.num;
                    sin >> temp.out;
                    printf("%7s%3s", temp.out.c_str(), "|");
                }
            }
        }
        puts(""); // 输出换行符，结束当前行的打印
    }
    RED;
    for (int i = 0; i < (T.size() + 1) * 10; i++)
        printf("-");
    WHITE;
    std::cout << std::endl;
    BottomUpAnalyse("(i*i)+i");
    system("pause");
}

void show_productions()
{

    std::cout << "产生式：" << std::endl;
    for (const auto &production : produtions)
    {
        std::cout << production.left << " -> " << production.right << std::endl;
    }
}

int main(void)
{
    int n;
    char s[MAX_F];
    std::cout << "输入文法:(格式如下)" << std::endl;
    std::cout << "1\nE->T\n\n";
    std::cin >> n;              // 输入文法数量
    for (int i = 0; i < n; i++) // 输入文法
    {
        std::cin >> s;
        // 将 '-' 处置为字符串结束符 '\0'，使得左部字符串终止
        s[1] = 0;
        // s为左部，s+j+2为右部，back和id 为-1，-1默认值
        produtions.push_back(PRODUCTION(s, s + 3, -1, -1));
    }
    MAKE_ITEM();    // 构建项目表
    MAKE_FIRST();   // 生成FIRST集
    MAKE_FOLLOW();  // 生成FOLLOW集
    MAKE_CLOURSE(); // 构建项目集规范族
    GET_T();        // 构建符号集 T
    MAKE_GO();      // 构建转移表go
    make_table();   // 构造SLR分析表
    while (true)    // 执行LR分析过程
    {
        system("cls");
        show_productions(); // 显示当前产生式
        menu();
        int ch = 0;
        std::cin >> ch;
        if (ch == 1)
            show_first();
        else if (ch == 2)
            show_follow();
        else if (ch == 3)
            show_items();
        else if (ch == 4)
            show_closures();
        else if (ch == 5)
            show_slr_and_analyse();
        else
            return 0;
    }
    return 0;
}

/*
7
S->E
E->E+T
E->T
T->T*F
T->F
F->(E)
F->i
*/

/*
7
S->E
E->aA
E->bB
A->cA
A->d
B->cB
B->d
*/

/*
5
S->G
G->AG
G->b
A->GA
A->a
*/