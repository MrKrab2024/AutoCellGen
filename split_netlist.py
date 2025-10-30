#!/usr/bin/env python3
"""
网表拆分脚本
将 ASAP7 网表中 nfin > 3 的晶体管拆分为多个 nfin <= 3 的晶体管
拆分策略：尽量使用 3，剩余部分按 3331 形式拆分
"""

import re
import sys
import os

def split_nfin(nfin):
    """
    将 nfin 拆分为 3331 形式，尽量使用 3，拆分数量最少
    
    Args:
        nfin (int): 原始 nfin 值
        
    Returns:
        list: 拆分后的 nfin 列表
    """
    if nfin <= 3:
        return [nfin]
    
    result = []
    remaining = nfin
    
    # 尽量使用 3
    while remaining >= 3:
        if remaining == 4:
            # 特殊情况：4 = 3 + 1
            result.append(3)
            result.append(1)
            remaining = 0
        elif remaining == 5:
            # 特殊情况：5 = 3 + 2
            result.append(3)
            result.append(2)
            remaining = 0
        elif remaining == 6:
            # 特殊情况：6 = 3 + 3
            result.append(3)
            result.append(3)
            remaining = 0
        else:
            result.append(3)
            remaining -= 3
    
    # 处理剩余部分
    if remaining > 0:
        result.append(remaining)
    
    return result

def parse_transistor_line(line):
    """
    解析晶体管行，提取各个参数
    
    Args:
        line (str): 晶体管行
        
    Returns:
        dict: 包含晶体管信息的字典
    """
    # 匹配晶体管行格式：MM0 net1 net2 net3 net4 nmos_rvt w=81.0n l=20n nfin=3
    # 支持 n 和 u 单位
    pattern = r'^(\s*)(\w+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+w=([0-9.]+[nu])\s+l=([0-9.]+[nu])\s+nfin=(\d+)(.*)$'
    match = re.match(pattern, line)
    
    if not match:
        return None
    
    return {
        'indent': match.group(1),
        'name': match.group(2),
        'drain': match.group(3),
        'gate': match.group(4),
        'source': match.group(5),
        'bulk': match.group(6),
        'type': match.group(7),
        'width': match.group(8),
        'length': match.group(9),
        'nfin': int(match.group(10)),
        'extra': match.group(11)
    }

def calculate_width(nfin, original_width, original_nfin):
    """
    根据 nfin 比例计算新的宽度
    
    Args:
        nfin (int): 新的 nfin 值
        original_width (str): 原始宽度字符串
        original_nfin (int): 原始 nfin 值
        
    Returns:
        str: 新的宽度字符串
    """
    # 提取数值部分
    width_value = float(original_width.replace('n', ''))
    
    # 按比例计算新宽度
    new_width = width_value * nfin / original_nfin
    
    return f"{new_width:.2f}n"

def generate_split_transistors(trans_info, split_nfins):
    """
    生成拆分后的晶体管行
    
    Args:
        trans_info (dict): 原始晶体管信息
        split_nfins (list): 拆分后的 nfin 列表
        
    Returns:
        list: 拆分后的晶体管行列表
    """
    result = []
    
    for i, nfin in enumerate(split_nfins):
        # 生成新的晶体管名称
        new_name = f"{trans_info['name']}_{i}"
        
        # 计算新的宽度
        new_width = calculate_width(nfin, trans_info['width'], trans_info['nfin'])
        
        # 生成新的晶体管行
        new_line = (f"{trans_info['indent']}{new_name} "
                   f"{trans_info['drain']} "
                   f"{trans_info['gate']} "
                   f"{trans_info['source']} "
                   f"{trans_info['bulk']} "
                   f"{trans_info['type']} "
                   f"w={new_width} "
                   f"l={trans_info['length']} "
                   f"nfin={nfin}{trans_info['extra']}")
        
        result.append(new_line)
    
    return result

def process_netlist(input_file, output_file):
    """
    处理网表文件，拆分 nfin > 3 的晶体管
    
    Args:
        input_file (str): 输入文件路径
        output_file (str): 输出文件路径
    """
    print(f"正在处理网表文件: {input_file}")
    
    with open(input_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    output_lines = []
    split_count = 0
    
    for line_num, line in enumerate(lines, 1):
        # 检查是否是晶体管行
        trans_info = parse_transistor_line(line.strip())
        
        if trans_info and trans_info['nfin'] > 3:
            print(f"第 {line_num} 行: 发现需要拆分的晶体管 {trans_info['name']} (nfin={trans_info['nfin']})")
            
            # 拆分 nfin
            split_nfins = split_nfin(trans_info['nfin'])
            print(f"  拆分方案: {trans_info['nfin']} -> {split_nfins}")
            
            # 生成拆分后的晶体管
            split_transistors = generate_split_transistors(trans_info, split_nfins)
            
            # 添加到输出，每个晶体管占一行
            for trans in split_transistors:
                output_lines.append(trans + '\n')
            split_count += 1
            
        else:
            # 保持原行不变
            output_lines.append(line)
    
    # 写入输出文件
    with open(output_file, 'w', encoding='utf-8') as f:
        f.writelines(output_lines)
    
    print(f"\n处理完成!")
    print(f"共拆分了 {split_count} 个晶体管")
    print(f"输出文件: {output_file}")

def main():
    """主函数"""
    if len(sys.argv) != 3:
        print("用法: python split_netlist.py <输入文件> <输出文件>")
        print("示例: python split_netlist.py DATA/input/asap7sc7p5t.sp DATA/input/asap7sc7p5t_split.sp")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    if not os.path.exists(input_file):
        print(f"错误: 输入文件不存在: {input_file}")
        sys.exit(1)
    
    try:
        process_netlist(input_file, output_file)
    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
