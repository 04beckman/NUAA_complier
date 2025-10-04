import random
from copy import deepcopy


# edges是一个矩阵
# 对应索引存放[[end1,value1],[end2,value2]]
class Graph:
    node_num = 0
    edges = []

    def __init__(self, node_num):
        self.node_num = node_num
        for _ in range(node_num):
            self.edges.append([])

    def get_edge(
        self, start, end
    ):  # 获取终结符 获取从 start 到 end 的边的标签（如果存在的话）。
        for edge in self.edges[start]:
            if edge[0] == end:
                return edge[1]
        return -1  # no edge between start and end

    def get_end(
        self, start, val
    ):  # 获取从 start 状态出发，经过 val 标签的边所到达的所有状态。
        ans = []
        for edge in self.edges[start]:
            if edge[1] is val:
                ans.append(edge[0])  # 获取下一状态
        return ans

    def add_edge(
        self, start, end, value
    ):  # 向图中添加一条从 start 到 end 的边，标签为 value。
        self.edges[start].append([end, value])


def get_key(
    dic, val
):  # 用来查找字典 dic 中所有值为 val 的键（即找到与某个状态集对应的所有状态）。
    return [k for k, v in dic.items() if v == val]


def move(
    graph, T, val
):  # 给定状态集 T，该函数返回所有从 T 中的状态出发，经过 val 转换后到达的状态集合。
    ans = []
    for node in T:
        ans += graph.get_end(node, val)
    return ans


def eps_cover(_graph, T):  # 计算状态集 T 的ε-闭包。
    ans = []
    for node in T:
        ans.append(node)
    for node in T:
        next_eps = graph.get_end(node, "e")  # 以ε为输入字符获取下一个状态
        for nodee in next_eps:
            if nodee not in ans:
                ans.append(nodee)
        #!!
        for nodeee in eps_cover(graph, next_eps):
            if nodeee not in ans:
                ans.append(nodeee)
    ans.sort()
    return ans


# dfa是字典
def minimize(initial, final, dfa):
    allstates = initial + final

    def get_source_set(target_set, char):  # 返回能经过char到达target_set的状态
        source_set = set()
        for state in allstates:
            try:
                if dfa[state][char] in target_set:
                    source_set.update(str(state))
            except KeyError:
                pass
        return source_set

    P = [set(final), set(initial)]  # 每个阶段划分好的等价类集合
    Wait = [
        set(final),
        set(initial),
    ]  # 目的等价类(判断哪些状态可以经过char到达该等价类)
    while Wait:

        A = random.choice(Wait)
        Wait.remove(A)
        for char in ["a", "b"]:
            X = get_source_set(A, char)  # 求哪些状态可以经过char到达A
            X = set([int(i) for i in X])
            P_temp = []

            for Y in P:
                # 拆分等价类Y
                q1 = X & Y  # 属于等价类Y的状态
                q2 = Y - X  # 不属于等价类Y的状态

                if len(q1) and len(
                    q2
                ):  # X和Y相交 且 X != Y，也就是说存在不属于等价类Y的状态(X&Y)
                    P_temp.append(q1)
                    P_temp.append(q2)

                    if Y in Wait:  # 如果Y还未被划分
                        Wait.remove(Y)
                    if len(q1) <= len(q2):
                        Wait.append(q1)
                    else:
                        Wait.append(q2)
                else:  # X和Y不相交 或 X = Y 或 X|Y = 空
                    P_temp.append(Y)  # 直接将Y加入新划分的等价类
            P = deepcopy(P_temp)
    return P


if __name__ == "__main__":  # start convert
    #
    graph = Graph(8)
    graph.add_edge(0, 5, "e")
    graph.add_edge(5, 5, "a")
    graph.add_edge(5, 5, "b")
    graph.add_edge(5, 1, "e")
    graph.add_edge(1, 3, "a")
    graph.add_edge(1, 4, "b")
    graph.add_edge(3, 2, "a")
    graph.add_edge(4, 2, "b")
    graph.add_edge(2, 6, "e")
    graph.add_edge(6, 6, "a")
    graph.add_edge(6, 6, "b")
    graph.add_edge(6, 7, "e")
    #
    new_set = []
    state_name = {}
    wait = []
    wait.append(eps_cover(graph, [0]))  # 初始状态为0，求闭包
    name = 0
    while wait:
        curr_state = wait[0]  # current processing state
        # 从状态集curr_state经过“a”能到达的状态集
        a = eps_cover(graph, move(graph, curr_state, "a"))  # Ia
        b = eps_cover(graph, move(graph, curr_state, "b"))  # Ib

        new_set.append([curr_state, a, b])  # 新增确定化状态
        state_name[name] = curr_state  # 重新为其标号
        name += 1

        flag_a = 0
        flag_b = 0

        for elt in new_set:  # if a or b haven't been processed, need to be processed
            if elt[0] == a:  # 如果集合a已经存在于新增确定化状态
                flag_a = 1
            if elt[0] == b:  # 如果集合b已经存在于新增确定化状态
                flag_b = 1
            if flag_a and flag_b:
                break
        if (
            not flag_a and a not in wait
        ):  # 如果集合a不存在新增确定化状态 且不存在于wait中
            wait.append(a)
        if (
            not flag_b and b not in wait
        ):  # 如果集合b不存在新增确定化状态 且不存在于wait中
            wait.append(b)

        wait.pop(0)

    name = 0
    dfa = {}
    print("--------------确定化--------------")
    for row in new_set:
        print(str(name) + ": " + str(row[0]))
        dfa[name] = []
        name += 1

    print("\n")

    initial = []  # 终态的集合
    final = []  # 初态的集合
    for row in new_set:
        I = get_key(state_name, row[0])
        Ia = get_key(state_name, row[1])
        Ib = get_key(state_name, row[2])
        tmp = {"a": Ia[0], "b": Ib[0]}
        dfa[I[0]] = tmp
        if name - 1 in row[0]:
            final.append(I[0])
        else:
            initial.append(I[0])

        print(str(I) + ": " + str(Ia) + ", " + str(Ib))  # 获取每个确定化状态的标号
    # print(initial, final)
    dfam = minimize(initial, final, dfa)
    print("\n")
    print("--------------最小化--------------")
    print(dfam)
