from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting
import re
import os

PRIORITY_HIGH = 0x00 # CAN frame bit 15
PRIORITY_LOW = 0x80

INC_DIRECTORY_PATH = r'C:\\Users\\hrvoje\\Documents\\vsite\\MJ808\\mj8x8\\Core\\Inc'

UNICAST = 0x00 # CAN frame bit 14
BROADCAST = 0x40
    
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
    devices = {}

    my_mj8x8_devices = {
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

    data_array_index = 0 # used to fill the data_array upon data frame reception
    data_array = [0] * 8 # fill with 0's
    my_can_message = {  # Define a CAN message as a dictionary
                        'identifier': 0, # Identifier, either 11 bit or 29 bit
                        'extended': False, # (optional) Indicates that this identifier is a 29 bit extended identifier. This key is not present on regular 11 bit identifiers
                        'remote': False, # (optional) Present and true for remote frames
                        'dlc': 0, # Data length code (number of bytes in the payload)
                        'data': data_array, # CAN Payload, 8 bytes
                        'crc': 0, # 16bit CRC value
                        'ack': 0 # True when an ACK was present
    }

    # An optional list of types this analyzer produces, providing a way to customize the way frames are displayed in Logic 2.
    result_types = {
                    'mj808': { 'format': '{{data.description}}' },
                    'mj818': { 'format': '{{data.description}}' },
                    'mj828': { 'format': '{{data.description}}' },
                    'mj838': { 'format': '{{data.description}}' },
                    'DLC': { 'format': '{{data.description}}' },
                    'HeartBeat': { 'format': '{{data.description}}' },
                    'activity': { 'format': '{{data.description}}' },
                    'can_error': { 'format': 'ERR' },
                    'ack_field': { 'format': 'ACK' },
   }

    def __init__(self):
        pass

    def get_capabilities(self):
        pass

    def set_settings(self, settings):
        pass

    def create_device(self, key, name, activity_data):
        device = {'name': name, 'activity': activity_data}
        self.devices[key] = device

    def parse_device_activity(self, directory):
        device_folders = []
        devices_dict = {} # a dictionary of all the discovered devices
        devices_dict_iterator = 0 # iterator for the above

        # Find folders that match the pattern 'mjXXX'
        for folder_name in os.listdir(directory):
            if re.match(r'mj\d{3}', folder_name):
                device_folders.append(folder_name)

        if not device_folders:
            print("No matching folders found.")
            return

        # Iterate through each identified folder
        for device in device_folders:
            print("device: ", device)
            device_activity_dict = {}

            # Build the path to device.h in each folder
            device_header_path = os.path.join(directory, device, f"{device}.h")

            # Check if the device header file exists
            if os.path.isfile(device_header_path):
                devices_dict[devices_dict_iterator] = device
                devices_dict_iterator += 1
            else:
                continue

            # print("devices_dict: ", devices_dict) # device_dict is ok

            # Read and parse the device header file
            with open(device_header_path, 'r') as device_header:
                inside_union = False
                union_lines = []

                # Iterate through lines in the file
                for line in device_header:
                    # Start capturing lines inside the union
                    if 'typedef union' in line:
                        inside_union = True

                    # Stop capturing lines inside the union
                    if inside_union and 'include' in line:
                        inside_union = False
                        break

                    # Capture lines inside the union
                    if inside_union and line.strip().startswith('uint8_t'):
                        union_lines.append(line.strip())

                # Parse lines to extract comments
                for union_line in union_lines:
                    # Use regular expression to find comments between '//' in each line
                    comments = re.findall(r'\/\/\s*(.*?)\s*\/\/', union_line)
                    print("comments: ", comments)

                    # Store comments in the dictionary with keys starting from 0
                    # for i, comment in enumerate(comments):
                        # device_activity_dict[i] = comment.strip()
                        # device_dict[i] = comment

            # # Print the results for the current device
            # print(f"Device: {device}")
            # print("Device Activity Dictionary:")
            # for key, value in device_activity_dict.items():
            #     print(f"Key {key}: {value}")
            # print("\n" + "=" * 40 + "\n")
    
    def decode(self, frame: AnalyzerFrame):
        frame_type = 'frame.type'
        data = {'input_type': frame.type}

        if frame.type == 'identifier_field':
            self.data_array_index = 0 # mark beginning of data capture
            self.my_can_message['identifier'] = frame.data['identifier']
            # self.my_can_message['extended'] = frame.data['extended']
            # self.my_can_message['remote_frame'] = frame.data['remote_frame']

            if self.my_can_message['identifier'] & PRIORITY_LOW: # determine priority from CAN frame
                priority = 'low'
            else:
                priority = 'HIGH'

            if self.my_can_message['identifier'] & BROADCAST: # determine recipients
                self.Cast = 'brd'
            else:
                self.Cast = 'uni'

            choice =  CAST_CHOICES.get(self.my_choices_setting)

            if (choice == self.Cast) or (choice == 'all'):
                sender_hex_id = hex( (self.my_can_message['identifier'] & SENDER_DEVICE_MASK) >> 2 )[2:] # mask out all but bits13:10
                frame_type = self.my_mj8x8_devices[sender_hex_id] # record sender name as a frame type

                self.FlagDisplay = 1
            else:
                self.FlagDisplay = 0

            if self.FlagDisplay:
                return AnalyzerFrame(frame_type, frame.start_time, frame.end_time, { 'description': '{} {}'.format(priority, frame_type) })
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
            if self.Cast == 'brd': # only broadcast frames send out HeartBeats; unicast are all commands
                if self.data_array_index == 1: # 1st data frame
                    if self.my_can_message['data'][0] == '00': # HeartBeat message
                        return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'HB' })
            
                if self.data_array_index == 2: # 2nd data frame
                    if self.my_can_message['data'][0] == '00': # was preceeded by a HeartBeat message
                        if self.my_can_message['data'][1] == '00': # idle device
                            return AnalyzerFrame('activity', frame.start_time, frame.end_time, { 'description': 'idle' })
                        else:
                            self.parse_device_activity(INC_DIRECTORY_PATH)
                            return AnalyzerFrame('activity', frame.start_time, frame.end_time, { 'description': 'act' })
                        
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