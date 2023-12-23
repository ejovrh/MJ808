'''
abstract: 
  this HLA analyzes mj8x8 CAN data streams

core components:
    HLA - HighLevelAnalyzer.py - this file
    mj8x8_commands.h - C header for mj8x8 command defines - see MJ8X8_HEADER_PATH
    device header file(s) - C headers for concrete devices - see DEVICE_HEADER_PATH


principle of operation: 
    interpret two kinds of frames: broadcast or unicast
    broadcast:
        1. in __init__: parse mj8x8_commands.h for possible devices as defined in "typedef union {} device_t". 
            populate a super dictionary with it (device hex ID and name)
        2. in __init__: find header files in DEVICE_HEADER_PATH and decode e.g "typedef union mj838_activity_t" into activities
            populate earlier super dict. with this info
        3. in decode(): loop over each bit of the HeartBeat activity byte and determnine which bits are set -> indicate that activity by building an analyzer frame

    unicast: TODO        
'''

from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting
import re
import os

# standard CAN ID frame
PRIORITY_HIGH = 0x000
PRIORITY_LOW = 0x200

MJ8X8_HEADER_PATH = r'C:\\Users\\hrvoje\\Documents\\vsite\\MJ808\\mj8x8\\Core\\Inc\\mj8x8'
DEVICE_HEADER_PATH = r'C:\\Users\\hrvoje\\Documents\\vsite\\MJ808\\mj8x8\\Core\\Inc'

UNICAST = 0x000
BROADCAST = 0x100
    
SENDER_DEVICE_MASK = 0x3C # CAN frame bits 13:10 - the sender of a message

CAST_CHOICES = { # dictionary for dropdown choices
                'all': 'all',
                'unicast': 'uni',
                'broadcast': 'brd',
}

class Hla(HighLevelAnalyzer): # High level analyzers must subclass the HighLevelAnalyzer class
    FlagDisplay = 0 # flag to display a frame or not
    Cast = '' # broadcast or unicast
    my_string_setting = StringSetting() # List of settings that a user can set for this High Level Analyzer
    my_choices_setting = ChoicesSetting(label='cast', choices=CAST_CHOICES.keys()) # List of settings that a user can set for this High Level Analyzer
    data_array_index = 0 # used to fill the data_array upon data frame reception
    data_array = [0] * 8 # fill with 0's

    my_can_message = {  # Define a CAN message as a dictionary
                        'identifier': 0, # raw Identifier, either 11 bit or 29 bit
                        'extended': False, # (optional) Indicates that this identifier is a 29 bit extended identifier. This key is not present on regular 11 bit identifiers
                        'remote': False, # (optional) Present and true for remote frames
                        'dlc': 0, # Data length code (number of bytes in the payload)
                        'data': data_array, # CAN Payload, 8 bytes
                        'crc': 0, # 16bit CRC value
                        'ack': 0, # True when an ACK was present
                        'hexID': 0, # translated CAN Identifier 
    }

    # An optional list of types this analyzer produces, providing a way to customize the way frames are displayed in Logic 2.
    result_types = {
                    'mj808': { 'format': '{{data.description}}' },
                    'mj818': { 'format': '{{data.description}}' },
                    'mj828': { 'format': '{{data.description}}' },
                    'mj838': { 'format': '{{data.description}}' },
                    'mj514': { 'format': '{{data.description}}' },
                    'DLC': { 'format': '{{data.description}}' },
                    'HeartBeat': { 'format': '{{data.description}}' },
                    'activity': { 'format': '{{data.description}}' },
                    'can_error': { 'format': 'ERR' },
                    'ack_field': { 'format': 'ACK' },
   }

    def __init__(self):
        self.DynamicDeviceActivityDicts = {} # super-dictionary containing e.g. mj808 as key
        ''' example content:
        self.DynamicDeviceActivityDicts:
        {
            'mj_0': {'hexID': '0', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '0': 'mj_0', 
            'mj_1': {'hexID': '1', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '1': 'mj_1', 
            'mj_2': {'hexID': '2', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '2': 'mj_2', 
            'mj828': {'hexID': '3', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '3': 'mj828', 
            'mj838': {'hexID': '4', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '4': 'mj838', 
            'mj_5': {'hexID': '5', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '5': 'mj_5', 
            'mj_6': {'hexID': '6', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '6': 'mj_6', 
            'mj_7': {'hexID': '7', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '7': 'mj_7', 
            'mj808': {'hexID': '8', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '8': 'mj808', 
            'mj818': {'hexID': '9', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, '9': 'mj818', 
            'mj_10': {'hexID': 'a', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'a': 'mj_10', 
            'mj_11': {'hexID': 'b', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'b': 'mj_11', 
            'mj514': {'hexID': 'c', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'c': 'mj514', 
            'mj_13': {'hexID': 'd', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'd': 'mj_13', 
            'mj_14': {'hexID': 'e', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'e': 'mj_14', 
            'mj_15': {'hexID': 'f', 0: '', 1: '', 2: '', 3: '', 4: '', 5: '', 6: '', 7: ''}, 'f': 'mj_15'
        }

        '''

        self.Parsemj8x8HeaderForDevices(MJ8X8_HEADER_PATH) # parse mj8x8_commands.h for devices
        self.ParseDeviceHeaderForActivity(DEVICE_HEADER_PATH) # parse device header file for mjxxx_activity_t struct and deduce device activity options
        return None

    def get_capabilities(self):
        pass

    def set_settings(self, settings):
        pass

    def PopulateDynamicDeviceActivityDicts(self, hexID, device, actID, act): # populates a data structure with device name (e.g. mj808) as key
        # Check if the device already exists as an dictionary element
        if device not in self.DynamicDeviceActivityDicts:
            self.DynamicDeviceActivityDicts[device] = {}
        
        # Add key-value pair to the dynamically created variable
        self.DynamicDeviceActivityDicts[hexID] = device # hexID to device mapping
        
        self.DynamicDeviceActivityDicts[device]['hexID'] = hexID # device to hexID mapping
        # self.DynamicDeviceActivityDicts[device]['name'] = device # 

        self.DynamicDeviceActivityDicts[device][actID] = act

    def ParseDeviceHeaderForActivity(self, directory): # parses device header file for mjxxx_activity_t structure and deduces activity
        device_folders = []

        # Find folders that match the pattern 'mjXXX'
        for folder_name in os.listdir(directory):
            if re.match(r'mj\d{3}', folder_name):
                device_folders.append(folder_name)

        if not device_folders:
            print("No matching folders found.")
            return

        # Iterate through each identified folder
        for device in device_folders:
            # Build the path to device.h in each folder
            device_header_path = os.path.join(directory, device, f"{device}.h")

            # Check if the device header file exists
            if not os.path.isfile(device_header_path):
                continue

            # at this point we have a list of existing devices, i.e. existing header files (e.g. mj808.h and so on)
            hexID = self.DynamicDeviceActivityDicts[device]['hexID']
            
            # Read and parse the device header file
            with open(device_header_path, 'r') as device_header:
                inside_union = False
                union_lines = []

                # Iterate through lines in the file - go over heach found device
                for line in device_header:
                    # Start capturing lines inside the union
                    if 'struct' in line: # string struct is found
                        inside_union = True

                    # Stop capturing lines inside the union
                    if inside_union and '};' in line: # string } marks the end of the C struct definition
                        inside_union = False
                        break

                    # Capture lines inside the union
                    if inside_union and line.strip().startswith('uint8_t'): # lines of the C struct definition and beginning with  uint8_t 
                        union_lines.append(line.strip())

                activity_iterator = 0 # start at bit 0

                # Parse lines to extract comments
                for union_line in union_lines: # now go over each line of the activity_t C struct
                    # Use regular expression to find comments between '//' in each line
                    activity = re.findall(r'\/\/\s*(.*?)\s*\/\/', union_line) # e.g. '// LED //' or '// CAN //'

                    self.PopulateDynamicDeviceActivityDicts(hexID, device, activity_iterator, activity) # store them in the dict of dicts.
                    activity_iterator += 1 # 8 bits in total

    def Parsemj8x8HeaderForDevices(self, directory): # parses device header file for mjxxx_activity_t structure and deduces activity
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
                        for i in range(8):
                            self.PopulateDynamicDeviceActivityDicts(hex_id, device_name, i, '')

        # print("self.DynamicDeviceActivityDicts:\n", self.DynamicDeviceActivityDicts)

    def DecodeActivityByte(self, in_byte, device): # decodes the HeartBeat act byte according to DynamicDeviceActivityDicts[]
        return_string = ""
        in_byte_int = int(in_byte, 16)

        # Iterate over the bits in the byte
        for i in range(8):
            # Check if the bit is set in the input byte
            if (in_byte_int & (1 << i)) != 0:
                # Check if the corresponding bit index exists in the device dictionary
                return_string += ''.join(self.DynamicDeviceActivityDicts[device][i])
                return_string += ' '

        return return_string
    
    def decode(self, frame: AnalyzerFrame):
        if frame.type == 'identifier_field':
            self.data_array_index = 0 # mark beginning of data capture
            self.my_can_message['identifier'] = frame.data['identifier']
            # self.my_can_message['extended'] = frame.data['extended']
            # self.my_can_message['remote_frame'] = frame.data['remote_frame']

            if (self.my_can_message['identifier'] & PRIORITY_LOW) == PRIORITY_LOW: # determine priority from CAN frame
                priority = 'low'
            else:
                priority = 'HIGH'

            if (self.my_can_message['identifier'] & BROADCAST) == BROADCAST: # determine recipients - unicast or broadcast
                self.Cast = 'brd'
            else:
                self.Cast = 'uni'

            choice = CAST_CHOICES.get(self.my_choices_setting)

            if (choice == self.Cast) or (choice == 'all'):
                self.my_can_message['hexID'] = hex( (self.my_can_message['identifier'])) # 
                hexID = int(self.my_can_message['hexID'], 16)
                sender = (hexID & 0x00F0 ) >> 4
                recipient = hexID & 0x0F

                sender = self.DynamicDeviceActivityDicts[str(sender)]
                recipient = self.DynamicDeviceActivityDicts[str(recipient)]
                
                if recipient == 'mj_0':
                    recipient = 'ALL'
                
                self.FlagDisplay = 1
            else:
                self.FlagDisplay = 0

            if self.FlagDisplay:
                return AnalyzerFrame(sender, frame.start_time, frame.end_time, { 'description': '{}: {} -> {}'.format(priority, sender, recipient) })
            else:
                return None

        if frame.type == 'control_field':
            self.my_can_message['dlc'] = frame.data['num_data_bytes']

            if self.FlagDisplay:
                return AnalyzerFrame('DLC', frame.start_time, frame.end_time, { 'description': '{} B'.format(self.my_can_message['dlc']) })
            else: 
                return None

        if frame.type == 'data_field':
            self.my_can_message['data'][self.data_array_index] = frame.data['data'].hex()
            self.data_array_index += 1

            if self.FlagDisplay == 0:
                return None
            if self.Cast == 'brd': # the only broadcast frames are HeartBeats; unicast are all commands
                if self.data_array_index == 1: # 1st data frame
                    if self.my_can_message['data'][0] == '00': # HeartBeat message
                        return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'HeartBeat' })
            
                if self.data_array_index == 2: # 2nd data frame
                    if self.my_can_message['data'][0] == '00': # was preceeded by a HeartBeat message
                        if self.my_can_message['data'][1] == '00': # just display 'idle'
                            return AnalyzerFrame('activity', frame.start_time, frame.end_time, { 'description': 'idle' })
                        else: # analyze activity and display it
                            sender = str((int(self.my_can_message['hexID'], 16) & 0x00F0 ) >> 4)
                            device = self.DynamicDeviceActivityDicts[sender]
                            in_byte = self.my_can_message['data'][1]
                            text = self.DecodeActivityByte(in_byte, device)
                            return AnalyzerFrame('activity', frame.start_time, frame.end_time, { 'description': text  })
                        
            if self.data_array_index == 3: # 3rd data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

            if self.data_array_index == 4: # 4th data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

            if self.data_array_index == 5: # 5th data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

            if self.data_array_index == 6: # 6th data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

            if self.data_array_index == 7: # 7th data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

            if self.data_array_index == 8: # 8th data frame
                return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'not implemented' })

        if frame.type == 'crc_field':
            self.my_can_message['crc'] = frame.data['crc']
            return None

        if frame.type == 'ack_field':
            self.my_can_message['ack'] = frame.data['ack']

            # keep the ack frame hidden for now
            # if self.FlagDisplay:
            #     return AnalyzerFrame(frame.type, frame.start_time, frame.end_time, None)
            # else: 
            #     return None
            return None
                
        if frame.type == 'can_error':
            self.my_can_message['ack'] = 'nack'

            if self.FlagDisplay:
                return AnalyzerFrame(frame.type, frame.start_time, frame.end_time, None)
            else: 
                return None