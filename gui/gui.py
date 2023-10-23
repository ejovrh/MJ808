import tkinter as tk
from tkinter import Text, Label, messagebox
import serial
import threading
import queue

# Create a custom font size
custom_font_size = 10

# Create text entry fields with the default value "0x00" and a custom font size
num_fields_per_row = 10
num_rows = 6
default_value = "0x00"
entry_field_width = 6  # Width 
entry_field_height = 1  # Height 
custom_font = ("Arial", custom_font_size)

# Create a global variable to keep track of the serial connection and reading flag
ser = None
reading_flag = False
reading_thread = None  # Store the reading thread

# Create a Tkinter window
root = tk.Tk()
root.title("BQ25798 registers")  # Set the window title
root.geometry('{}x{}'.format(1024, 768))  # Resized window to fit the vertical arrangement

# Create frames for organizing the GUI elements with a height of 100
top_frame = tk.Frame(root, bg='gray', width=1024, height=50, pady=3)
bq25798_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
status_frame = tk.Frame(root, bg='gray', width=1024, pady=3)
button_frame = tk.Frame(root, bg='gray', width=1024, pady=3)

# Draw grid
top_frame.grid(row=1, column=0, sticky='nsew')
bq25798_frame.grid(row=2, column=0, sticky='nsew')
status_frame.grid(row=3, column=0, sticky='nsew')
button_frame.grid(row=4, column=0, sticky='nsew')

# Create a list to store references to text entry fields
entry_fields = []

# Function to update text entry fields with received data
def update_entry_fields(data):
    values = data.strip().split()  # Split the data using space as a delimiter
    for i, value in enumerate(values):
        if i < len(entry_fields):
            entry_fields[i].delete(1.0, tk.END)  # Clear the existing content
            entry_fields[i].insert(1.0, value)  # Insert the new value

# Function to read and display data from COM port
def read_com_data(ser, queue):
    try:
        while reading_flag:
            data = ser.readline().decode('utf-8')
            queue.put(data)  # Put the data into the queue
    except serial.SerialException as e:
        queue.put(f"Serial Error: {e}")  # Put the error message into the queue

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
    root.after(100, process_queue)  # Start processing the data queue

# Function to process the data queue
def process_queue():
    try:
        data = data_queue.get_nowait()  # Get data from the queue
        if data.startswith("Serial Error"):
            show_error(data)
        else:
            update_entry_fields(data)
        root.after(100, process_queue)  # Continue processing the data queue
    except queue.Empty:
        root.after(100, process_queue)  # Queue is empty, continue processing

# Function to stop reading data and close the serial connection
def stop_reading():
    global ser, reading_flag
    reading_flag = False
    if ser:
        ser.close()
        ser = None

# Function to exit the application
def exit_app():
    stop_reading()  # Stop reading data (close the serial connection)
    root.quit()  # Quit the main application loop

# Function to display an error popup
def show_error(message):
    messagebox.showerror("Error", message)
    print(f"Error: {message}")  # Print the error to the command prompt

# Function definition for RegToVal
def RegToVal(in_val=0, in_offset=0, in_stepsize=1):
    if not isinstance(in_val, str):
        raise ValueError("in_val must be a hexadecimal string.")
    if not in_val.isalnum():
        raise ValueError("in_val must be an alphanumeric string.")

    try:
        in_val = int(in_val, 16)
    except ValueError:
        raise ValueError("in_val must be a valid hexadecimal string.")

    if not (0 <= in_offset <= 65535) or not (0 <= in_stepsize <= 65535):
        raise ValueError("in_offset and in_stepsize must be within the range [0, 65535].")

    retval = (in_val * in_stepsize) + in_offset
    retval /= 1000  # Divide retval by 1000
    return retval

# list of 60 register names
register_name = ['REG00', 'REG01', 'REG03', 'REG05', 'REG06', 'REG08', 'REG09', 'REG0A', 'REG0B', 'REG0D',
                 'REG0E', 'REG0F', 'REG10', 'REG11', 'REG12', 'REG13', 'REG14', 'REG15', 'REG16', 'REG17',
                 'REG18', 'REG19', 'REG1B', 'REG1C', 'REG1D', 'REG1E', 'REG1F', 'REG20', 'REG21', 'REG22',
                 'REG23', 'REG24', 'REG25', 'REG26', 'REG34', 'REG27', 'REG28', 'REG29', 'REG2A', 'REG2B',
                 'REG2C', 'REG2D', 'REG2E', 'REG2F', 'REG30', 'REG31', 'REG33', 'REG35', 'REG37', 'REG39',
                 'REG3B', 'REG3D', 'REG3F', 'REG41', 'REG43', 'REG45', 'REG47', 'REG48', 'PG', 'IRQ',
                 'STAT']

# list of 60 offset values
register_offset = [2500, 0, 0, 0, 0, 0, 0, 0, 2800, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0]

# list of 60 offset values
register_step_size = [250, 10, 10, 100, 10, 40, 40, -1, 10, -1,
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                      -1, 10, -1, -1, -1, -1, -1, -1, -1, -1,
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                      -1, -1, -1, -1, -1, 1, 1, 1, 1, 1,
                      1, 1, 0.0976563, 0.5, 1, 1, -1, -1, -1, -1,
                      -1]

# Create bq25798 register fields in the bq25798_frame
for i in range(num_rows):
    for j in range(num_fields_per_row):
        
        field_num = i * num_fields_per_row + j
        if field_num >= 57:
            break

        entry_field = Text(bq25798_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        entry_field.insert(1.0, default_value)  # Set the default value
        entry_field.grid(row=i, column=j, padx=5, pady=5)
        entry_fields.append(entry_field)

# Create controller status entry fields in the status_frame
for i in range(3):
    entry_field = Text(status_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
    entry_field.insert(1.0, default_value)  # Set the default value
    entry_field.grid(row=0, column=i, padx=5, pady=5, sticky='nsew')
    entry_fields.insert(i, entry_field)


# Create buttons in the button_frame
start_button = tk.Button(button_frame, text="Start Reading", command=start_reading)
stop_button = tk.Button(button_frame, text="Stop Reading", command=stop_reading)
exit_button = tk.Button(button_frame, text="Exit", command=exit_app)

# Pack the buttons in the middle of the button_frame
start_button.pack(side="left", padx=5, pady=5)
stop_button.pack(side="left", padx=5, pady=5)
exit_button.pack(side="left", padx=5, pady=5)

# Create a queue for communication between threads
data_queue = queue.Queue()

root.mainloop()
