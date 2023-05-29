import struct
from sys import argv

if len(argv) < 3:
    print("Arguments: <input_txt_file> <output_bin_file> [float]")
    exit(1)

filename = argv[1]
binname = argv[2]

if len(argv) > 3 and argv[3] == "float":
    packstr = "<{}f"
else:
    packstr = "<{}d"

# 打开文件并读取数据
a = 0  # 记录文件的行数
b = None  # 记录文件的列数
with open(filename, "r") as f:
    with open(binname, "wb") as fb:
        fb.seek(8)
        for line in f:
            # 按逗号分隔并转换为浮点数类型
            line_list = [float(num) for num in line.split(",")]
            data_bytes = struct.pack(packstr.format(len(line_list)), *line_list)
            fb.write(data_bytes)

            # 更新行数和列数
            a += 1
            if b is None:
                b = len(line_list)
            else:
                assert b == len(line_list)
        fb.flush()
        fb.seek(0)
        fb.write(struct.pack("<ii", b, a))

