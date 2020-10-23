#!coding=utf-8
import sys
import os
import re
# 函数


def addSuffix_filename(file_path, addSuffix):  # file_path为文件夹路径；addSuffix为后缀
    for root, dirs, files in os.walk(file_path):  # 获取文档内所有文件
        for file_name in files:  # 取出文件夹下各文件名
            if file_name.endswith('.sol'):  # 选出要修改的文件类型；
                os.rename(os.path.join(root, file_name), os.path.join(
                    root, file_name.replace('.sol', addSuffix+'.hpp')))  # 此处是把后缀部分进行替换
                print('new file name is {0}'.format(
                    file_name.replace('.txt', addSuffix+'.txt')))  # 输出添加后缀后的名字

# addSuffix_filename(r'.', '')


# (?P<myname>exp)
# 1
# 后向引用的方式：

# (?P=myname)
def bupdateFile(file, old_str, new_str):
    r = r'function\s(?P<func>.*?\((\n\s*.*?[,|\n])*?\))\s.*?returns\s*\((?P<ret>.*?)\);'
    rr = "virtual ?P=func ?P=ret = 0;"
    p = re.compile(r)
    with open(file, "r", encoding="utf-8") as f1:
        line = f1.read()
        # for line in f1:
        ret = re.findall(p, line)
        if len(ret)>0:
            print(re.findall(p, line))
            print(re.sub(r, rr, line))
        # f2.write(re.sub(old_str,new_str,line))
    # os.remove(file)
    # os.rename("%s.bak" % file, file)


def batchupdate_filecontent(file_path, addSuffix):  # file_path为文件夹路径；addSuffix为后缀
    for root, dirs, files in os.walk(file_path):  # 获取文档内所有文件
        for file_name in files:  # 取出文件夹下各文件名
            if file_name.endswith('.hpp'):  # 选出要修改的文件类型；
                # os.rename(os.path.join(root, file_name), os.path.join(root,file_name.replace('.sol',addSuffix+'.hpp'))) #此处是把后缀部分进行替换
                print('new file name is {0}'.format( file_name)) #输出添加后缀后的名字
                bupdateFile(os.path.join(root, file_name), "", "")


batchupdate_filecontent(r'.', '')


def updateFile(file, old_str, new_str):
    with open(file, "r", encoding="utf-8") as f1, open("%s.bak" % file, "w", encoding="utf-8") as f2:
        for line in f1:
            f2.write(re.sub(old_str, new_str, line))
    os.remove(file)
    os.rename("%s.bak" % file, file)

# updateFile(r"D:\zdz\myfile.txt", "zdz", "daziran")#将"D:\zdz\"路径的myfile.txt文件把所有的zdz改为daziran
