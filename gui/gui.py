import tkinter as tk
from tkinter import Text, messagebox
import serial
import threading
import queue

# Create a global variable to keep track of the serial connection and reading flag
ser = None
reading_flag = False
reading_thread = None  # Store the reading thread

# Create a Tkinter window
app = tk.Tk()
app.title("BQ25798 registers")  # Set the window title
app.geometry("1024x768")  # Set the window size to 1024x768
app.resizable(False, False)  # Make the window non-resizable

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
    app.after(100, process_queue)  # Start processing the data queue

# Function to process the data queue
def process_queue():
    try:
        data = data_queue.get_nowait()  # Get data from the queue
        if data.startswith("Serial Error"):
            show_error(data)
        else:
            update_entry_fields(data)
        app.after(100, process_queue)  # Continue processing the data queue
    except queue.Empty:
        app.after(100, process_queue)  # Queue is empty, continue processing

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
    app.quit()  # Quit the main application loop

# Function to display an error popup
def show_error(message):
    messagebox.showerror("Error", message)
    print(f"Error: {message}")  # Print the error to the command prompt

# Create a custom font size
custom_font_size = 12

# Create text entry fields with the default value "0x00" and a custom font size
num_fields_per_row = 10
num_rows = 6
default_value = "0x00"
entry_field_width = 6  # Width of 6 characters
entry_field_height = 1  # Height of 1 character (slightly taller than the default font size)
custom_font = ("Arial", custom_font_size)

# Create a frame for the text entry fields
entry_frame = tk.Frame(app)
entry_frame.grid(row=0, column=0, sticky='nsew')

# Create controller status entry fields in the first row
for i in range(3):
    entry_field = Text(entry_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
    entry_field.insert(1.0, default_value)  # Set the default value
    entry_field.grid(row=0, column=i, padx=5, pady=5, sticky='nsew')
    entry_fields.insert(i, entry_field)

# Create bq25798 register fields in the second row
for i in range(num_rows):
    for j in range(num_fields_per_row):
        field_num = i * num_fields_per_row + j
        if field_num >= 57:
            break
        entry_field = Text(entry_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        entry_field.insert(1.0, default_value)  # Set the default value
        entry_field.grid(row=i + 1, column=j, padx=5, pady=5, sticky='nsew')
        entry_fields.append(entry_field)

# Create a frame for the buttons and place it at the bottom of the window
button_frame = tk.Frame(app)
button_frame.grid(row=1, column=0, sticky='nsew')

# Create a button to start reading data
start_button = tk.Button(button_frame, text="Start Reading", command=start_reading)
start_button.grid(row=0, column=0, padx=5, pady=5)

# Create a button to stop reading data
stop_button = tk.Button(button_frame, text="Stop Reading", command=stop_reading)
stop_button.grid(row=0, column=1, padx=5, pady=5)

# Create an exit button to close the application
exit_button = tk.Button(button_frame, text="Exit", command=exit_app)
exit_button.grid(row=0, column=2, padx=5, pady=5)

# Configure grid weights to allow resizing
entry_frame.grid_rowconfigure(0, weight=1)
entry_frame.grid_columnconfigure(0, weight=1)
button_frame.grid_rowconfigure(0, weight=1)
button_frame.grid_columnconfigure(0, weight=1)

# Create a queue for communication between threads
data_queue = queue.Queue()

app.mainloop()
