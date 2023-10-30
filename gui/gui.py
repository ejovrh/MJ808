import threading
import queue
from tkinter import * 
from tkinter import messagebox
import serial
import time

from strings import * # local strings.py file containing arrays of strings

NumberofDataTokens = 60
clicked_field = 11

data_queue = queue.Queue()  # Initialize a queue for data processing

default_value = "n/a"  # default value
entry_field_width = 6   #
entry_field_height = 0  #
custom_font = ("Arial", 10)  # font to be used
fields = {}  # dictionary containing GUI widgets
data_buffer = ""    # data buffer for serial read
current_values:int = [default_value] * NumberofDataTokens
last_values:int = [default_value] * NumberofDataTokens
last_update_times = [time.time()] * NumberofDataTokens
last_change_time = [0] * NumberofDataTokens
          
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

        self.tooltip = Toplevel(self.widget)
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
    root = Tk()

root.title("BQ25798 registers")

# Create frames for organizing the GUI elements
top_frame = Frame(root, bg='gray', width=1024, height=50, pady=3)
bq25798_frame = Frame(root, bg='gray', width=1024, pady=3)
status_frame = Frame(root, bg='gray', width=1024, pady=3)
button_frame = Frame(root, bg='gray', width=1024, pady=3)
byte_frame = Frame(root, bg='gray', width=1024, height=200)
bottom_frame = Frame(root, bg='gray', width=1024, height=50, pady=3)

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
    global data_buffer
    data_buffer += data # Append the received data to the buffer

    field_num = 0 # start at data & field zero

    while '\n' in data_buffer: # Keep processing as long as there are newline characters in the buffer
        data_lines, data_buffer = data_buffer.split('\n', 1) # Split the buffer by newline characters
        values = data_lines.strip().split() # split line into space-seperated values

        if len(values) != NumberofDataTokens: # if values is longer than we have data
            continue    #...get out
        
        for field_num, value in enumerate(values):  # loop over values with field_num and value
            current_time = time.time() # record current time
            hexvalue:str = value    # save the original value as hex
            decimalvalue = int(value, 16)  # convert value to integer

            if fptr[field_num] != retnone:   # only if the function pointer is not retval
                current_values[field_num] = fptr[field_num](decimalvalue, register_offset[field_num], register_step_size[field_num])  # execute whatever the fuction pointer points to
            else:   # if it is retval
                current_values[field_num] = hexvalue   # display the hex value (registers with bitfie)

            key = str("device_values"+str(field_num))   # in DS p.57 - string "REG" appended with column "Offset": e.g. REG00
            fields[key].delete(1.0, END)
            fields[key].insert(1.0, f"{current_values[field_num]}{register_unit[field_num]}\n")

            # # FIXME
            # if current_values[i] != last_values[clicked_field]:
            #     if fptr_hover[i] != retnone:
            #         fptr_hover[i](str(value), register_offset[i], register_step_size[i])

            if current_values[field_num] != last_values[field_num]:
                # if fields[key].cget("bg") != "yellow":
                fields[key].config(bg='yellow')
                last_change_time[field_num] = current_time
                last_values[field_num] = current_values[field_num]
            else:
                if current_time - last_change_time[field_num] >= 5:
                    fields[key].config(bg='white')

            field_num +=1

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

previous = 0    # holds the previous state
def label_click(event, num):
    global previous
    key = str("bq25798_register_short_name"+str(previous))
    fields[key].config(bg="gray")
    
    key = str("bq25798_register_short_name"+str(num))

    if fields[key]:
        fields[key].config(bg="red")

    previous = num # save state

    return None

def dectostr(in_val:int) -> str:
    decimal = int(in_val, 16)  # Convert in_val to an integer
    binary_str = bin(decimal)[2:]  # Convert the integer to a binary string and remove the '0b' prefix
    binary_str = binary_str.zfill(8) # Pad the binary string to 8 characters with leading zeros if needed"
    return binary_str

def populate_8_bitfields(in_name:str, in_val:int, description:str, bit_set, bit_unset:str):
    binary_str = dectostr(in_val)

    for i in range(8):
        # bit name
        key = str("bit_field_name"+str(i)) # in DS pp.59 - column "Field": tree text byte register description
        
        if in_name[i] == "RESERVED":
                fields[key].config(bg='gray')
        else:
                fields[key].config(bg='white')


        fields[key].delete(1.0, END)  # Clear the bit_field
        fields[key].insert(1.0, in_name[i])
        ###

        # bit value
        key = str("bit_field"+str(i)) # bit value according to column "Description"
        fields[key].delete(1.0, END)  # Clear the bit_field

        if binary_str[i] == 1:
            bit_val = bit_set
        else:
            bit_val = bit_unset

        if bit_val[i] == "n/a":
                fields[key].config(bg='gray')
        else:
                fields[key].config(bg='white')

        fields[key].insert(1.0, bit_val[i])
        ###  

        # bit description
        key = str("bit_field_description"+str(i)) # in DS pp.59 - column "Description": tree text byte register description

        if description[i] == "RESERVED":
                fields[key].config(bg='gray')
        else:
                fields[key].config(bg='white')

        fields[key].delete(1.0, END)  # Clear the bit_field
        fields[key].insert(1.0, description[i])
        ###

def apply_mask(in_byte:int, in_mask:int, rsh:int) -> int:
    retval:int = int(in_byte,16) & int(in_mask, 16)
    retval = retval >> rsh
    return retval

def REG08(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg08_bit_names, in_val, reg08_description, reg08_bits_set, reg08_bits_unset) 

    vbat_lowv = apply_mask(in_val, '0xc0', 6)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg08_bits_VBAT_LOWV[vbat_lowv]) # insert bitfield value

    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, f"{RegToVal(apply_mask(in_val, '0x3f', 0), 0, 40)} A\n") # insert bitfield value # TODO

    return in_val

def REG09(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg09_bit_names, in_val, reg09_description, reg09_bits_set, reg09_bits_unset) 
    
    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, f"{RegToVal(apply_mask(in_val, '0x3f', 0), 0, 40)} A\n") # insert bitfield value
    return in_val

def REG0A(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg0a_bit_names, in_val, reg0a_description, reg0a_bits_set, reg0a_bits_unset) 

    cell = apply_mask(in_val, '0xc0', 6)
    trechg = apply_mask(in_val, '0x30', 4)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg0a_bits_CELL[cell]) # insert bitfield value

    key = str("bit_field4") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg0a_bits_TRECHG[trechg]) # insert bitfield value

    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, f"{RegToVal(apply_mask(in_val, '0x3f', 0), 50, 50)} V\n") # insert bitfield value # TODO

    return in_val

def REG0D(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg0d_bit_names, in_val, reg0d_description, reg0d_bits_set, reg0d_bits_unset) 

    # individual bitfields
    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, f"{RegToVal(apply_mask(in_val, '0x7f', 0), 0, 40)} A\n") # insert bitfield value # TODO

    return in_val

def REG0E(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg0e_bit_names, in_val, reg0e_description, reg0e_bits_set, reg0e_bits_unset) 

    topoff = apply_mask(in_val, '0xC0', 6)
    chg_tmr = apply_mask(in_val, '0x06', 1)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg0e_bits_TOPOFF_TMR[topoff]) # insert bitfield value

    key = str("bit_field1") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg0e_bits_CHG_TMR[chg_tmr]) # insert bitfield value

    return in_val

def REG0F(in_val:int, _ignore1:int, _ignore2:int):
    populate_8_bitfields(reg0f_bit_names, in_val, reg0f_description, reg0f_bits_set, reg0f_bits_unset)
    return in_val

def REG10(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg10_bit_names, in_val, reg10_description, reg10_bits_set, reg10_bits_unset) 

    vbus_backup = apply_mask(in_val, '0xC0', 6)
    vac_ovp = apply_mask(in_val, '0x30', 4)
    watchdog = apply_mask(in_val, '0x07', 0)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg10_bits_VBUS_BACKUP[vbus_backup]) # insert bitfield value

    key = str("bit_field4") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg10_bits_VAC_OVP[vac_ovp]) # insert bitfield value

    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg10_bits_WATCHDOG[watchdog]) # insert bitfield value

    return in_val

def REG11(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg11_bit_names, in_val, reg11_description, reg11_bits_set, reg11_bits_unset) 

    sdrv = apply_mask(in_val, '0x06', 0)

    # individual bitfields
    key = str("bit_field1") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg11_bits_SDRV_CTRL[sdrv]) # insert bitfield value

    return in_val

def REG12(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg12_bit_names, in_val, reg12_description, reg12_bits_set, reg12_bits_unset)
    return in_val

def REG13(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg13_bit_names, in_val, reg13_description, reg13_bits_set, reg13_bits_unset)
    return in_val

def REG14(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg14_bit_names, in_val, reg14_description, reg14_bits_set, reg14_bits_unset) 

    ibat = apply_mask(in_val, '0x18', 3)

    # individual bitfields
    key = str("bit_field3") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg14_bits_IBAT_REG[ibat]) # insert bitfield value

    return in_val

def REG15(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg15_bit_names, in_val, reg15_description, reg15_bits_set, reg15_bits_unset) 

    voc_pct = apply_mask(in_val, '0xe0', 5)
    voc_dly = apply_mask(in_val, '0x18', 3)
    voc_rate = apply_mask(in_val, '0x06', 1)

    # individual bitfields
    key = str("bit_field5") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg15_bits_VOC_PCT[voc_pct]) # insert bitfield value
    key = str("bit_field3") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg15_bits_VOC_DLY[voc_dly]) # insert bitfield value
    key = str("bit_field1") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg15_bits_VOC_RATE[voc_rate]) # insert bitfield value

    return in_val

def REG16(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg16_bit_names, in_val, reg16_description, reg16_bits_set, reg16_bits_unset) 

    treg = apply_mask(in_val, '0xc0', 6)
    tshut = apply_mask(in_val, '0x30', 4)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg16_bits_TREG[treg]) # insert bitfield value
    key = str("bit_field4") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg16_bits_TSHUT[tshut]) # insert bitfield value

    return in_val

def REG17(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg17_bit_names, in_val, reg17_description, reg17_bits_set, reg17_bits_unset) 

    vset = apply_mask(in_val, '0xe0', 5)
    iseth = apply_mask(in_val, '0x18', 3)
    isetc = apply_mask(in_val, '0x06', 1)

    # individual bitfields
    key = str("bit_field5") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg17_bits_JEITA_VSET[vset]) # insert bitfield value
    key = str("bit_field3") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg17_bits_JEITA_ISETH[iseth]) # insert bitfield value
    key = str("bit_field1") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg17_bits_JEITA_ISETC[isetc]) # insert bitfield value

    return in_val

def REG18(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg18_bit_names, in_val, reg18_description, reg18_bits_set, reg18_bits_unset) 

    cool = apply_mask(in_val, '0xe0', 6)
    warm = apply_mask(in_val, '0x18', 4)
    hot = apply_mask(in_val, '0x06', 2)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg18_bits_TS_COOL[cool]) # insert bitfield value
    key = str("bit_field4") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg18_bits_TS_WARM[warm]) # insert bitfield value
    key = str("bit_field2") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg18_bits_BHOT[hot]) # insert bitfield value

    return in_val

def REG19(in_val, _ignore1, _ignore2):
    return in_val

def REG1B(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg1b_bit_names, in_val, reg1b_description, reg1b_bits_set, reg1b_bits_unset)
    return in_val

def REG1C(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg1c_bit_names, in_val, reg1c_description, reg1c_bits_set, reg10_bits_unset) 

    chg = apply_mask(in_val, '0xe0', 5)
    bus = apply_mask(in_val, '0x1e', 1)

    # individual bitfields
    key = str("bit_field5") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg1c_bits_CHG_STAT[chg]) # insert bitfield value
    key = str("bit_field1") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg1c_bits_VBUS_STAT[bus]) # insert bitfield value

    return in_val

def REG1D(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg1d_bit_names, in_val, reg1d_description, reg1d_bits_set, reg1d_bits_unset) 

    ico = apply_mask(in_val, '0xc0', 6)

    # individual bitfields
    key = str("bit_field6") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg1d_bits_ICO_STAT[ico]) # insert bitfield value

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
    populate_8_bitfields(reg21_bit_names, in_val, reg21_description, reg21_bits_set, reg21_bits_unset) 

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
    populate_8_bitfields(reg2e_bit_names, in_val, reg2e_description, reg2e_bits_set, reg2e_bits_unset) 

    adc = apply_mask(in_val, '0x30', 4)

    # individual bitfields
    key = str("bit_field4") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg2e_bits_ADC_SAMPLE[adc]) # insert bitfield value

    return in_val

def REG2F(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg2f_bit_names, in_val, reg2f_description, reg2f_bits_set, reg2f_bits_unset)
    return in_val

def REG30(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg30_bit_names, in_val, reg30_description, reg30_bits_set, reg30_bits_unset)
    return in_val

def REG47(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg47_bit_names, in_val, reg47_description, reg47_bits_set, reg47_bits_unset) 

    dp = apply_mask(in_val, '0xe0', 5)
    dm = apply_mask(in_val, '0x1c', 2)

    # individual bitfields
    key = str("bit_field5") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg47_bits_DPLUS_DAC[dp]) # insert bitfield value
    key = str("bit_field2") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg47_bits_DMINUS_DAC[dm]) # insert bitfield value

    return in_val

def REG48(in_val, _ignore1, _ignore2):
    populate_8_bitfields(reg48_bit_names, in_val, reg48_description, reg48_bits_set, reg48_bits_unset) 

    pn = apply_mask(in_val, '0x38', 3)
    rev = apply_mask(in_val, '0x07', 0)

    # individual bitfields
    key = str("bit_field3") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg48_bits_PN[pn]) # insert bitfield value
    key = str("bit_field0") # create key
    fields[key].delete(1.0, END)  # Clear the bit_field
    fields[key].insert(1.0, reg48_bits_DEV_REV[rev]) # insert bitfield value

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
                retnone, retnone, retnone, retnone, retnone, REG47, REG48, retnone, retnone, retnone    # REG3D to REG48, along with PG, IRQ, STAT
            ]

# create bq25798 register labels and fields
field_num = 0
for i in range(6):  # 6 rows
    for j in range(10): # 10 register values per row
        if field_num >= 57: # 56 - number of bq2798 registers
            break

        # create label with register short name
        key = str("bq25798_register_short_name"+str(field_num))   # in DS p.57 - string "REG" appended with column "Offset": e.g. REG00
        fields[key] = Label(bq25798_frame, text=register_name[field_num])
        fields[key].config(bg='gray')
        fields[key].grid(row=i, column=2*j, padx=2, pady=5, sticky="W")
        tooltip = ToolTip(fields[key], register_description[field_num]) # Add tooltips to the labels


        key = str("device_values"+str(field_num))   # in DS p.57 - string "REG" appended with column "Offset": e.g. REG00
        fields[key] = Text(bq25798_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
        fields[key].insert(1.0, str(default_value))
        fields[key].grid(row=i, column=2*j + 1, padx=2, pady=5)
        fields[key].bind("<FocusIn>", lambda event, num=field_num: on_entry_field_click(event, num)) # Bind the click event to the entry field
        fields[key].bind("<FocusIn>", lambda event, num=field_num: on_entry_field_click(event, num)) # Bind the click event to the entry field
        fields[key].bind("<Button-1>",lambda event, num=field_num: label_click(event, num))  # Bind the click event to the label

        field_num +=1 

        
# create charger status labels and fields
for i in range(3):
    # create label with register short name
    key = str("device_status_short_name"+str(field_num))   # in DS p.57 - string "REG" appended with column "Offset": e.g. REG00
    fields[key] = Label(status_frame, text=register_name[field_num])
    fields[key].config(bg='gray')
    fields[key].grid(row=0, column=2 * i, padx=2, pady=5, sticky="e")
    tooltip = ToolTip(fields[key], register_description[field_num])


    key = str("device_values"+str(field_num))
    fields[key] = Text(status_frame, width=entry_field_width, height=entry_field_height, font=custom_font)
    fields[key].insert(1.0, default_value)
    fields[key].grid(row=0, column=2 * i + 1, padx=2, pady=5)
    
    field_num +=1 

# Create buttons for controlling the application
start_button = Button(button_frame, text="Start Reading", command=start_reading)
stop_button = Button(button_frame, text="Stop Reading", command=stop_reading)
exit_button = Button(button_frame, text="Exit", command=exit_app)

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
for i in range(7, -1, -1):
    key = str("bit_number"+str(i))   # in DS pp.59 - column "Bit": bit0 to bit7`
    label_text = "bit" + str(i)
    fields[key] = Label(byte_frame, width=4, text=label_text)
    fields[key].config(bg='gray')
    fields[key].grid(row=2*j+1, column=0, padx=5, pady=5, sticky='W')   

    key = str("bit_field_name"+str(i)) # in DS pp.59 - column "Field": tree text byte register description
    fields[key] = Text(byte_frame, width=22, height=entry_field_height, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=2*j+1, column=1, padx=2, sticky='W')

    key = str("bit_field"+str(i)) # bit value according to column "Description"
    fields[key] = Text(byte_frame, width=80, height=entry_field_height, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=2*j+1, column=1, padx=2)

    key = str("bit_field_description"+str(i)) # in DS pp.59 - column "Description": tree text byte register description
    fields[key] = Text(byte_frame, width=125, height=entry_field_height, font=custom_font)    # value...
    fields[key].insert(1.0, "")
    fields[key].grid(row=2*j+2, column=1, padx=2, sticky='W')
    j += 1

# Place buttons in the button frame
start_button.grid(row=0, column=1, padx=5, pady=5, sticky='W')
stop_button.grid(row=0, column=1, padx=5, pady=5)
exit_button.grid(row=0, column=1, padx=5, pady=5, sticky='E')

# Start the Tkinter main loop
root.mainloop()
