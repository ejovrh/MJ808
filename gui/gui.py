import threading
import queue
import tkinter as tk
from tkinter import Text, Label, messagebox
import serial
import time
import inspect

from strings import *

data_queue = queue.Queue()  # Initialize a queue for data processing

num_fields_per_row = 10 # how many register fields per row
num_rows = 6    # how many rows
default_value = "n/a"  # default value
entry_field_width = 6   #
entry_field_height = 0  #
custom_font = ("Arial", 10)  # font to be used
entry_fields = []    # container for register values as read from device and computed
fields = {}  # dictionary
bit_description_fields = {}  # dictionary
data_buffer = ""    # data buffer for serial read
current_values:int = [default_value] * 60
last_values:int = [default_value] * 60
last_update_times = [time.time()] * 60
last_change_times = [0] * 60
clicked_field = 21
current_label = None  # Variable to keep track of the currently clicked label
                    
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
bottom_frame = tk.Frame(root, bg='gray', width=1024, height=50, pady=3)

# Draw grid
top_frame.grid(row=0, column=0, sticky='nsew')
bq25798_frame.grid(row=1, column=0, sticky='nsew')
status_frame.grid(row=2, column=0, sticky='nsew')
button_frame.grid(row=3, column=0, sticky='nsew')
byte_frame.grid(row=4, column=0, sticky='nsew')
top_frame.grid(row=5, column=0, sticky='nsew')

# Callback function to execute the appropriate function based on field_num
def on_entry_field_click(event, field_num:int):
    if field_num < len(fptr):
        try:
            fptr_hover[field_num](current_values[field_num], register_offset[field_num], register_step_size[field_num]) # Call the function from fptr based on field_num with the clicked value
            print(f"Result for {register_name[field_num]}: {current_values[field_num]}")
        except ValueError as e:
            return None
            show_error(f"Error for {register_name[field_num]}: {e}")    # Handle the ValueError gracefully (e.g., display an error message)

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
        current_time = time.time()

        for i, value in enumerate(values):  # loop over values with i and value
            if i < len(entry_fields):
                hexvalue:str = value    # save the original value as hex
                value = int(value, 16)  # convert value to integer

                if fptr[i] != retnone:   # only if the function pointer is not retval
                    current_values[i] = fptr[i](value, register_offset[i], register_step_size[i])  # execute whatever the fuction pointer points to
                else:   # if it is retval
                    current_values[i] = hexvalue   # display the hex value (registers with bitfie)

                # FIXME
                # if current_values[i] != last_values[clicked_field]:
                #     if fptr_hover[i] != retval:
                #         fptr_hover[i](str(value), register_offset[i], register_step_size[i])

                entry_fields[i].delete(1.0, tk.END)

                if current_values[i] != last_values[i]:
                    if entry_fields[i].cget("bg") != "yellow":
                        entry_fields[i].config(bg='yellow')
                        last_change_times[i] = current_time
                elif current_time - last_change_times[i] >= 5:
                    entry_fields[i].config(bg='white')

                entry_fields[i].insert(1.0, f"{current_values[i]}{register_unit[i]}\n")
                last_values[i] = current_values[i]

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
def RegToVal(in_val:int, in_offset:int, in_stepsize:int) -> int:
    return ( ((in_val * in_stepsize) + in_offset ) / 1000 )

# Function for calculating values based on register data, offset and bit stpe size
def RegToTemp(in_val:int, in_offset:int, in_stepsize:float):
    if in_val & (1 << 15):  # Compute the two's complement
        in_val -= 1 << 16

    return ( ((in_val * in_stepsize) + in_offset ) / 1000 )

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




def label_click(event):
    global current_label  # Declare current_label as a global variable
    if current_label:
        # Unbold the previous label
        # current_label.config(font=("normal", 10))
        current_label.config(bg="white")
        
    # Set the current_label to the label that was clicked
    current_label = event.widget
    # Bold the clicked label
    # current_label.config(font=("bold", 12))
    current_label.config(bg="lightblue")

def dectostr(in_val:int) -> str:
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix
    binary_str = binary_str.zfill(8) # Pad the binary string to 8 characters with leading zeros if needed"
    return binary_str

def populate_8_bitfields(in_name:str, in_val:int, description:str, bit_set, bit_unset:str):
    binary_str = dectostr(in_val)

    for i in range(8):
        key = str("bit_field_name"+str(i)) # in DS pp.59 - column "Field": tree text byte register description
        fields[key].delete(1.0, tk.END)  # Clear the bit_field
        fields[key].insert(1.0, in_name[i])

        key = str("bit_field"+str(i)) # bit value according to column "Description"
        fields[key].delete(1.0, tk.END)  # Clear the bit_field

        if binary_str[i] == 1:
            fields[key].insert(1.0, bit_set[i])
        else:
            fields[key].insert(1.0, bit_unset[i])

        key = str("bit_field_description"+str(i)) # in DS pp.59 - column "Description": tree text byte register description
        fields[key].delete(1.0, tk.END)  # Clear the bit_field
        fields[key].insert(1.0, description[i])

def REG0F(in_val:int, _ignore1:int, _ignore2:int):
    populate_8_bitfields(reg0f_bit_names, in_val, reg0f_description, reg0f_bits_set, reg0f_bits_unset)
    return in_val

def REG10(in_val, _ignore1, _ignore2):
    return in_val

def REG11(in_val, _ignore1, _ignore2):
    return in_val

def REG12(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg12_bit_names, in_val, reg12_description, reg12_bits_set, reg12_bits_unset)
    return in_val

def REG13(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg13_bit_names, in_val, reg13_description, reg13_bits_set, reg13_bits_unset)
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
    populate_8_bitfields(reg1b_bit_names, in_val, reg1b_description, reg1b_bits_set, reg1b_bits_unset)
    return in_val

def REG1C(in_val, _ignore1, _ignore2):
    return in_val

def REG1D(in_val, _ignore1, _ignore2):
    return in_val

def REG1E(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg1e_bit_names, in_val, reg1e_description, reg1e_bits_set, reg1e_bits_unset)
    return in_val

def REG1F(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg1f_bit_names, in_val, reg1f_description, reg1f_bits_set, reg1f_bits_unset)
    return in_val

def REG20(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg20_bit_names, in_val, reg20_description, reg20_bits_set, reg20_bits_unset)
    return in_val

def REG21(in_val, _ignore1, _ignore2):
    return in_val

def REG22(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg22_bit_names, in_val, reg22_description, reg22_bits_set, reg22_bits_unset)
    return in_val

def REG23(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg23_bit_names, in_val, reg23_description, reg23_bits_set, reg23_bits_unset)
    return in_val

def REG24(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg24_bit_names, in_val, reg24_description, reg24_bits_set, reg24_bits_unset)
    return in_val

def REG25(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg25_bit_names, in_val, reg25_description, reg25_bits_set, reg25_bits_unset)
    return in_val

def REG26(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg26_bit_names, in_val, reg26_description, reg26_bits_set, reg26_bits_unset)
    return in_val

def REG27(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg27_bit_names, in_val, reg27_description, reg27_bits_set, reg27_bits_unset)
    return in_val

def REG28(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg28_bit_names, in_val, reg28_description, reg28_bits_set, reg28_bits_unset)
    return in_val

def REG29(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg29_bit_names, in_val, reg29_description, reg29_bits_set, reg29_bits_unset)
    return in_val

def REG2A(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2a_bit_names, in_val, reg2a_description, reg2a_bits_set, reg2a_bits_unset)
    return in_val

def REG2B(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2b_bit_names, in_val, reg2b_description, reg2b_bits_set, reg2b_bits_unset)
    return in_val

def REG2C(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2c_bit_names, in_val, reg2c_description, reg2c_bits_set, reg2c_bits_unset)
    return in_val

def REG2D(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2d_bit_names, in_val, reg2d_description, reg2d_bits_set, reg2d_bits_unset)
    return in_val

def REG2E(in_val, _ignore1, _ignore2):
    return in_val

def REG2F(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2f_bit_names, in_val, reg2f_description, reg2f_bits_set, reg2f_bits_unset)
    return in_val

def REG30(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg30_bit_names, in_val, reg30_description, reg30_bits_set, reg30_bits_unset)
    return in_val

def REG47(in_val, _ignore1, _ignore2):
    return in_val

# Function that returns the input
def retnone(input, _ignore1, _ignore2):
    return None

fptr = [RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, retnone, retnone, retnone, RegToVal, retnone, # REG00 to REG0D
        retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone,   # REG0E to REG17
        retnone, RegToVal, retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone,   # REG18 to REG22
        retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone, retnone,   # REG23 to REG2C
        retnone, retnone, retnone, retnone, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, # REG2D to REG3B
        RegToVal, RegToVal, RegToTemp, RegToVal, RegToVal, retnone, retnone, retnone, retnone, retnone    # REG3D to REG48, along with PG, IRQ, STAT
        ]

fptr_hover = [retnone, retnone, retnone, retnone, retnone, REG08, REG09, REG0A, retnone, REG0D, # REG00 to REG0D
                REG0E, REG0F, REG10, REG11, REG12, REG13, REG14, REG15, REG16, REG17,   # REG0E to REG17
                REG18, retnone, REG1B, REG1C, REG1D, REG1E, REG1F, REG20, REG21, REG22,   # REG18 to REG22
                REG23, REG24, REG25, REG26, REG27, REG28, REG29, REG2A, REG2B, REG2C,   # REG23 to REG2C
                REG2D, REG2E, REG2F, REG30, retnone, retnone, retnone, retnone, retnone, retnone, # REG2D to REG3B
                retnone, retnone, retnone, retnone, retnone, REG47, retnone, retnone, retnone, retnone    # REG3D to REG48, along with PG, IRQ, STAT
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
        entry_field.bind("<FocusIn>", lambda event, num=field_num: on_entry_field_click(event, num)) # Bind the click event to the entry field
        entry_field.bind("<Button-1>", label_click)  # Bind the click event to the label

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

# byte_frame.columnconfigure(0, weight=1)
# byte_frame.columnconfigure(1, weight=1
byte_frame.rowconfigure(0, weight=1)
byte_frame.rowconfigure(1, weight=1)


# create itemps in byte_frame 
j:int = 0
for i in range(8):  
    key = str("bit_number"+str(i))   # in DS pp.59 - column "Bit": bit0 to bit7`
    label_text = "bit" + str(i)
    fields[key] = Label(byte_frame, width=4, text=label_text)
    fields[key].config(bg='gray')
    fields[key].grid(row=i+j+1, column=0, padx=5, pady=5, sticky='W')   

    key = str("bit_field_name"+str(i)) # in DS pp.59 - column "Field": tree text byte register description
    fields[key] = Text(byte_frame, width=20, height=entry_field_height, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=i+j+1, column=1, padx=2, sticky='W')

    key = str("bit_field"+str(i)) # bit value according to column "Description"
    fields[key] = Text(byte_frame, width=75, height=entry_field_height, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=i+j+1, column=1, padx=2)

    key = str("bit_field_description"+str(i)) # in DS pp.59 - column "Description": tree text byte register description
    fields[key] = Text(byte_frame, width=125, height=2, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=i+j+2, column=1, padx=2, sticky='W')
    j += 1

# Place buttons in the button frame
start_button.grid(row=0, column=1, padx=5, pady=5, sticky='W')
stop_button.grid(row=0, column=1, padx=5, pady=5)
exit_button.grid(row=0, column=1, padx=5, pady=5, sticky='E')

# Start the Tkinter main loop
root.mainloop()
