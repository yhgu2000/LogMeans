import struct
from sys import argv

if len(argv) != 3:
    print("Arguments: <input_txt_file> <output_bin_file>")
    exit(1)

a = 0  # 记录文件的行数
b = 0  # 记录文件的列数
data_list = []  # 保存文件数据的一维数组
filename = argv[1]
binname = argv[2]

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

# 将行数、列数和一维数组转换为二进制数据
header = struct.pack("<ii", b, a)  # 将行数和列数打包为8字节的二进制数据
data_bytes = struct.pack("<{}d".format(a * b), *data_list)  # 将一维数组打包为二进制数据

# 将二进制数据写入文件
with open(binname, "wb") as f:
    f.write(header)
    f.write(data_bytes)
