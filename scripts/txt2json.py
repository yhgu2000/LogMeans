import json
from sys import argv


if len(argv) != 3:
    print("Arguments: <input_txt_file> <output_json_file>")
    exit(1)

a = 0  # 记录文件的行数
b = 0  # 记录文件的列数
data_list = []  # 保存文件数据的一维数组
filename = argv[1]
jsonname = argv[2]

# 打开文件并读取数据
with open(filename, "r") as f:
    for line in f:
        # 按逗号分隔并转换为浮点数类型
        line_list = [float(num) for num in line.split(",")]
        data_list += line_list

        # 更新行数和列数
        a += 1
        if b == 0:
            b = len(line_list)

# 将数据保存到字典中
data_dict = {"dataset": {"rows": b, "cols": a, "data": data_list}}

# 将字典转换为JSON字符串
data_json = json.dumps(data_dict)

# 将JSON字符串写入文件
with open(jsonname, "w") as f:
    f.write(data_json)
