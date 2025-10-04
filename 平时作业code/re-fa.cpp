#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXN 1000
#define Visible_letter_count (Visible_letter_max - Visible_letter_min + 1) // 算空格, 从32到126共94个可见字符.
#define Visible_letter_min 32
#define Visible_letter_max 126

const char kong = ' '; // 空格代表空串

//-a->to
typedef struct NODE
{
    int to; // 1
    char a; // a
} NODE;
typedef struct FA // 邻接表存储FA
{
    int n, BEGIN;                        // n为节点个数（包括空节点），BEGIN为初态
    int ENDS[MAXN / 100], len_ENDS;      // 终态集
    NODE data[MAXN][MAXN];               // 邻接表
    int len[MAXN];                       // data每一行的长度
    char alphabet[Visible_letter_count]; // 字母表
    int len_alph;                        // 字母表的长度
} FA;
typedef struct BE // 正则表达式转NFA时，用于记录一个NFA的初态和终态
{
    int BEGIN;
    int END;
} BE;

//------------------------------------------------------------------------------1//
int IsEND(FA *fa, int a) // 判断一个节点是否为终态
{
    for (int i = 0; i < fa->len_ENDS; i++)
    {
        if (a == fa->ENDS[i])
            return 1; // 如果集合中包含之前NFA中的终态，那么该集合就是DFA中的终态
    }
    return 0;
}
int insert(FA *fa, int from, int to, char a) // 向fa的邻接表中插入一条边
{
    int i;
    // 先判断是否为新节点
    if (from >= fa->n)
    {
        if (fa->n >= MAXN)
            return 1;         // 节点个数是否超出最大值
        fa->len[fa->n++] = 0; // 每行长度为0
    }
    if (to >= fa->n)
    {
        if (fa->n >= MAXN)
            return 1;
        fa->len[fa->n++] = 0;
    }

    // 判断边是否已经存在
    for (i = 0; i < fa->len[from]; i++)
        if ((fa->data[from][i]).a == a && (fa->data[from][i]).to == to)
            return 0;

    if (fa->len[from] >= MAXN)
        return 1; // 邻接表的长度是否超过最大值

    // 插入
    (fa->data[from][fa->len[from]]).a = a;
    (fa->data[from][fa->len[from]++]).to = to;
    return 0;
}
void initAlphabet(FA *fa) // 初始化字母表
{
    fa->len_alph = 0;
    int temp[Visible_letter_count] = {0}; // 哈希表
    for (int i = 0; i < fa->n; i++)
    {
        for (int j = 0; j < fa->len[i]; j++)
        {
            int a = fa->data[i][j].a;
            if (a != ' ')
                temp[a - Visible_letter_min] = 1;
        }
    }
    for (int i = 0; i < Visible_letter_count; i++)
        if (temp[i])
            fa->alphabet[fa->len_alph++] = i + Visible_letter_min;
}
int to(FA *fa, int from, char a) // 计算from节点输入a时的下一个节点
{
    for (int i = 0; i < fa->len[from]; i++)
        if (fa->data[from][i].a == a)
            return fa->data[from][i].to;
    return -1;
}
int AndFa(FA *fa, BE *stack_FA, int *len_stack_FA) // 正则表达式转NFA时的连接操作
{
    if (*len_stack_FA < 2)
        return 1;
    int b = stack_FA[--*len_stack_FA].BEGIN;
    int e = stack_FA[*len_stack_FA].END;
    int b1 = stack_FA[--*len_stack_FA].BEGIN;
    int e1 = stack_FA[*len_stack_FA].END;
    if (insert(fa, e1, b, kong))
        return 1;
    // 注意栈是先进后出的，所以b1是begin e是end
    stack_FA[*len_stack_FA].BEGIN = b1;
    stack_FA[(*len_stack_FA)++].END = e;
    return 0;
}
int OrFa(FA *fa, BE *stack_FA, int *len_stack_FA) // 正则表达式转NFA时的|操作
{
    if (*len_stack_FA < 2)
        return 1;
    int b = stack_FA[--*len_stack_FA].BEGIN;
    int e = stack_FA[*len_stack_FA].END;
    int b1 = stack_FA[--*len_stack_FA].BEGIN;
    int e1 = stack_FA[*len_stack_FA].END;
    if (insert(fa, b1, b, kong))
        return 1;
    if (insert(fa, e1, e, kong))
        return 1;
    // 注意栈是先进后出的，所以b1是begin e是end
    stack_FA[*len_stack_FA].BEGIN = b1;
    stack_FA[(*len_stack_FA)++].END = e;
    return 0;
}

// 将正则表达式转换成NFA
int ReToNFA(char *re, FA *nfa) // 此处用&代表连接
{
    nfa->len_ENDS = 0; // 初始化终态集的个数为0个
    if (*re != '\0')   // 如果re的长度大于0
    {
        // len_stack_op：记录栈顶指针的位置   len_stack_FA：记录栈中元素个数
        int len_stack_op = 1, len_stack_FA = 0;
        // 用于分析操作符的栈 stack_op   temp用来辅助分析&操作
        char stack_op[MAXN] = {'('}, temp;
        // 用于分析状态
        BE stack_FA[MAXN];
        nfa->n = 0;
        strcat(re, ")");    // 在re的开始和结束添加括号，方便计算
        while (*re != '\0') //    (a|b)(c|d)*     (  )   |  *  &  ab
        {
            switch (*re)
            {
            case '(':
                if (len_stack_op >= MAXN)
                    return 1; // 栈是否溢出
                stack_op[len_stack_op++] = '(';
                break;
            case ')':
                while (1) // 先做前面的收尾工作
                {
                    if (len_stack_op < 1)
                        return 1; // 如果栈空，代表正则表达式括号不匹配
                    if (stack_op[--len_stack_op] == '(')
                        break;

                    // 此处要先判断&，再判断|，因为当&和|连续出现时，&靠近栈顶 ()&()|()
                    if (stack_op[len_stack_op] == '&')
                    {
                        if (AndFa(nfa, stack_FA, &len_stack_FA))
                            return 1;
                    }
                    else if (stack_op[len_stack_op] == '|')
                    {
                        if (OrFa(nfa, stack_FA, &len_stack_FA))
                            return 1;
                    }
                }
                // 如果下一个输入时左括号或字符，则添加一个&
                temp = *(re + 1);
                if (temp != 0 && temp != ')' && temp != '*' && temp != '|')
                {
                    if (len_stack_op > 0 && stack_op[len_stack_op - 1] == '&') //()&()b  (
                    {
                        len_stack_op--;
                        if (AndFa(nfa, stack_FA, &len_stack_FA))
                            return 1;
                    }
                    if (len_stack_op >= MAXN)
                        return 1;
                    stack_op[len_stack_op++] = '&'; //()b
                }
                break;
            case '*': // 闭包
                if (len_stack_FA < 1)
                    return 1; //*前必有前面形成的NFA
                // 如果是终结符
                if (insert(nfa, stack_FA[len_stack_FA - 1].END, stack_FA[len_stack_FA - 1].BEGIN, kong))
                    return 1;
                // 不是终结符
                temp = *(re + 1);
                if (temp != 0 && temp != ')' && temp != '*' && temp != '|')
                {
                    if (len_stack_op > 0 && stack_op[len_stack_op - 1] == '&') // 如 // a&b*b
                    {
                        len_stack_op--;
                        if (AndFa(nfa, stack_FA, &len_stack_FA))
                            return 1;
                    }
                    if (len_stack_op >= MAXN)
                        return 1;
                    stack_op[len_stack_op++] = '&'; // 如 a*b
                }
                break;
            case '|':                                                      // 优先级与大于或，且与和或都是双目运算符 如 a&b|c
                if (len_stack_op > 0 && stack_op[len_stack_op - 1] == '&') // 前面有&则先做与运算
                {
                    len_stack_op--;
                    if (AndFa(nfa, stack_FA, &len_stack_FA))
                        return 1;
                }
                if (len_stack_op > 0 && stack_op[len_stack_op - 1] == '|') // 如果前面有|先做前面的或
                {
                    len_stack_op--;
                    if (OrFa(nfa, stack_FA, &len_stack_FA))
                        return 1;
                }
                if (len_stack_op >= MAXN)
                    return 1;
                stack_op[len_stack_op++] = '|'; // 将此次的或进栈  a|
                break;
            default:
                if (*re < Visible_letter_min || *re > Visible_letter_max)
                    return 1; // 排除一些字符

                // 插入边（字母）
                if (insert(nfa, nfa->n, nfa->n + 1, *re))
                    return 1;
                // 更新状态
                if (len_stack_FA >= MAXN)
                    return 1;
                stack_FA[len_stack_FA].BEGIN = nfa->n - 2;
                stack_FA[len_stack_FA++].END = nfa->n - 1;
                // 预判下一个字符是不是边（字母）
                temp = *(re + 1);
                if (temp != 0 && temp != ')' && temp != '*' && temp != '|')
                {
                    if (len_stack_op > 0 && stack_op[len_stack_op - 1] == '&') // 如果前面包含一个& 如 a&bb
                    {
                        len_stack_op--;
                        if (AndFa(nfa, stack_FA, &len_stack_FA))
                            return 1;
                    }
                    if (len_stack_op >= MAXN)
                        return 1;
                    stack_op[len_stack_op++] = '&'; // 否则将与操作入栈 ab
                }
            }
            re++;
        }
        if (len_stack_op > 0 || len_stack_FA > 1)
            return 1;
        nfa->BEGIN = stack_FA[0].BEGIN;               // 初态
        nfa->ENDS[nfa->len_ENDS++] = stack_FA[0].END; // 在此处只有一个终态
        initAlphabet(nfa);
    }
    else
    {
        // 为了满足输入为空的特殊情况
        nfa->n = 2;
        nfa->BEGIN = nfa->len[0] = nfa->len[1] = 0;
        nfa->ENDS[nfa->len_ENDS++] = 1;
    }
    return 0;
}

typedef struct SET // 集合
{
    int n; // 记录当前t中的元素个数
    int t[MAXN];
} SET;

int set_insert(SET *set, int a) // 集合插入
{
    int i;
    for (i = 0; i < set->n; i++)
        if ((set->t)[i] == a)
            return 1;
    if (set->n >= MAXN)
        return -1;
    (set->t)[set->n++] = a;
    return 0;
}
int set_delete(SET *set, int a) // 删除集合的一个元素
{
    int i;
    for (i = 0; i < set->n; i++)
        if (set->t[i] == a)
            break;
    if (i == set->n)
        return -1;
    for (int j = i + 1; j < set->n; j++)
        set->t[j - 1] = set->t[j];
    set->n--;
    return 0;
}

// 查找集合sets[sets_n]与前面的那个集合一样，如果没有一样的，则将sets[sets_n]加入sets
// 集合内部是有序的
int if_same(SET *sets, int *sets_n)
{
    int i, j, len = sets[*sets_n].n;
    for (i = 0; i < *sets_n; i++)
    {
        if (sets[i].n != len)
            continue;
        for (j = 0; j < len; j++)
            if (sets[i].t[j] != sets[*sets_n].t[j])
                break;

        if (j == len)
        {
            return i;
        }
    }
    return (*sets_n)++;
}
int cmp(const void *_a, const void *_b) // 用于qsort
{
    return *((int *)_a) - *((int *)_b);
}

/*-----------------------------------------------------(2)将一个 NFA 转换为 DFA */
SET sets[MAXN]; // 在NFAToDFA存状态集，在DFA化简中存状态集
int len_sets = 0;
int NFAToDFA(FA *nfa, FA *dfa)
{
    dfa->n = 0;
    dfa->len_ENDS = 0;
    int i, j, k, m;

    // 求ε-clouser(0)
    len_sets = 1; // 记录当前集合内的个数
    sets[0].n = 0;
    set_insert(sets, 0);
    for (i = 0; i < sets[0].n; i++)
    {
        NODE *temp = (nfa->data)[sets[0].t[i]];
        int temp_len = (nfa->len)[sets[0].t[i]]; // 每一行的最大长度
        for (j = 0; j < temp_len; j++)
            if (temp[j].a == kong)
                set_insert(sets, temp[j].to); // 如果连接的边为空就把状态加入
    }
    // 将集合排序，方便判断两个集合是否一样
    qsort(sets[0].t, sets[0].n, sizeof(int), cmp);

    for (i = 0; i < len_sets; i++) // 遍历状态集
    {
        for (j = 0; j < nfa->len_alph; j++) // 遍历边
        {
            sets[len_sets].n = 0;
            // 求move(sets[i],alphabet[j])
            for (k = 0; k < sets[i].n; k++) // 遍历每个状态集中的元素
            {
                for (m = 0; m < (nfa->len)[sets[i].t[k]]; m++) // 查看每个元素的邻接边
                    if ((nfa->data)[sets[i].t[k]][m].a == nfa->alphabet[j])
                        set_insert(sets + len_sets, (nfa->data)[sets[i].t[k]][m].to);
            }
            // 求ε-clouser(move(sets[i],alphabet[j]))
            for (m = 0; m < sets[len_sets].n; m++) // 遍历每个状态集中的元素
            {
                NODE *temp = (nfa->data)[sets[len_sets].t[m]];
                int temp_len = (nfa->len)[sets[len_sets].t[m]];
                for (k = 0; k < temp_len; k++) // 将通过空能到达的元素插入
                    if (temp[k].a == kong)
                        set_insert(sets + len_sets, temp[k].to);
            }
            qsort(sets[len_sets].t, sets[len_sets].n, sizeof(int), cmp);

            // 如果集合不为空，且是新集合，就在dfa中加入对应的边
            if (sets[len_sets].n)
            {
                k = if_same(sets, &len_sets);
                insert(dfa, i, k, nfa->alphabet[j]); // 形成DFA新的边
            }
        }
    }
    dfa->BEGIN = 0;
    // 求终态
    for (i = 0; i < len_sets; i++) // 每个状态集
    {
        for (j = 0; j < sets[i].n; j++) // 每个状态集的全部元素
        {
            if (IsEND(nfa, sets[i].t[j])) // 如果包含NFA的终态，则该状态就是终态
            {
                dfa->ENDS[dfa->len_ENDS++] = i; // 加入终态集
                break;
            }
        }
    }

    // 复制字母表
    dfa->len_alph = nfa->len_alph;
    for (i = 0; i < nfa->len_alph; i++)
        dfa->alphabet[i] = nfa->alphabet[i];
    return 0;
}

// 找到元素a所在的集合
int FindInSets(SET *Sets, int set_n, int a)
{
    if (a < 0)
        return -1;
    for (int i = 0; i < set_n; i++)
    {
        for (int j = 0; j < Sets[i].n; j++)
            if (Sets[i].t[j] == a)
                return i;
    }
    return -1;
}

/*-----------------------------------------------------（3）对 DFA 进行最小化*/
int type[MAXN], len_type; // 在SimplifyDFA中，存每个节点经过映射后所在的集合
void SimplifyDFA(FA *dfa)
{
    initAlphabet(dfa); // 初始化字母表

    // 终态和非终态放在两个集合   sets[0]初态    sets[1]终态
    len_sets = 2;
    sets[0].n = sets[1].n = 0;
    for (int i = 0; i < dfa->n; i++) // 遍历dfa的所有状态
        if (IsEND(dfa, i))
            set_insert(sets + 1, i);
        else
            set_insert(sets, i);

    int flag = 1;
    while (flag)
    {
        flag = 0;
        for (int i = 0; i < len_sets; i++) // 遍历DFA每个状态
        {
            len_type = sets[i].n; // 获取集合中的元素个数
            if (len_type < 2)
                continue; // 只有一个元素的集合不用再计算

            for (int j = 0; j < dfa->len_alph; j++) // 遍历每条边
            {
                sets[len_sets].n = 0;
                // 求type
                for (int k = 0; k < len_type; k++) // 遍历sets集合中的每个元素
                {
                    int t = to(dfa, sets[i].t[k], dfa->alphabet[j]); // 获取每个集合中元素输入不同的边转向的元素
                    // 如 type[k]=0; 表示集合sets[i]的第k个元素，在划分后的第0个集合，也就是初态集
                    type[k] = FindInSets(sets, len_sets, t); // 表示集合sets[i]的第k个元素，输入dfa->alphabet[j]后转向状态t所在的集合
                }

                // 对集合进行划分
                // 以type[0]为基础，若type[k]和type[0]不一样，则将所有type[m]=type[k]的m放在一个新的集合
                // type[k]=-1代表没有映射，-2代表已经被移动过
                for (int k = 1; k < len_type; k++) // 遍历集合中的每个元素
                    if (type[k] != -2 && type[k] != type[0])
                    {
                        flag = 1; // 运行到此，则sets有所更新，需要重新循环

                        for (int m = k + 1; m < len_type; m++)
                            if (type[m] == type[k]) // 将所有type[m]=type[k]的m放在一个新的集合
                            {
                                type[m] = -2;
                                set_delete(sets + i, m);        // 将m从原来集合中删除
                                set_insert(sets + len_sets, m); // 将m放入新集合
                            }
                        // k和m是一类要放在一起
                        type[k] = -2;                   // 操作k元素
                        set_delete(sets + i, k);        // 将k从原来集合中删除
                        set_insert(sets + len_sets, k); // 将k放入新集合
                        len_sets++;
                    }
            }
        }
    }

    // 在dfa中，将在同一个集合的状态合并
    for (int i = 0; i < len_sets; i++) // 遍历所有状态集（相对于划分后的状态集）
    {
        int temp0 = sets[i].t[0];
        for (int j = 1; j < sets[i].n; j++) // 遍历所有元素
        {
            int tempj = sets[i].t[j];

            // 合并tempj发出的边
            for (int k = 0; k < dfa->len[tempj]; k++)
                insert(dfa, temp0, dfa->data[tempj][k].to, dfa->data[tempj][k].a);

            // 合并发向tempj的边
            for (int k = 0; k < dfa->n; k++)
                for (int m = 0; m < dfa->len[k]; m++)
                    if (dfa->data[k][m].to == tempj)
                        dfa->data[k][m].to = temp0;

            // 将data[tempj]的长度置为-1，代表tempj被弃用
            dfa->len[tempj] = -1;

            // 如果tempj是终态，则在终态集中删除
            if (IsEND(dfa, tempj))
            {
                for (int k = tempj + 1; k < dfa->len_ENDS; k++)
                    dfa->ENDS[k - 1] = dfa->ENDS[k];
                dfa->len_ENDS--;
            }
        }
    }
}

/*-----------------------------------------------------(4)利用 DFA 判断输入的字符串是否为符合词法规则 */
int RunDFA(FA *dfa, char *s) // 判断s是否被dfa接受
{
    int state = dfa->BEGIN;
    while (*s != '\0')
    {
        state = to(dfa, state, *s);
        if (state < 1)
            return 1;
        s++;
    }
    if (IsEND(dfa, state))
        return 0;
    else
        return 1;
}

void PrintFA(FA *fa) // 打印FA
{
    printf("BEGIN:%d, END:%d", fa->BEGIN, fa->ENDS[0]);
    for (int i = 1; i < fa->len_ENDS; i++)
        printf(", %d", fa->ENDS[i]);
    printf("\n");
    for (int i = 0; i < fa->n; i++)
    {
        if (fa->len[i] < 0)
            continue;
        printf("%d: ", i);
        for (int j = 0; j < fa->len[i]; j++)
        {
            printf("(value:%c, to:%d) ", (fa->data[i][j]).a, (fa->data[i][j]).to);
        }
        printf("\n");
    }
    printf("\n");
}

FA NFA, DFA; // FA所占内存较大，只能为全局变量
void test()
{
    FA *nfa = &NFA, *dfa = &DFA;
    // re记录输入的正则表达式
    char re[100], s[100];

    while (1)
    {
        printf("请输入一个正则表达式(符号有(,),*,|,空格,空格代表空串),");
        printf("输入Crtl+Z退出正则表达式测试\n");
        if (!gets(re))
            break;
        if (ReToNFA(re, nfa))
        {
            printf("error\n\n");
            continue;
        }
        //        PrintFA(nfa);
        NFAToDFA(nfa, dfa);
        printf("\nNFA:\n");
        PrintFA(nfa);
        printf("DFA:\n");
        PrintFA(dfa);
        SimplifyDFA(dfa);
        printf("化简的DFA:\n");
        PrintFA(dfa);
        printf("\n");
        while (1)
        {
            printf("输入一个字符串,");
            printf("输入Crtl+Z退出DFA测试\n");
            if (!gets(s))
                break;
            // 将字符串放到dfa中运行一遍看能不能走到终态
            if (RunDFA(dfa, s))
                printf("Input does not match!\n"); // 不能走到终态，不匹配
            else
                printf("Input match!\n"); // 能走到终态，匹配
            printf("\n");
        }
    }
}

int main(void)
{
    test();
    return 0;
}
/*
(1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*
*/