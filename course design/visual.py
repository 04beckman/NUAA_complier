from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import tkinter as tk
from tkinter import ttk
import sys
import subprocess

stack = [0 for i in range(0, 8000)]  # 数据栈 前三个0是主函数的SL DL RA
code = []  # 存放代码

newbase = [0]  # 用于记录静态链的基址
setfile = False  # 标志是否加载文件
output_edit = None  # GUI 中用于显示输出的编辑框
font = QFont()
font.setPointSize(12)  # 设置字体大小为16


# 添加伪代码指令
def add_code(f, l, a):
    operation = dict()
    operation["F"] = f  # 指令类型
    operation["L"] = l  # 层次差
    operation["A"] = a  # 地址或常量
    code.append(operation)

    # 由当前的basic去找最近的直接外层，B是沿着静态链找的，因为要找最新的
    # 只要保证每一个静态链都是对的，就可以这么找到了，而递推计算，当level为0时，静态链就是自身，level为1时，会去找0……


# 获取静态链基址
def get_sl(B, level):
    res_B = B
    while level > 0:
        res_B = stack[res_B]
        level -= 1
    return res_B


# 解释器类
class Interpreter:
    def __init__(self):
        self.B = 0  # 基址寄存器
        self.T = 0  # 栈顶寄存器
        self.P = 0  # 下地址寄存器
        # 开始执行
        self.I = code[self.P]  # 当前指令
        self.P += 1
        global window

    def start(self):
        self.step()  # 显式调用 step() 方法开始执行

    def end(self):
        return self.P == 0

    #  根据当前B的值和level层差获取SL的值
    # 因为生成代码的时候就是给定层差，而每个层差的上一层就是定义它的，所以是可行的

    # 单步执行指令，解释器核心方法 step(self)，用来模拟运行 PL/0 编程语言的伪代码指令。
    # 每次调用 step 方法时，它会读取一条指令并根据指令类型执行相应的操作（类似虚拟机的指令执行过程）。
    def step(self):
        if (
            not self.end()
        ):  # 因为退出程序有一个 OPR 0 0 的，所以看到self.P要到0就说明退出了
            if self.I["F"] == "JMP":  # 直接跳转到对应指令
                self.P = self.I["A"]
            elif self.I["F"] == "JPC":
                if stack[self.T] == 0:  # 栈顶值为0才跳转
                    self.P = self.I["A"]
                self.T -= 1  # 无论是否跳转都要去除栈顶的值
            elif self.I["F"] == "INT":
                self.T += self.I["A"] - 1  # 开辟空间
            elif (
                self.I["F"] == "LOD"
            ):  # 变量加载指令，从指定层次差 L 和地址 A 加载变量到栈顶，获取局部变量或全局变量的值
                self.T += 1
                stack[self.T] = stack[
                    get_sl(self.B, self.I["L"]) + self.I["A"]
                ]  # 到了那层 找到真正基地址 再加addr
            elif (
                self.I["F"] == "STO"
            ):  # 变量存储指令，将栈顶值存储到指定层次差和地址的变量中，更新局部变量或全局变量的值
                if self.I["L"] == -1:  # 处理特殊地址（全局变量）
                    stack[self.T + self.I["A"]] = stack[self.T]
                    self.T -= 1
                else:
                    # print(get_sl(self.B, self.I['L']))
                    stack[get_sl(self.B, self.I["L"]) + self.I["A"]] = stack[self.T]
                    self.T -= 1
            elif self.I["F"] == "LIT":  # 加载常量到栈顶
                self.T += 1
                stack[self.T] = self.I["A"]
            elif (
                self.I["F"] == "CAL"
            ):  # 函数调用，调用函数并保存调用上下文（静态链、动态链、返回地址）
                self.T += 1
                stack[self.T] = get_sl(self.B, self.I["L"])  # 静态链
                stack[self.T + 1] = self.B  # 动态链
                stack[self.T + 2] = self.P  # 返回地址=当前的下一条地址
                self.B = self.T  # 新的基地址
                self.P = self.I["A"]  # 注意pro生成时填写的都是绝对地址code.len
                newbase.append(self.B)

            # 课设要求：假想机只能通过栈顶两个单元之间进行操作然后再放回栈顶
            elif self.I["F"] == "OPR":
                if self.I["A"] == 0:  # 函数返回
                    self.T = self.B - 1
                    self.P = stack[self.T + 3]  # 返回地址
                    self.B = stack[self.T + 2]
                    newbase.pop()
                elif self.I["A"] == 1:  # 取反操作
                    stack[self.T] = -stack[self.T]
                elif self.I["A"] == 2:  # 加法
                    self.T -= 1
                    stack[self.T] = stack[self.T] + stack[self.T + 1]
                elif self.I["A"] == 3:  # 减法
                    self.T -= 1
                    stack[self.T] = stack[self.T] - stack[self.T + 1]
                elif self.I["A"] == 4:  # 乘法
                    self.T -= 1
                    stack[self.T] = stack[self.T] * stack[self.T + 1]
                elif self.I["A"] == 5:  # 除法
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] // stack[self.T + 1])  # 整除
                elif self.I["A"] == 6:  # odd 奇偶
                    stack[self.T] = int(stack[self.T] % 2)
                elif self.I["A"] == 7:  # ==
                    self.T -= 1
                    stack[self.T] = int(
                        stack[self.T] == stack[self.T + 1]
                    )  # bool值转换为int
                elif self.I["A"] == 8:  # 不等于
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] != stack[self.T + 1])
                elif self.I["A"] == 9:  # <
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] < stack[self.T + 1])
                elif self.I["A"] == 10:  # >
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] > stack[self.T + 1])
                elif self.I["A"] == 11:  # >=
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] >= stack[self.T + 1])
                elif self.I["A"] == 12:  # <=
                    self.T -= 1
                    stack[self.T] = int(stack[self.T] <= stack[self.T + 1])
            elif self.I["F"] == "RED":  # 输入指令，提示输入
                self.T += 1
                # stack[self.T] = int(input("输入："))
                window.display_TEXT("")
                output_edit.setText("请输入数字")
                # print("输入数字！")
            elif self.I["F"] == "WRT":  # 输出指令，输出栈顶值
                # print("栈顶输出：", stack[self.T])
                window.output_STACK(str(stack[self.T]))
                self.T -= 1
            # print(self.P, stack[:T+8], T)
            self.I = code[self.P]  # 获取下一条指令
            # print(self.I)
            if (
                self.P == 0
            ):  # 因为退出程序有一个 OPR 0 0 的，所以看到self.P要到0就说明退出了
                return
            self.P += 1  # 默认self.P+1获取下一条指令 除非跳转


# GUI 界面
# 主窗口
class MainWindow(QMainWindow):
    def __init__(self):  # 创建一个 1200x960 的主窗口，并设置标题
        super().__init__()
        self.file_name = None
        self.output_file_name = "ouputerror.txt"
        self.setWindowTitle("PL/0 COMPLIER MADE BY NUAA")
        self.setFixedSize(1200, 960)

        # 设置图形场景，scene 是一个场景，用来管理绘制的图形（例如数据栈）；
        # view 是一个窗口，用来展示 scene 的内容
        self.scene = QGraphicsScene(self)
        self.view = QGraphicsView(self.scene, self)
        self.view.setGeometry(0, 0, 400, 960)
        self.interpreter = None

        # 运行按钮
        self.start_button = QPushButton("运行", self)
        self.start_button.move(230, 10)
        self.start_button.clicked.connect(self.begin)

        # 下一条指令按钮，逐步执行程序
        self.next_button = QPushButton("下一条", self)
        self.next_button.move(10, 50)
        self.next_button.clicked.connect(self.handle_next)

        # 对页面布局的代码。输出与输入框，text_edit显示源代码内容；output_edit显示程序输出内容
        self.text_edit = QTextEdit(self)
        self.text_edit.setFont(font)
        self.text_edit.setFixedSize(400, 450)  # 固定大小
        self.text_edit.move(400, 30)
        self.text_edit.setReadOnly(True)

        self.label3 = QLabel("源代码", self)
        self.label3.setFont(
            QFont("微软雅黑", 12, QFont.Bold)
        )  # 设置字体为微软雅黑，大小12，粗体
        self.label3.move(400, 0)
        self.label3.setFixedSize(400, 30)  # 固定大小
        self.label3.setWordWrap(True)
        self.label3.setStyleSheet("background-color: lightblue;")

        self.label2 = QLabel("问题", self)
        self.label2.move(400, 480)
        self.label2.setFont(
            QFont("微软雅黑", 12, QFont.Bold)
        )  # 设置字体为微软雅黑，大小12，粗体
        self.label2.setFixedSize(400, 30)  # 固定大小
        self.label2.setWordWrap(True)
        self.label2.setStyleSheet("background-color: lightblue;")

        self.output_edit = QTextEdit(self)
        self.output_edit.setFont(font)
        self.output_edit.setFixedSize(400, 450)  # 固定大小
        self.output_edit.move(400, 510)
        self.output_edit.setReadOnly(True)
        global output_edit
        output_edit = self.output_edit

        self.label4 = QLabel("目标代码", self)
        self.label4.move(800, 0)
        self.label4.setFont(
            QFont("微软雅黑", 12, QFont.Bold)
        )  # 设置字体为微软雅黑，大小12，粗体
        self.label4.setFixedSize(400, 30)  # 固定大小
        self.label4.setWordWrap(True)
        self.label4.setStyleSheet("background-color: lightblue;")

        self.code_edit = QTextEdit(self)
        self.code_edit.setFont(font)
        self.code_edit.setFixedSize(400, 450)  # 固定大小
        self.code_edit.move(800, 30)
        self.code_edit.setReadOnly(True)

        self.label5 = QLabel("词法单元", self)

        self.label5.setFixedSize(400, 30)  # 固定大小
        self.label5.move(800, 480)
        self.label5.setFont(
            QFont("微软雅黑", 12, QFont.Bold)
        )  # 设置字体为微软雅黑，大小12，粗体
        self.label5.setWordWrap(True)
        self.label5.setStyleSheet("background-color: lightblue;")

        self.word_edit = QTextEdit(self)
        self.word_edit.setFont(font)
        self.word_edit.setFixedSize(400, 450)  # 固定大小
        self.word_edit.move(800, 510)
        self.word_edit.setReadOnly(True)

        # 文件加载按钮， 打开 PL/0 程序文件
        self.open_button = QPushButton("打开文件", self)
        self.open_button.move(10, 10)
        self.open_button.clicked.connect(self.open_file)

        # 编译按钮，编译 PL/0 程序文件
        self.com_button = QPushButton("编译", self)
        self.com_button.move(120, 10)
        self.com_button.clicked.connect(self.complier)

        # 暂停按钮，暂停或继续程序的运行
        self.stop_button = QPushButton("暂停", self)
        self.stop_button.move(10, 150)
        self.stop_button.clicked.connect(self.handle_stop)

        # 动态显示用户提示信息，初始为空
        self.label = QLabel("", self)
        self.label.move(280, 10)

        # 用于显示数据栈栈顶的值，初始内容为 "栈顶输出："。支持动态更新和自动换行。
        self.label2 = QLabel("栈顶输出：", self)
        self.label2.setFont(QFont("Arial", 10))
        self.label2.move(10, 100)
        self.label2.resize(400, 60)
        self.label2.setWordWrap(True)

        # 输入框和输入按钮，用于用户输入数据
        self.data_edit = QLineEdit(self)
        self.data_edit.move(120, 50)

        self.input_button = QPushButton("输入", self)
        self.input_button.move(230, 50)
        self.input_button.clicked.connect(self.handle_input)

        self.send_time = QTimer(self)  # 定时器，用于控制程序的自动运行（逐条执行指令）

        self.info = ""  # 用于存储用户提示信息（例如提示用户输入数据），与标签 self.label 结合使用，动态更新提示内容
        self.output = ""  # 用于存储栈顶的输出结果，self.output 中的内容会被显示在标签 self.label2 上

    # 可视化数据栈的内容 和 当前指令的信息，并将其显示在程序的图形界面上，根据程序状态更新提示信息或输出结果
    def display(self):
        # 初始化画笔和字体
        pen = QPen(QColor(0, 100, 0))
        font = QFont("Arial", 10)
        y = 2000  # 初始化 y 坐标

        # 绘制数据栈的内容
        for i in range(self.interpreter.T + 1):  # 遍历从栈底到栈顶的所有栈元素
            self.scene.addRect(50, y, 40, 25, pen)  # 绘制一个矩形框表示栈单元
            # 绘制栈中内容
            text = self.scene.addText(str(stack[i]), font)  # 显示栈单元的值
            text.setDefaultTextColor(QColor(0, 0, 0))  # 设置文本颜色为黑色
            text.setPos(53, y)  # 设置文本在栈单元中的位置

            # 绘制栈索引
            # 每个栈单元的索引（从 0 开始）显示在矩形框的左侧。根据索引是单数字还是双数字，调整文本的位置以保持对齐。
            index = self.scene.addText(str(i), font)  # 显示栈单元的索引
            index.setDefaultTextColor(QColor(0, 0, 0))  # 设置索引的颜色为黑色
            if i < 10:
                index.setPos(25, y)  # 索引小于 10 时，位置偏左
            else:
                index.setPos(22, y)  # 索引为两位数时，调整位置使其居中

            # 标注静态链、动态链和返回地址
            if i in newbase:  # 如果当前索引是一个新的基址
                sl = self.scene.addText("静态链：", font)
                sl.setDefaultTextColor(QColor(0, 0, 0))
                sl.setPos(-50, y)  # 静态链标注位置
                dl = self.scene.addText("动态链：", font)
                dl.setDefaultTextColor(QColor(0, 0, 0))
                dl.setPos(-50, y - 40)  # 动态链标注位置
                ra = self.scene.addText("返回地址：", font)
                ra.setDefaultTextColor(QColor(0, 0, 0))
                ra.setPos(-63, y - 80)  # 返回地址标注位置
            y -= 40  # 每次绘制完一个栈单元，y 坐标向上移动 40 像素，每个栈单元占用的垂直空间为 40 像素（包括矩形框和标注的间隔）

        # 绘制指令信息
        # 这里-1是在最后会自动加一,
        # 显示当前即将执行的指令。包括指令的地址、操作符 F、层次差 L 和操作数/地址 A。
        if not self.interpreter.end():  # 如果程序尚未结束
            order = (
                "将要执行  "
                + str(self.interpreter.P - 1)
                + ": "
                + self.interpreter.I["F"]
                + " "
                + str(self.interpreter.I["L"])
                + " "
                + str(self.interpreter.I["A"])
            )
            text2 = self.scene.addText(order, font)  # 显示指令信息
            text2.setDefaultTextColor(QColor(0, 0, 0))
            text2.setPos(100, 2000)  # 设置文本位置
        else:
            output_edit.setText("程序运行结束")

        # 绘制提示
        if self.info != "":
            self.label.setText(self.info)  # 显示提示信息
            self.info = ""
        else:
            self.label.setText("")  # 如果没有提示信息，清空标签内容

        # 绘制栈顶输出
        # 如果程序有栈顶输出内容（存储在 self.output 中），更新到标签 label2。如果没有输出，则显示默认提示 "栈顶输出："
        if self.output != "":
            self.label2.setText("栈顶输出：" + self.output)

        else:
            self.label2.setText("栈顶输出：")

    # 文件选择和读取功能，用于打开.txt 文件，读取其中的内容，并将内容显示在文本框中。
    # 同时，它还会清除场景（scene）和输入框（data_edit）的内容，为后续的操作做好准备。
    def open_file(self):
        # 打开文件对话框，选择txt文件
        options = QFileDialog.Options()
        self.file_name, _ = QFileDialog.getOpenFileName(
            self, "打开文件", "", "Text Files (*.txt);;All Files (*)", options=options
        )

        # 检查 self.file_name 是否为空。如果用户取消了文件选择（未选择任何文件），self.file_name 为 None 或空字符串，此时不会执行文件读取逻辑
        if self.file_name:
            try:
                # 读取文件内容并显示在text_edit中
                with open(self.file_name, "r", encoding="utf-8") as file:
                    file_content = file.readlines()  # 读取整个文件内容

                    # 格式化文件内容并添加行号
                    formatted_content = ""  # 用于存储格式化后的文件内容。
                    for idx, line in enumerate(file_content):
                        formatted_content += f"{idx + 1:3}: {line}"  # 给每行添加行号
                    self.text_edit.setText(
                        formatted_content
                    )  # 将格式化后的内容显示在 text_edit 中
            except Exception as e:  # 如果读取文件出错
                self.text_edit.setText(f"无法打开文件: {e}")
        self.scene.clear()
        self.data_edit.clear()
        self.output_edit.clear()
        self.word_edit.clear()
        self.code_edit.clear()
        # print(file_name)

    def complier(self):
        code.clear()
        # print(self.file_name)

        # 启动外部 C++ 编译程序，并通过管道与 Python 交互
        process = subprocess.Popen(
            ["./main"],  # 执行名为 main 的 C++ 可执行文件
            stdin=subprocess.PIPE,  # 启用标准输入管道
            stdout=subprocess.PIPE,  # 启用标准输出管道
            stderr=subprocess.PIPE,  # 启用标准错误管道，用于捕获错误输出
        )

        # 将 self.file_name 作为输入传递给外部程序，进行编码后传输
        stdout, stderr = process.communicate(input=self.file_name.encode())

        # 读取编译输出文件，并将其内容（编译成功）显示在 GUI 控件 output_edit 中
        with open(self.output_file_name, "r") as file:
            file_content = file.read()
            self.output_edit.setText(file_content)

        # 读取名为 "code.txt" 的文件内容，并处理每一行
        origin = []
        with open("code.txt", "r") as f:
            origin = f.readlines()  # 读取所有行，保存到 origin 列表
        for i in range(len(origin)):
            origin[i] = origin[i][:-1]  # 删除行末的换行符
            x = origin[i].split()  # 按空格分割每行的内容
            add_code(x[0], int(x[1]), int(x[2]))

        # 创建解释器实例
        self.interpreter = Interpreter()

        # 如果 stderr 输出的内容为 "1"，表示编译失败或有错误
        if stderr.decode() == "1":
            self.begin()
            self.show_code()
            self.show_word()

    def display_TEXT(self, s):
        # 必须先输入才可以继续下一步
        self.next_button.setEnabled(False)  # 禁用“下一步”按钮
        self.info = s

    def output_STACK(self, s):  # 输出栈顶值
        self.output = self.output + s + " "
        # print(self.output)

    # 初始化栈和解释器的状态，并开始执行程序
    def begin(self):
        global newbase
        newbase.clear()  # 清空栈
        newbase.append(0)  # 将 0 添加到栈底
        self.output = ""
        self.interpreter.B = 0  # 基址寄存器
        self.interpreter.T = 0  # 栈顶寄存器
        self.interpreter.P = 0  # 下地址寄存器
        # 开始执行
        self.interpreter.I = code[self.interpreter.P]  # 获取当前指令
        self.interpreter.P += 1
        self.interpreter.step()  # 执行一步
        self.scene.clear()
        self.output_edit.clear()
        self.display()  # 更新显示

        self.timeIntervals = 1  # 设置时间间隔
        self.send_time.start(self.timeIntervals)  # 启动计时器
        # 给QTimer设定一个时间，每到达这个时间一次就会调用一次该方法
        self.send_time.timeout.connect(self.handle_next)

    # 下一步
    def handle_next(self):
        # print(self.interpreter.T)
        if not self.next_button.isEnabled():
            return
        if not self.interpreter.end():  # 如果程序没有结束
            self.data_edit.clear()
            self.interpreter.step()  # 执行下一步
            self.scene.clear()
            self.display()
        else:
            self.send_time.stop()  # 停止计时器
            self.start_button.setEnabled(True)  # 启用“开始”按钮

    # 处理用户输入的数据并更新栈
    def handle_input(self):
        data = self.data_edit.text()  # 获取用户输入的文本
        if data != "":  # 如果输入不为空
            stack[self.interpreter.T] = int(data)  # 将输入的数据存入栈
            self.scene.clear()
            self.display()
            self.data_edit.clear()
            self.next_button.setEnabled(True)  # 如果输入不为空

    # 处理“停止”按钮的事件，控制计时器的开始和停止
    def handle_stop(self):
        if self.send_time.isActive():
            self.send_time.stop()
        else:
            self.send_time.start()

    # 读取 code.txt 文件中的内容并在 GUI 中显示
    def show_code(self):
        with open("code.txt", "r") as file:
            file_content = file.readlines()
            formatted_content = ""
            for idx, line in enumerate(file_content):
                formatted_content += f"{idx:3}: {line}"  # 给每行添加行号
            self.code_edit.setText(
                formatted_content
            )  # 将格式化后的内容显示在 text_edit 中

    # 从 WordAnalyse.txt 文件中读取词法分析结果，并在 GUI 中显示，每行加上行号
    def show_word(self):
        with open("WordAnalyse.txt", "r", encoding="utf-8") as file:
            file_content = file.readlines()
            formatted_content = ""
            for idx, line in enumerate(file_content):
                formatted_content += f"{idx + 1:3}: {line}"
            self.word_edit.setText(
                formatted_content
            )  # 将格式化后的内容显示在 text_edit 中


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()  # 创建主窗口
    window.show()
    sys.exit(app.exec_())
