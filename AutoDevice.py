import subprocess
import re
import time

# NOTE! do a skip-worktree on main.h

MJ8x8_boards = {
    '0': 'mj???', # 0 Alpha 
    '1': 'mj???', # 0 Bravo
    '2': 'mj???', # 0 Charlie
    '3': 'mj828', # 0 Delta
    '4': 'mj838', # 1 Alpha
    '5': 'mj???', # 1 Bravo
    '6': 'mj???', # 1 Charlie
    '7': 'mj???', # 1 Delta
    '8': 'mj808', # 2 Alpha
    '9': 'mj818', # 2 Bravo
    'a': 'mj???', # 2 Charlie
    'b': 'mj???', # 2 Delta
    'c': 'mj???', # 3 Alpha
    'd': 'mj???', # 3 Bravo
    'e': 'mj???', # 3 Charlie
    'f': 'mj???', # 3 Delta
}

# check if a stlink debugger is connected
def check_stlink_connected() -> bool:
    command = r'"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD freq=8000 sn=001C002F3137510A39383538 reset=HWrst'
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    
    # Check if the output contains an error string
    if "Error: Serial number not found" in result.stdout:
        print("STLink-V3 not connected ?? Aborting main.h editing.")
        exit() # Abort further execution if ST-Link is not detected
    
    if "ST-LINK error (DEV_CONNECT_ERR)" in result.stdout:
        print("STLink-V3 busy ??")
        exit() # Abort further execution if ST-Link is not detected

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
    lines[start_index:end_index] = ['#define ', f'{device.upper()}_\t// what device to compile for\n']

    # Write the modified content back to the file
    with open(header_path, 'w') as header_file:
        header_file.writelines(lines)

if __name__ == "__main__":
    if not check_stlink_connected():
        exit() # Abort further execution if ST-Link is not detected

    output = execute_stm32_programmer_cli() # try to connect to the programmer and read out option bytes
    hexID = extract_data0_value(output) # parse readout output: device CAN ID in hex
    mj_board = MJ8x8_boards[hexID] # determine board
    print("board: ", mj_board) # print it

    write_to_main_h(mj_board) # modify main.h

    exit()