import subprocess
import re

MJ8x8_boards = {
    '0': 'MJ???_', # 0 Alpha 
    '1': 'MJ???_', # 0 Bravo
    '2': 'MJ???_', # 0 Charlie
    '3': 'MJ828_', # 0 Delta
    '4': 'MJ838_', # 1 Alpha
    '5': 'MJ???_', # 1 Bravo
    '6': 'MJ???_', # 1 Charlie
    '7': 'MJ???_', # 1 Delta
    '8': 'MJ808_', # 2 Alpha
    '9': 'MJ818_', # 2 Bravo
    'a': 'MJ???_,', # 2 Charlie
    'b': 'MJ???_', # 2 Delta
    'c': 'MJ???_', # 3 Alpha
    'd': 'MJ???_', # 3 Bravo
    'e': 'MJ???_', # 3 Charlie
    'f': 'MJ???_', # 3 Delta
}

def execute_stm32_programmer_cli():
    command = r'"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD -ob displ'
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    return result.stdout

def extract_data0_value(output):
    match = re.search(r'Data0\s+:\s+0x([0-9A-Fa-f]+)\s+', output) # Use a regular expression to extract the value from Data0 line
    return match.group(1).lower()

def write_to_main_h(device):
    header_path = r'C:\Users\hrvoje\Documents\vsite\MJ808\mj8x8\Core\Inc\main.h'

    # Read the existing content of the file
    with open(header_path, 'r') as header_file:
        lines = header_file.readlines()

    # Find the lines between the // AutoDevice comments
    start_comment = '// AutoDevice\n'
    end_comment = '// AutoDevice\n'
    start_index = lines.index(start_comment) + 1
    end_index = lines.index(end_comment, start_index)

    # Replace the lines with the new device information
    lines[start_index:end_index] = ['#define ', f'{device}\t// what device to compile for\n']

    # Write the modified content back to the file
    with open(header_path, 'w') as header_file:
        header_file.writelines(lines)

if __name__ == "__main__":
    output = execute_stm32_programmer_cli()
    data0_value = extract_data0_value(output)
    mj_board = MJ8x8_boards[data0_value]
    print("board: ", mj_board)

    write_to_main_h(mj_board)
