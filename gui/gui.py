import tkinter as tk
from tkinter import Text, messagebox
import serial
import threading
import queue

# Create a custom font size
custom_font_size = 12

# Create text entry fields with the default value "0x00" and a custom font size
num_fields_per_row = 10
num_rows = 6
default_value = "0x00"
entry_field_width = 6  # Width of 6 characters
entry_field_height = 1  # Height of 1 character (slightly taller than the default font size)
custom_font = ("Arial", custom_font_size)

# Create a global variable to keep track of the serial connection and reading flag
ser = None
reading_flag = False
reading_thread = None  # Store the reading thread

# Create a Tkinter window
root = tk.Tk()
root.title("BQ25798 registers")  # Set the window title
root.geometry("1024x768")  # Set the window size to 1024x768
root.resizable(False, False)  # Make the window non-resizable

# Create frames for organizing the GUI elements with a height of 100
top_frame = tk.Frame(root, width=1000, height=10, highlightbackground="black", highlightthickness=1)
status_frame = tk.Frame(root, width=1000, height=100, highlightbackground="black", highlightthickness=1)
bq25798_frame = tk.Frame(root, width=1000, height=100, highlightbackground="black", highlightthickness=1)
button_frame = tk.Frame(root, width=1000, height=100, highlightbackground="black", highlightthickness=1)

# Draw grid
top_frame.grid(row=1, column=0, sticky='nsew')
status_frame.grid(row=2, column=0, sticky='nsew')
bq25798_frame.grid(row=3, column=0, sticky='nsew')
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

# Create controller status entry fields in the status_frame
for i in range(3):
    entry_field = Text(status_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
    entry_field.insert(1.0, default_value)  # Set the default value
    entry_field.grid(row=0, column=i, padx=5, pady=5, sticky='nsew')
    entry_fields.insert(i, entry_field)
    
# Create bq25798 register fields in the bq25798_frame
for i in range(num_rows):
    for j in range(num_fields_per_row):
        field_num = i * num_fields_per_row + j
        if field_num >= 57:
            break
        entry_field = Text(bq25798_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        entry_field.insert(1.0, default_value)  # Set the default value
        entry_field.grid(row=i, column=j, padx=5, pady=5, sticky='nsew')
        entry_fields.append(entry_field)

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
