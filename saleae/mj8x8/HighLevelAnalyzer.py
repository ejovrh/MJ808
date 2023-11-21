from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting

PRIORITY_HIGH = 0x00 # CAN frame bit 15
PRIORITY_LOW = 0x80

UNICAST = 0x00 # CAN frame bit 14
BROADCAST = 0x40

SENDER_DEVICE_MASK = 0x3C # CAN frame bits 13:10 - the sender of a message

class Hla(HighLevelAnalyzer): # High level analyzers must subclass the HighLevelAnalyzer class
    # my_string_setting = StringSetting() # List of settings that a user can set for this High Level Analyzer
    # my_number_setting = NumberSetting(min_value=0, max_value=100) # List of settings that a user can set for this High Level Analyzer
    # my_choices_setting = ChoicesSetting(choices=('A', 'B')) # List of settings that a user can set for this High Level Analyzer

    my_mj8x8_devices = {
                        '0x0': '0A', # 0A
                        '0x1': '0B', # 0B
                        '0x2': '0C', # 0C
                        '0x3': 'MJ828', # 0D 
                        '0x4': 'MJ838', # 1A
                        '0x5': '1B', # 1B
                        '0x6': '1C', # 1C
                        '0x7': '1D', # 1D
                        '0x8': 'MJ808', # 2A
                        '0x9': 'MJ818', # 2B
                        '0xa': '2A', # 2C
                        '0xb': '2D', # 2D
                        '0xc': '3A', # 3A
                        '0xd': '3B', # 3B
                        '0xe': '3C', # 3C
                        '0xf': '3D', # 3D
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
                    'MJ808': { 'format': '{{data.description}}' },
                    'MJ818': { 'format': '{{data.description}}' },
                    'MJ828': { 'format': '{{data.description}}' },
                    'MJ838': { 'format': '{{data.description}}' },
                    'DLC': { 'format': '{{data.description}}' },
                    'HeartBeat': { 'format': '{{data.description}}' },
    }

    def __init__(self):
        pass

    def get_capabilities(self):
        pass

    def set_settings(self, settings):
        pass

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
                cast = 'brd'
            else:
                cast = 'uni'

            sender_hex_id = hex( (self.my_can_message['identifier'] & SENDER_DEVICE_MASK) >> 2 ) # mask out all but bits13:10
            frame_type = self.my_mj8x8_devices[sender_hex_id] # record sender name as a frame type
            return AnalyzerFrame(frame_type, frame.start_time, frame.end_time, { 'description': '{} {} {}'.format(priority, cast, frame_type) })


        if frame.type == 'control_field':
            self.my_can_message['dlc'] = frame.data['num_data_bytes']
            return AnalyzerFrame('DLC', frame.start_time, frame.end_time, { 'description': '{} Bytes'.format(self.my_can_message['dlc']) })


        if frame.type == 'data_field':
            self.my_can_message['data'][self.data_array_index] = frame.data['data'].hex()
            self.data_array_index += 1

            if self.data_array_index == 1: # 1st data frame
                if self.my_can_message['data'][0] == '00': # HeartBeat message
                    return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'HeartBeat: ' })
        
            if self.data_array_index == 2: # 2nd data frame
                if self.my_can_message['data'][0] == '00': # was preceeded by a HeartBeat message
                    if self.my_can_message['data'][1] == '00': # idle device
                        return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': 'idle' })
                    
                    if self.my_can_message['data'][1] == '01': # idle device
                        return AnalyzerFrame('HeartBeat', frame.start_time, frame.end_time, { 'description': '01' })
                    

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
            return AnalyzerFrame(frame_type, frame.start_time, frame.end_time, data)