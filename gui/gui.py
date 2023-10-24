import threading
import queue
import tkinter as tk
from tkinter import Text, Label, messagebox
import serial

import time

# Create a queue for communication between threads
data_queue = queue.Queue()

custom_font_size = 10   # Create a custom font size
num_fields_per_row = 10 # how many register fields per row
num_rows = 6    # how many rows
default_value = "n/a"  # default value
entry_field_width = 6   #
entry_field_height = 1  #
custom_font = ("Arial", custom_font_size)  # font to be used
entry_fields = []    # container for register values as read from device and computed
data_buffer = ""    # data buffer for serial read
last_values = [default_value] * 60
last_update_times = [time.time()] * 60
last_change_times = [0] * 60
# clicked_field = 21
                     
# Create a global variable to keep track of the serial connection and reading flag
ser = None
reading_flag = False
reading_thread = None

class ToolTip:
    def __init__(self, widget, text):
        self.widget = widget
        self.text = text
        self.tooltip = None

        widget.bind("<Enter>", self.display_tooltip)
        widget.bind("<Leave>", self.hide_tooltip)

    def display_tooltip(self, event):
        x, y, _, _ = self.widget.bbox("insert")
        x += self.widget.winfo_rootx() + 25
        y += self.widget.winfo_rooty() + 25

        self.tooltip = tk.Toplevel(self.widget)
        self.tooltip.wm_overrideredirect(True)
        self.tooltip.wm_geometry(f"+{x}+{y}")
        label = Label(self.tooltip, text=self.text, background="lightyellow", relief="solid", borderwidth=1)
        label.pack()

    def hide_tooltip(self, event):
        if self.tooltip:
            self.tooltip.destroy()
            self.tooltip = None

# Create a Tkinter window
if __name__ == '__main__':
    root = tk.Tk()

root.title("BQ25798 registers")

# Create frames for organizing the GUI elements
top_frame = tk.Frame(root, bg='gray', width=1024, height=50, pady=3)
bq25798_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
status_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
button_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
byte_frame = tk.Frame(root, bg='gray', width=1024, height=200)

# Draw grid
top_frame.grid(row=0, column=0, sticky='nsew')
bq25798_frame.grid(row=1, column=0, sticky='nsew')
status_frame.grid(row=2, column=0, sticky='nsew')
button_frame.grid(row=3, column=0, sticky='nsew')
byte_frame.grid(row=4, column=0, sticky='nsew')

# Callback function to execute the appropriate function based on field_num
def on_entry_field_click(event, field_num):
    clicked_field = field_num   # save the last click

    if field_num < len(fptr):
        # Get the value of the clicked entry field
        clicked_entry_value = entry_fields[field_num].get("1.0", "end-1c")
        try:
            int(clicked_entry_value, 16)
            # Call the function from fptr based on field_num with the clicked value
            fptr_hover[field_num](clicked_entry_value, register_offset[field_num], register_step_size[field_num])
            # Do something with the result (e.g., display it)
            print(f"Result for {register_name[field_num]}: {clicked_entry_value}")
        except ValueError as e:
            return None
            # Handle the ValueError gracefully (e.g., display an error message)
            show_error(f"Error for {register_name[field_num]}: {e}")

# Function to update text entry fields with received data
def update_entry_fields(data):
    # Append the received data to the buffer
    global data_buffer
    data_buffer += data

    # Keep processing as long as there are newline characters in the buffer
    while '\n' in data_buffer:
        # Split the buffer by newline characters
        data_lines, data_buffer = data_buffer.split('\n', 1)
        values = data_lines.strip().split()

        # Process the values and update the entry fields
        current_time = time.time()
        for i, value in enumerate(values):
            if i < len(entry_fields):
                entry_fields[i].delete(1.0, tk.END)
                result = str(fptr[i](value, register_offset[i], register_step_size[i])) + register_unit[i]

                # if result != last_values[clicked_field]:
                #     fptr_hover[i](value, register_offset[i], register_step_size[i])


                if result != last_values[i]:
                    if entry_fields[i].cget("bg") != "yellow":
                        entry_fields[i].config(bg='yellow')
                        last_change_times[i] = current_time
                elif current_time - last_change_times[i] >= 5:
                    entry_fields[i].config(bg='white')

                entry_fields[i].insert(1.0, result)
                last_values[i] = result

# Function to reset the background color to white
def reset_bg_color(i):
    entry_fields[i].config(bg='white')

# Function to read and display data from COM port
def read_com_data(ser, queue):
    try:
        while reading_flag:
            data = ser.readline().decode('ascii')
            queue.put(data)
    except serial.SerialException as e:
        queue.put(f"Serial Error: {e}")

# Function to read and process data from the serial port
def read_serial_data(ser, data_queue):
    while reading_flag:
        data = ser.readline().decode('ascii')
        data_queue.put(data)

# Function to update the GUI from the data queue
def update_gui_from_queue():
    while reading_flag:
        try:
            data = data_queue.get(timeout=1)  # Adjust the timeout as needed
            # Process the data and update the GUI here
            update_entry_fields(data)
            #print(data)  # For debugging, you can print the data
        except queue.Empty:
            pass
        
# Function to start reading data
def start_reading():
    global reading_flag, reading_thread, ser
    if not ser:
        try:
            ser = serial.Serial('COM4', 115200)
        except serial.SerialException as e:
            show_error(f"Serial Error: {e}")
            return
    reading_flag = True
    reading_thread = threading.Thread(target=read_serial_data, args=(ser, data_queue))
    reading_thread.start()
    gui_thread = threading.Thread(target=update_gui_from_queue)
    gui_thread.start()

# Function to process the data queue
def process_queue():
    try:
        data = data_queue.get_nowait()
        if data.startswith("Serial Error"):
            show_error(data)
        else:
            update_entry_fields(data)
        root.after(100, process_queue)
    except queue.Empty:
        root.after(100, process_queue)

# Function to stop reading data and close the serial connection
def stop_reading():
    global ser, reading_flag
    reading_flag = False
    if ser:
        ser.close()
        ser = None

# Function to exit the application
def exit_app():
    stop_reading()
    root.quit()

# Function to display an error popup
def show_error(message):
    messagebox.showerror("Error", message)
    print(f"Error: {message}")

# Function for calculating values based on register data, offset and bit stpe size
def RegToVal(in_val=0, in_offset=0, in_stepsize=1):
    in_val = int(in_val, 16)
    retval = (in_val * in_stepsize) + in_offset
    retval /= 1000
    return retval

# Function for calculating values based on register data, offset and bit stpe size
def RegToTemp(in_val=0, in_offset=0, in_stepsize=1):
    in_val = int(in_val, 16)

    if in_val & (1 << 15):
        # Compute the two's complement
        in_val -= 1 << 16

    retval = (in_val * in_stepsize) + in_offset
    retval /= 1000
    return retval

def REG08(in_val, _ignore1, _ignore2):
    return in_val

def REG09(in_val, _ignore1, _ignore2):
    return in_val

def REG0A(in_val, _ignore1, _ignore2):
    return in_val

def REG0D(in_val, _ignore1, _ignore2):
    return in_val

def REG0E(in_val, _ignore1, _ignore2):
    return in_val

def REG0F(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG10(in_val, _ignore1, _ignore2):
    return in_val

def REG11(in_val, _ignore1, _ignore2):
    return in_val

def REG12(in_val, _ignore1, _ignore2):
    return in_val

def REG13(in_val, _ignore1, _ignore2):
    return in_val

def REG14(in_val, _ignore1, _ignore2):
    return in_val

def REG15(in_val, _ignore1, _ignore2):
    return in_val

def REG16(in_val, _ignore1, _ignore2):
    return in_val

def REG17(in_val, _ignore1, _ignore2):
    return in_val

def REG18(in_val, _ignore1, _ignore2):
    return in_val

def REG19(in_val, _ignore1, _ignore2):
    return in_val

def REG1B(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG1C(in_val, _ignore1, _ignore2):
    return in_val

def REG1D(in_val, _ignore1, _ignore2):
    return in_val

def REG1E(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG1F(in_val, _ignore1, _ignore2):
    return in_val

def REG20(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG21(in_val, _ignore1, _ignore2):
    return in_val

def REG22(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG23(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG24(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG25(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG26(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG27(in_val, _ignore1, _ignore2):
    return in_val

def REG28(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG29(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG2A(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG2B(in_val, _ignore1, _ignore2):
    return in_val

def REG2C(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG2D(in_val, _ignore1, _ignore2):
    return in_val

def REG2E(in_val, _ignore1, _ignore2):
    return in_val

def REG2F(in_val, _ignore1, _ignore2):
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix

    # Pad the binary string to 8 characters with leading zeros if needed
    binary_str = binary_str.zfill(8)

    # place bit info in byte_frame
    for i in range(8):
        label_text = "bit" + str(i)
        label = tk.Label(byte_frame, text=label_text)
        label.config(bg='gray')
        label.grid(row=i+1, column=0, padx=5, pady=5, sticky='E')
        bit_field = Text(byte_frame, width=50, height=entry_field_height, font=custom_font)
        bit_field.delete(1.0, tk.END)  # Clear the bit_field
        bit_field.insert(1.0, binary_str[i])
        bit_field.grid(row=i+1, column=2, padx=2, sticky='W')

    return in_val

def REG30(in_val, _ignore1, _ignore2):
    return in_val

def REG47(in_val, _ignore1, _ignore2):
    return in_val

# Function that returns the input
def retval(input, _ignore1, _ignore2):
    return input

register_name = ['REG00', 'REG01', 'REG03', 'REG05', 'REG06', 'REG08', 'REG09', 'REG0A', 'REG0B', 'REG0D',
                 'REG0E', 'REG0F', 'REG10', 'REG11', 'REG12', 'REG13', 'REG14', 'REG15', 'REG16', 'REG17',
                 'REG18', 'REG19', 'REG1B', 'REG1C', 'REG1D', 'REG1E', 'REG1F', 'REG20', 'REG21', 'REG22',
                 'REG23', 'REG24', 'REG25', 'REG26', 'REG27', 'REG28', 'REG29', 'REG2A', 'REG2B', 'REG2C', 
                 'REG2D', 'REG2E', 'REG2F', 'REG30', 'REG31', 'REG33', 'REG35', 'REG37', 'REG39', 'REG3B', 
                 'REG3D', 'REG3F', 'REG41', 'REG43', 'REG45', 'REG47', 'REG48', 'PG', 'IRQ', 'STAT']

register_offset = [2500, 0, 0, 0, 0, 0, 0, 0, 2800, 0,  # REG00 to REG0D
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG0E to REG17
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG18 to REG22
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG23 to REG2C
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG2D to REG3B
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0         # REG3D to REG48, along with PG, IRQ, STAT
                   ]

register_step_size = [250, 10, 10, 100, 10, 40, 40, -1, 10, -1, # REG00 to REG0D
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG0E to REG17
                      -1, 10, -1, -1, -1, -1, -1, -1, -1, -1,   # REG18 to REG22
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG23 to REG2C
                      -1, -1, -1, -1, 1, 1, 1, 1, 1, 1,         # REG2D to REG3B
                      1, 0.0976563, 0.5, 1, 1, -1, -1, -1, -1, -1   # REG3D to REG48, along with PG, IRQ, STAT
                      ]

register_description = [
                        "Minimal System Voltage",   # 0h
                        "Charge Voltage Limit", # 1h
                        "Charge Current Limit", # 3h
                        "Input Voltage Limit",  # 5h
                        "Input Current Limit", # 6h
                        "Precharge Control", # 8h
                        "Termination Control", # 9h
                        "Re-charge Contro", # Ah
                        "VOTG regulation", # Bh
                        "IOTG regulation", # Dh
                        "Timer Control", # Eh
                        "Charger Control 0", # Fh
                        "Charger Control 1", # 10h
                        "Charger Control 2", # 11h
                        "Charger Control 3", # 12h
                        "Charger Control 4", # 13h
                        "Charger Control 5", # 14h
                        "MPPT Control", # 15h
                        "Temperature Control", # 16h
                        "NTC Control 0", # 17h
                        "NTC Control 1", # 18h
                        "ICO Current Limit", # 19h
                        "Charger Status 0", # 1Bh
                        "Charger Status 1", # 1Ch
                        "Charger Status 2", # 1Dh
                        "Charger Status 3", # 1Eh
                        "Charger Status 4", # 1Fh
                        "FAULT Status 0", # 20h
                        "FAULT Status 1", # 21h
                        "Charger Flag 0", # 22h
                        "Charger Flag 1", # 23h
                        "Charger Flag 2", # 24h
                        "Charger Flag 3", # 25h
                        "FAULT Flag 0", # 26h
                        "FAULT Flag 1", # 27h
                        "Charger Mask 0", # 28h
                        "Charger Mask 1", # 29h
                        "Charger Mask 2", # 2Ah
                        "Charger Mask 3", # 2Bh
                        "FAULT Mask 0", # 2Ch
                        "FAULT Mask 1", # 2Dh
                        "ADC Control", # 2Eh
                        "ADC Function Disable 0", # 2Fh
                        "ADC Function Disable 1", # 30h
                        "IBUS ADC", # 31h
                        "IBAT ADC", # 33h
                        "VBUS ADC", # 35h
                        "VAC1 ADC" , # 37h
                        "VAC2 ADC", # 39h
                        "VBAT ADC", # 3Bh
                        "VSYS ADC", # 3Dh
                        "TS ADC", # 3Fh
                        "TDIE_ADC", # 41h
                        "D+ ADC", # 43h
                        "D- ADC", # 45h
                        "DPDM Driver", # 47h
                        "Part Information", # 48h
                        "LMR34206 Power Good", # 
                        "BQ2798 Interrupt", # 
                        "BQ2798 status" # 
]

register_unit = [
                "V", "V", "A", "V", "A", "", "", "", "V", "",   # REG00 to REG0D
                "", "", "", "", "", "", "", "", "", "",         # REG0E to REG17
                "", "A", "", "", "", "", "", "", "", "",        # REG18 to REG22
                "", "", "", "", "", "", "", "", "", "",         # REG23 to REG2C
                "", "", "", "", "A", "A", "V", "V", "V", "V",   # REG2D to REG3B
                "V", "%", "°C", "V", "V", "", "", "", "", "",   # REG3D to REG48, along with PG, IRQ, STAT
]

fptr = [RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, retval, retval, retval, RegToVal, retval, # REG00 to REG0D
        retval, retval, retval, retval, retval, retval, retval, retval, retval, retval,   # REG0E to REG17
        retval, RegToVal, retval, retval, retval, retval, retval, retval, retval, retval,   # REG18 to REG22
        retval, retval, retval, retval, retval, retval, retval, retval, retval, retval,   # REG23 to REG2C
        retval, retval, retval, retval, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, # REG2D to REG3B
        RegToVal, RegToVal, RegToTemp, RegToVal, RegToVal, retval, retval, retval, retval, retval    # REG3D to REG48, along with PG, IRQ, STAT
        ]

fptr_hover = [retval, retval, retval, retval, retval, REG08, REG09, REG0A, retval, REG0D, # REG00 to REG0D
                REG0E, REG0F, REG10, REG11, REG12, REG13, REG14, REG15, REG16, REG17,   # REG0E to REG17
                REG18, retval, REG1B, REG1C, REG1D, REG1E, REG1F, REG20, REG21, REG22,   # REG18 to REG22
                REG23, REG24, REG25, REG26, REG27, REG28, REG29, REG2A, REG2B, REG2C,   # REG23 to REG2C
                REG2D, REG2E, REG2F, REG30, retval, retval, retval, retval, retval, retval, # REG2D to REG3B
                retval, retval, retval, retval, retval, REG47, retval, retval, retval, retval    # REG3D to REG48, along with PG, IRQ, STAT
            ]

# create bq25798 register labels and fields
field_num = 0
for i in range(6):  # 6 rows
    for j in range(10): # 10 register values per row
        if field_num >= 57:
            break

        # Create labels for register names
        label = Label(bq25798_frame, text=register_name[field_num])
        label.grid(row=i, column=2*j, padx=2, pady=5, sticky="e")

        # Add tooltips to the labels
        tooltip_text = register_description[field_num]
        tooltip = ToolTip(label, tooltip_text)

        # Create text entry fields for register values
        entry_field = Text(bq25798_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        entry_field.insert(1.0, str(default_value))
        entry_field.grid(row=i, column=2*j + 1, padx=2, pady=5)
        entry_fields.append(entry_field)
        entry_field.bind("<FocusIn>", lambda event, num=field_num: on_entry_field_click(event, num)) # Bind the click event to the entry field

        # Bind the click event to the entry field
        entry_field.bind("<FocusIn>", lambda event, num=field_num: on_entry_field_click(event, num))

        field_num +=1 
        
# create charger status labels and fields
for i in range(3):
    # Create labels for status registers
    label = Label(status_frame, text=register_name[field_num])  
    label.grid(row=0, column=2 * i, padx=2, pady=5, sticky="e")

    # Add tooltips to the labels
    tooltip_text = register_description[field_num]
    tooltip = ToolTip(label, tooltip_text)
    
    # Create text entry fields for status registers
    entry_field = Text(status_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
    entry_field.insert(1.0, default_value)
    entry_field.grid(row=0, column=2 * i + 1, padx=2, pady=5)
    entry_fields.append(entry_field)
    field_num +=1 

# Create buttons for controlling the application
start_button = tk.Button(button_frame, text="Start Reading", command=start_reading)
stop_button = tk.Button(button_frame, text="Stop Reading", command=stop_reading)
exit_button = tk.Button(button_frame, text="Exit", command=exit_app)

# Configure button frame layout
button_frame.columnconfigure(0, weight=1)
button_frame.columnconfigure(1, weight=3)
button_frame.columnconfigure(2, weight=1)

# Place buttons in the button frame
start_button.grid(row=0, column=1, padx=5, pady=5, sticky='W')
stop_button.grid(row=0, column=1, padx=5, pady=5)
exit_button.grid(row=0, column=1, padx=5, pady=5, sticky='E')

# Initialize a queue for data processing
data_queue = queue.Queue()

# Start the Tkinter main loop
root.mainloop()
