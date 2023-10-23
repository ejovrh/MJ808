# Import necessary modules
import tkinter as tk
from tkinter import Text, Label, messagebox
import serial
import threading
import queue


custom_font_size = 10   # Create a custom font size
num_fields_per_row = 10 # how many register fields per row
num_rows = 6    # how many rows
default_value = "0x00"  # default value
entry_field_width = 6   #
entry_field_height = 1  #
custom_font = ("Arial", custom_font_size)  # font to be used
entry_fields = []   # container for register values as read from device and computed
data_buffer = ""    # data buffer for serial read

# Create a global variable to keep track of the serial connection and reading flag
ser = None
reading_flag = False
reading_thread = None

# Create a Tkinter window
root = tk.Tk()
root.title("BQ25798 registers")

# Create frames for organizing the GUI elements
top_frame = tk.Frame(root, bg='gray', width=1024, height=50, pady=3)
bq25798_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
status_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
button_frame = tk.Frame(root, bg='gray', width=1024, pady=3)

# Draw grid
top_frame.grid(row=0, column=0, sticky='nsew')
bq25798_frame.grid(row=1, column=0, sticky='nsew')
status_frame.grid(row=2, column=0, sticky='nsew')
button_frame.grid(row=3, column=0, sticky='nsew')

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
        for i, value in enumerate(values):
            if i < len(entry_fields):
                entry_fields[i].delete(1.0, tk.END)
                entry_fields[i].insert(1.0, fptr[i](value, register_offset[i], register_step_size[i]))

# Function to read and display data from COM port
def read_com_data(ser, queue):
    try:
        while reading_flag:
            data = ser.readline().decode('ascii')
            queue.put(data)
    except serial.SerialException as e:
        queue.put(f"Serial Error: {e}")

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
    reading_thread = threading.Thread(target=read_com_data, args=(ser, data_queue))
    reading_thread.start()
    root.after(100, process_queue)

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

# Function that returns the input
def retval(input, two, three):
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

fptr = [RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, retval, RegToVal, retval, # REG00 to REG0D
        retval, retval, retval, retval, retval, retval, retval, retval, retval, retval,   # REG0E to REG17
        retval, RegToVal, retval, retval, retval, retval, retval, retval, retval, retval,   # REG18 to REG22
        retval, retval, retval, retval, retval, retval, retval, retval, retval, retval,   # REG23 to REG2C
        retval, retval, retval, retval, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, RegToVal,         # REG2D to REG3B
        RegToVal, RegToVal, RegToVal, RegToVal, RegToVal, retval, retval, retval, retval, retval   # REG3D to REG48, along with PG, IRQ, STAT
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

        # Create text entry fields for register values
        entry_field = Text(bq25798_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        entry_field.insert(1.0, default_value)
        entry_field.grid(row=i, column=2*j + 1, padx=2, pady=5)
        entry_fields.append(entry_field)
        field_num +=1 
        
# create charger status labels and fields
for i in range(3):
    # Create labels for status registers
    label = Label(status_frame, text=register_name[field_num])  
    label.grid(row=0, column=2 * i, padx=2, pady=5, sticky="e")

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
button_frame.columnconfigure(1, weight=1)
button_frame.columnconfigure(2, weight=1)

# Place buttons in the button frame
start_button.grid(row=0, column=1, padx=5, pady=5, sticky='W')
stop_button.grid(row=0, column=1, padx=5, pady=5)
exit_button.grid(row=0, column=1, padx=5, pady=5, sticky='E')

# Initialize a queue for data processing
data_queue = queue.Queue()

# Start the Tkinter main loop
root.mainloop()
