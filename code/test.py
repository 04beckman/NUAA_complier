import subprocess

# 获取用户输入
num = input()

# 调用 C++ 程序，传递输入数据并获取结果
process = subprocess.Popen(
    ["./main"],  # C++ 可执行文件
    stdin=subprocess.PIPE,  # 启用标准输入管道
    stdout=subprocess.PIPE,  # 启用标准输出管道
    stderr=subprocess.PIPE,
)
# 将用户输入传给 C++ 程序
stdout, stderr = process.communicate(input=num.encode())
