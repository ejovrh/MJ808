import subprocess
import re
import time

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

# check if a stlink debugger is connected
def check_stlink_connected() -> bool:
    command = r'"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD freq=8000 sn=001C002F3137510A39383538 reset=HWrst'
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    
    # Check if the output contains an error string
    if "Error: Serial number not found" in result.stdout:
        return False
    
    return True

# try to read out option byte
def execute_stm32_programmer_cli() -> str:
    command = r'"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD freq=8000 sn=001C002F3137510A39383538 reset=HWrst -ob displ'
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True) # First connection attempt

    # Check if the output contains the string "Error: No STM32 target found!"
    if "Error: No STM32 target found!" in result.stdout:
        time.sleep(1) # sleep for 1s
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True) # First connection attempt

        # If the second attempt also fails, print an error message and return an empty string
        if "UPLOADING OPTION BYTES DATA" not in result.stdout:
            print("MJ8x8 device not powered on ??. Aborting main.h editing.")
            exit() # Abort further execution if ST-Link cant connect

    return result.stdout

def extract_data0_value(output) -> str:
    match = re.search(r'Data0\s+:\s+0x([0-9A-Fa-f]+)\s+', output)
    return match.group(1).lower()

def write_to_main_h(device) -> None:
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
    if not check_stlink_connected():
        print("STLink-V3 not connected ?? Aborting main.h editing.")
        exit() # Abort further execution if ST-Link is not detected

    output = execute_stm32_programmer_cli() # try to connect to the programmer and read out option bytes
    data0_value = extract_data0_value(output) # parse readout output
    mj_board = MJ8x8_boards[data0_value] # determine board
    print("board: ", mj_board) # print it

    write_to_main_h(mj_board) # modify main.h

    exit()