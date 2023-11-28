import subprocess
import re
import time
import os
import shutil

MJ8X8_HEADER_PATH = r'C:\\Users\\hrvoje\\Documents\\vsite\\MJ808\\mj8x8\\Core\\Inc\\mj8x8'

# dictionary for detected boards
MJ8x8_boards = {}

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

def parse_mj8x8_header_for_devices(directory): # parses device header file for mjxxx_activity_t structure and deduces activity
    commands_header_path = os.path.join(directory, "mj8x8_commands.h")

    # Check if the commands header file exists
    if not os.path.isfile(commands_header_path):
        print("mj8x8_commands.h not found.")
        return

    # Read and parse the commands header file
    with open(commands_header_path, 'r') as commands_header:
        inside_struct_device = False

        # Iterate through lines in the file
        for line in commands_header:
            # Start capturing lines inside the struct device_t
            if 'typedef union' in line:
                inside_struct_device = True
                continue

            # Stop capturing lines inside the struct device_t
            if inside_struct_device and '};' in line:
                inside_struct_device = False
                break

            # Capture lines inside the struct device_t
            if inside_struct_device and 'uint16_t' in line and ':1' in line:
                # Use regular expressions to find the device name and hex ID
                device_name_match = re.search(r'uint16_t\s+(\w+)\s*:', line)
                hex_id_match = re.search(r'\/\/\s*([0-9A-Fa-f]+)\s*\/\/', line)
                
                if device_name_match and hex_id_match:
                    device_name = device_name_match.group(1)    # e.g. mj828
                    hex_id = hex_id_match.group(1)  # e.g. hex 3

                    # Insert the device into the dictionary
                    MJ8x8_boards[hex_id] = device_name

def extract_data0_value(output) -> str:
    match = re.search(r'Data0\s+:\s+0x([0-9A-Fa-f]+)\s+', output)
    return match.group(1).lower()

def copy_main_h_if_not_exists():
    header_path = r'C:\Users\hrvoje\Documents\vsite\MJ808\mj8x8\Core\Inc\main.h'
    tracked_header_path = r'C:\Users\hrvoje\Documents\vsite\MJ808\mj8x8\Core\Inc\main.h_tracked'

    if not os.path.isfile(header_path):
        shutil.copy(tracked_header_path, header_path)

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

    copy_main_h_if_not_exists()  # Check if main.h exists and copy main.h_tracked if not
    output = execute_stm32_programmer_cli() # try to connect to the programmer and read out option bytes
    parse_mj8x8_header_for_devices(MJ8X8_HEADER_PATH) # parse mj8x8_commands.h for devices
    hexID = extract_data0_value(output) # parse readout output: device CAN ID in hex
    mj_board = MJ8x8_boards[hexID] # determine board

    print("board: ", mj_board) # print it
    write_to_main_h(mj_board) # modify main.h

    exit()