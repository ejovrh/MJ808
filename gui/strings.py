register_name:str = ['REG00', 'REG01', 'REG03', 'REG05', 'REG06', 'REG08', 'REG09', 'REG0A', 'REG0B', 'REG0D',  # index 0 to 9
                 'REG0E', 'REG0F', 'REG10', 'REG11', 'REG12', 'REG13', 'REG14', 'REG15', 'REG16', 'REG17',      # index 10 to 19
                 'REG18', 'REG19', 'REG1B', 'REG1C', 'REG1D', 'REG1E', 'REG1F', 'REG20', 'REG21', 'REG22',      # index 20 to 29
                 'REG23', 'REG24', 'REG25', 'REG26', 'REG27', 'REG28', 'REG29', 'REG2A', 'REG2B', 'REG2C',      # index 30 to 39
                 'REG2D', 'REG2E', 'REG2F', 'REG30', 'REG31', 'REG33', 'REG35', 'REG37', 'REG39', 'REG3B',      # index 40 to 49
                 'REG3D', 'REG3F', 'REG41', 'REG43', 'REG45', 'REG47', 'REG48', 'PG', 'IRQ', 'STAT']            # index 50 to 59

register_description:str = [
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

register_unit:str = [
                "V", "V", "A", "V", "A", "", "", "", "V", "",   # REG00 to REG0D
                "", "", "", "", "", "", "", "", "", "",         # REG0E to REG17
                "", "A", "", "", "", "", "", "", "", "",        # REG18 to REG22
                "", "", "", "", "", "", "", "", "", "",         # REG23 to REG2C
                "", "", "", "", "A", "A", "V", "V", "V", "V",   # REG2D to REG3B
                "V", "%", "°C", "V", "V", "", "", "", "", "",   # REG3D to REG48, along with PG, IRQ, STAT
]

register_offset:int = [2500, 0, 0, 0, 0, 0, 0, 0, 2800, 0,  # REG00 to REG0D
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG0E to REG17
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG18 to REG22
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG23 to REG2C
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG2D to REG3B
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0         # REG3D to REG48, along with PG, IRQ, STAT
                   ]

register_step_size: int = [250, 10, 10, 100, 10, 40, 40, -1, 10, -1, # REG00 to REG0D
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG0E to REG17
                      -1, 10, -1, -1, -1, -1, -1, -1, -1, -1,   # REG18 to REG22
                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG23 to REG2C
                      -1, -1, -1, -1, 1, 1, 1, 1, 1, 1,         # REG2D to REG3B
                      1, 0.0976563, 0.5, 1, 1, -1, -1, -1, -1, -1   # REG3D to REG48, along with PG, IRQ, STAT
                      ]

reg0f_description = [
                    "Enables backup mode where OTG automatically engages when VBUS droops below voltage set in REG10",   # bit0
                    "Termination",   # bit1
                    "HI Z mode",   # bit2
                    "Force start input current optimizer",   # bit3
                    "Input Current Optimizer (ICO)",   # bit4
                    "Charger Enable Configuration",   # bit5
                    "Force a battery discharging current",   # bit6
                    "Enable the auto battery discharging during the battery OVP fault"   # bit7
                    ]

reg0f_bits_set = [
                  "Enable",   # bit0
                  "Enable (default)",   # bit1
                  "Enable",   # bit2
                  "Force ICO start",   # bit3
                  "Enable ICO",   # bit4
                  "Charge Enable (default)",   # bit5
                  "Force the charger to apply a discharging current on BAT regardless the battery OVP status",   # bit6
                  "The charger will apply a discharging current on BAT during battery OVP"   # bit7
                ]

reg0f_bits_unset = [
                  "Disable (default)",   # bit0
                  "Disable",   # bit1
                  "Disable (default)",   # bit2
                  "Do NOT force ICO (Default)",   # bit3
                  "Disable ICO (default)",   # bit4
                  "Charge Disable",   # bit5
                  "IDLE (default)",   # bit6
                  "The charger will NOT apply a discharging current on BAT during battery OVP"   # bit7
                ]

reg12_description = [
                  "Disable OOA in forward mode",   # bit0
                  "Disable OOA in OTG mode",   # bit1
                  "Disable BATFET LDO mode in pre-charge stage.",   # bit2
                  "When wake up the device from ship mode, how much time (tSM_EXIT) is required to pull low the QON pin.",   # bit3
                  "Disable PFM in forward mode",   # bit4
                  "Disable PFM in OTG mode",   # bit5
                  "OTG mode control",   # bit6
                  "When this bit is set, the charger will force both EN_ACDRV1=0 and EN_ACDRV2=0"   # bit7
                ]

reg12_bits_set = [
                  "Disable",   # bit0
                  "Disable",   # bit1
                  "Disable",   # bit2
                  "15ms",   # bit3
                  "Disable",   # bit4
                  "Disable",   # bit5
                  "OTG Enable",   # bit6
                  "force"   # bit7
                ]

reg12_bits_unset = [
                  "Enable (Default)",   # bit0
                  "Enable (Default)",   # bit1
                  "Enable (Default)",   # bit2
                  "1s (Default)",   # bit3
                  "Enable (Default)",   # bit4
                  "Enable (Default)",   # bit5
                  "OTG Disable (default)",   # bit6
                  "NOT force"   # bit7
                ]

reg13_description = [
                  "Enable IBUS_OCP in forward mode",   # bit0
                  "Force VINDPM detection",   # bit1
                  "Disable OTG mode VOTG UVP hiccup protection",   # bit2
                  "Disable forward mode VSYS short hiccup protection",   # bit3
                  "Disable the STAT pin outpu",   # bit4
                  "Switching frequency selection, this bit POR default value is based on the PROG pin strapping.",   # bit5
                  "External ACFET1-RBFET1 gate driver control",   # bit6
                  "External ACFET2-RBFET2 gate driver control"   # bit7
                ]

reg13_bits_set = [
                  "Enable (default)",   # bit0
                  "Force the converter stop switching, and ADC measures the VBUS voltage without input current, then the charger updates the VINDPM register accordingly",   # bit1
                  "Disable",   # bit2
                  "Disable",   # bit3
                  "Disable",   # bit4
                  "750 kHz",   # bit5
                  "turn on",   # bit6
                  "turn on"   # bit7
                ]

reg13_bits_unset = [
                  "Disable",   # bit0
                  "Do NOT force VINDPM detection (default)",   # bit1
                  "Enable (Default)",   # bit2
                  "Enable (Default)",   # bit3
                  "Enable (Default)",   # bit4
                  "1.5 MHz",   # bit5
                  "turn off (default)",   # bit6
                  "turn off (default)"   # bit7
                ]

reg1b_description = [
                  "VBUS present status",   # bit0
                  "VAC1 insert status",   # bit1
                  "VAC2 insert status",   # bit2
                  "Power Good Status",   # bit3
                  "RESERVED",   # bit4
                  "I2C watch dog timer status",   # bit5
                  "VINDPM status (forward mode) or VOTG status (OTG mode)",   # bit6
                  "IINDPM status (forward mode) or IOTG status (OTG mode)"   # bit7
                ]

reg1b_bits_set = [
                  "VBUS present (above present threshold)",   # bit0
                  "VAC1 present (above present threshold)",   # bit1
                  "VAC2 present (above present threshold)",   # bit2
                  "Power good",   # bit3
                  "n/a",   # bit4
                  "WD timer expired",   # bit5
                  "In VINDPM regulation or VOTG regualtion",   # bit6
                  "In IINDPM regulation or IOTG regulation"   # bit7
                ]

reg1b_bits_unset = [
                  "VBUS NOT present",   # bit0
                  "VAC1 NOT present",   # bit1
                  "VAC2 NOT present",   # bit2
                  "NOT in power good status",   # bit3
                  "n/a",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg1e_description = [
                  "RESERVED",   # bit0
                  "Pre-charge timer status",   # bit1
                  "Trickle charge timer status",   # bit2
                  "Fast charge timer status",   # bit3
                  "VSYS Regulation Status (forward mode)",   # bit4
                  "ADC Conversion Status (in one-shot mode only)",   # bit5
                  "The ACFET1-RBFET1 status",   # bit6
                  "The ACFET2-RBFET2 status"   # bit7
                ]

reg1e_bits_set = [
                  "n/a",   # bit0
                  "Safety timer expired",   # bit1
                  "Safety timer expired",   # bit2
                  "Safety timer expired",   # bit3
                  "In VSYSMIN regulation (VBAT < VSYSMIN)",   # bit4
                  "Conversion complete",   # bit5
                  "ACFET1-RBFET1 is placed",   # bit6
                  "ACFET2-RBFET2 is placed"   # bit7
                ]

reg1e_bits_unset = [
                  "n/a",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Not in VSYSMIN regulation (VBAT > VSYSMIN)",   # bit4
                  "Conversion NOT complete",   # bit5
                  "ACFET1-RBFET1 is NOT placed",   # bit6
                  "ACFET2-RBFET2 is NOT placed"   # bit7
                ]

reg1f_description = [
                  "The TS temperature is in the hot range, higher than T5",   # bit0
                  "The TS temperature is in the warm range, between T3 and T5",   # bit1
                  "The TS temperature is in the cool range, between T1 and T2",   # bit2
                  "The TS temperature is in the cold range, lower than T1",   # bit3
                  "The battery voltage is too low to enable OTG mode",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg1f_bits_set = [
                  "TS status is in hot range",   # bit0
                  "TS status is in warm range",   # bit1
                  "TS status is in cool range",   # bit2
                  "TS status is in cold range",   # bit3
                  "The battery volage is too low to enable the OTG operation",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg1f_bits_unset = [
                  "TS status is NOT in hot range",   # bit0
                  "TS status is NOT in warm range",   # bit1
                  "TS status is NOT in cool range",   # bit2
                  "TS status is NOT in cold range",   # bit3
                  "The battery voltage is high enough to enable the OTG operation",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg20_description = [
                  "VAC1 over-voltage status",   # bit0
                  "VAC2 over-voltage status",   # bit1
                  "Converter over current status",   # bit2
                  "IBAT over-current status",   # bit3
                  "IBUS over-current status",   # bit4
                  "VBAT over-voltage status",   # bit5
                  "VBUS over-voltage status",   # bit6
                  "IBAT regulation status"   # bit7
                ]

reg20_bits_set = [
                  "Device in over voltage protection",   # bit0
                  "Device in over voltage protection",   # bit1
                  "Converter in over current protection",   # bit2
                  "Device in over current protectio",   # bit3
                  "Device in over current protection",   # bit4
                  "Device in over voltage protection",   # bit5
                  "Device in over voltage protection",   # bit6
                  "Device in battery discharging current regulation"   # bit7
                ]

reg20_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg21_description = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC temperature shutdown status",   # bit2
                  "RESERVED",   # bit3
                  "OTG under voltage status",   # bit4
                  "OTG over voltage status",   # bit5
                  "VSYS over-voltage status",   # bit6
                  "VSYS short circuit status"   # bit7
                ]

reg21_bits_set = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Device in thermal shutdown protection",   # bit2
                  "n/a",   # bit3
                  "Device in OTG under voltage",   # bit4
                  "Device in OTG over-voltage",   # bit5
                  "Device in SYS over-voltage protection",   # bit6
                  "Device in SYS short circuit protection"   # bit7
                ]

reg21_bits_unset = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg22_description = [
                  "VBUS present flag",   # bit0
                  "VAC1 present flag",   # bit1
                  "VAC2 present flag",   # bit2
                  "Power good flag",   # bit3
                  "Poor source detection flag",   # bit4
                  "I2C watchdog timer flag",   # bit5
                  "VINDPM / VOTG Flag",   # bit6
                  "IINDPM / IOTG flag"   # bit7
                ]

reg22_bits_set = [
                  "VBUS present status changed",   # bit0
                  "VAC1 present status changed",   # bit1
                  "VAC2 present status changed",   # bit2
                  "Any change in PG_STAT even (adapter good qualification or adapter good going away)",   # bit3
                  "Poor source status rising edge detected",   # bit4
                  "WD timer signal rising edge detected",   # bit5
                  "VINDPM / VOTG regulation signal rising edge detected",   # bit6
                  "IINDPM / IOTG signal rising edge detected"   # bit7
                ]

reg22_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg23_description = [
                  "BC1.2 status Flag",   # bit0
                  "BC1.2 status Flag",   # bit1
                  "IC thermal regulation flag",   # bit2
                  "RESERVED",   # bit3
                  "VBUS status flag",   # bit4
                  "RESERVED",   # bit5
                  "ICO status flag",   # bit6
                  "Charge status flag"   # bit7
                ]

reg23_bits_set = [
                  "BC1.2 detection status changed",   # bit0
                  "VBAT present status change",   # bit1
                  "TREG signal rising threshold detected",   # bit2
                  "n/a",   # bit3
                  "VBUS status changed",   # bit4
                  "n/a",   # bit5
                  "ICO status changed",   # bit6
                  "Charge status changed"   # bit7
                ]

reg23_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "n/a",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg24_description = [
                  "Top off timer flag",   # bit0
                  "Pre-charge timer flag",   # bit1
                  "Trickle charge timer flag",   # bit2
                  "Fast charge timer flag",   # bit3
                  "VSYSMIN regulation flag",   # bit4
                  "ADC conversion flag (only in one-shot mode)",   # bit5
                  "D+/D- detection is done flag",   # bit6
                  "RESERVED"   # bit7
                ]

reg24_bits_set = [
                  "Top off timer expired rising edge detected",   # bit0
                  "Pre-charge timer expired rising edge detected",   # bit1
                  "Trickle charger timer expired rising edge detected",   # bit2
                  "Fast charge timer expired rising edge detected",   # bit3
                  "Entered or existed VSYSMIN regulation",   # bit4
                  "Conversion completed",   # bit5
                  "D+/D- detection is completed",   # bit6
                  "n/a"   # bit7
                ]

reg24_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Conversion NOT completed",   # bit5
                  "D+/D- detection is NOT started or still ongoing",   # bit6
                  "n/a"   # bit7
                ]

reg25_description = [
                  "TS hot temperature flag",   # bit0
                  "TS warm temperature flag",   # bit1
                  "TS cool temperature flag",   # bit2
                  "TS cold temperature flag",   # bit3
                  "VBAT too low to enable OTG flag",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg25_bits_set = [
                  "TS across hot temperature (T5) is detected",   # bit0
                  "TS across warm temperature (T3) is detected",   # bit1
                  "TS across cool temperature (T2) is detected",   # bit2
                  "TS across cold temperature (T1) is detected",   # bit3
                  "VBAT falls below the threshold to enable the OTG mode",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg25_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg26_description = [
                  "VAC1 over-voltage flag",   # bit0
                  "VAC2 over-voltage flag",   # bit1
                  "Converter over-current flag",   # bit2
                  "IBAT over-current flag",   # bit3
                  "IBUS over-current flag",   # bit4
                  "VBAT over-voltage flag",   # bit5
                  "VBUS over-voltage flag",   # bit6
                  "IBAT regulation flag"   # bit7
                ]

reg26_bits_set = [
                  "Enter VAC1 OVP",   # bit0
                  "Enter VAC2 OVP",   # bit1
                  "Enter converter OCP",   # bit2
                  "Enter discharged OCP",   # bit3
                  "Enter IBUS OCP",   # bit4
                  "Enter VBAT OVP",   # bit5
                  "Enter VBUS OVP",   # bit6
                  "Enter or exit IBAT regulation"   # bit7
                ]

reg26_bits_unset = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg27_description = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC thermal shutdown flag",   # bit2
                  "RESERVED",   # bit3
                  "OTG under-voltage flag",   # bit4
                  "OTG over-voltage flag",   # bit5
                  "VSYS over-voltage flag",   # bit6
                  "VSYS short circuit flag"   # bit7
                ]

reg27_bits_set = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TS shutdown signal rising threshold detected",   # bit2
                  "n/a",   # bit3
                  "Stop OTG due to VBUS under-voltage",   # bit4
                  "Stop OTG due to VBUS over voltage",   # bit5
                  "Stop switching due to system over-voltage",   # bit6
                  "Stop switching due to system short"   # bit7
                ]

reg27_bits_unset = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg28_description = [
                  "VBUS present mask flag",   # bit0
                  "VAC1 present mask flag",   # bit1
                  "VAC2 present mask flag",   # bit2
                  "Power Good mask flag",   # bit3
                  "Poor source detection mask flag",   # bit4
                  "I2C watch dog timer mask flag",   # bit5
                  "VINDPM / VOTG mask flag",   # bit6
                  "IINDPM / IOTG mask flag"   # bit7
                ]

reg28_bits_set = [
                  "VBUS present status change does NOT produce INT",   # bit0
                  "VAC1 present status change does NOT produce INT",   # bit1
                  "VAC2 present status change does NOT produce INT",   # bit2
                  "PG toggle does NOT produce INT",   # bit3
                  "Poor source detected does NOT produce INT",   # bit4
                  "I2C watch dog timer expired does produce INT pulse",   # bit5
                  "Enter VINDPM / VOTG does NOT produce INT pulse",   # bit6
                  "Enter IINDPM / IOTG does NOT produce INT pulse"   # bit7
                ]

reg28_bits_unset = [
                  "VBUS present status change does produce INT",   # bit0
                  "VAC1 present status change does produce INT",   # bit1
                  "VAC2 present status change does produce INT",   # bit2
                  "PG toggle does produce INT",   # bit3
                  "Poor source detected does produce INT",   # bit4
                  "I2C watch dog timer expired does produce INT pulse",   # bit5
                  "Enter VINDPM / VOTG does produce INT pulse",   # bit6
                  "Enter IINDPM / IOTG does produce INT pulse"   # bit7
                ]

reg29_description = [
                  "BC1.2 status mask flag",   # bit0
                  "VBAT present mask flag",   # bit1
                  "IC thermal regulation mask flag",   # bit2
                  "RESERVED",   # bit3
                  "VBUS status mask flag",   # bit4
                  "RESERVED",   # bit5
                  "ICO status mask flag",   # bit6
                  "Charge status mask flag"   # bit7
                ]

reg29_bits_set = [
                  "BC1.2 status change does NOT produce INT",   # bit0
                  "VBAT present status change does NOT produce INT",   # bit1
                  "entering TREG does NOT produce INT",   # bit2
                  "n/a",   # bit3
                  "VBUS status change does NOT produce INT",   # bit4
                  "n/a",   # bit5
                  "ICO status change does NOT produce INT",   # bit6
                  "ICO status change does NOT produce INT"   # bit7
                ]

reg29_bits_unset = [
                  "BC1.2 status change does produce INT",   # bit0
                  "VBAT present status change does produce INT",   # bit1
                  "entering TREG does produce INT",   # bit2
                  "n/a",   # bit3
                  "VBUS status change does produce INT",   # bit4
                  "n/a",   # bit5
                  "ICO status change does produce INT",   # bit6
                  "ICO status change does produce INT"   # bit7
                ]

reg2a_description = [
                  "Top off timer mask flag",   # bit0
                  "Pre-charge timer mask flag",   # bit1
                  "Trickle charge timer mask flag",   # bit2
                  "Fast charge timer mask flag",   # bit3
                  "VSYS min regulation mask flag",   # bit4
                  "ADC conversion mask flag (only in one-shot mode)",   # bit5
                  "D+/D- detection is done mask flag",   # bit6
                  "RESERVED"   # bit7
                ]

reg2a_bits_set = [
                  "Top off timer expire does NOT produce INT",   # bit0
                  "Pre-charge timer expire does NOT produce INT",   # bit1
                  "Trickle charge timer expire does NOT produce INT",   # bit2
                  "Fast charge timer expire does NOT produce INT",   # bit3
                  "enter or exit VSYSMIN regulation does NOT produce INT pulse",   # bit4
                  "ADC conversion done does NOT produce INT pulse",   # bit5
                  "D+/D- detection done does NOT produce INT pulse",   # bit6
                  "n/a"   # bit7
                ]

reg2a_bits_unset = [
                  "Top off timer expire does produce INT",   # bit0
                  "Pre-charge timer expire does produce INT",   # bit1
                  "Trickle charge timer expire does produce INT",   # bit2
                  "Fast charge timer expire does produce INT",   # bit3
                  "enter or exit VSYSMIN regulation does produce INT pulse",   # bit4
                  "ADC conversion done does produce INT pulse",   # bit5
                  "D+/D- detection done does produce INT pulse",   # bit6
                  "n/a"   # bit7
                ]

reg2b_description = [
                  "TS hot temperature interrupt mask",   # bit0
                  "TS warm temperature interrupt mask",   # bit1
                  "TS cool temperature interrupt mask",   # bit2
                  "VBAT too low to enable OTG mask",   # bit3
                  "VBAT too low to enable OTG mask",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg2b_bits_set = [
                  "TS across hot temperature (T5) does NOT produce INT",   # bit0
                  "TS across warm temperature (T3) does NOT produce INT",   # bit1
                  "TS across cool temperature (T2) does NOT produce INT",   # bit2
                  "TS across cold temperature (T1) does NOT produce INT",   # bit3
                  "VBAT falling below the threshold to enable the OTG mode, does NOT produce INT",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg2b_bits_unset = [
                  "TS across hot temperature (T5) does produce INT",   # bit0
                  "TS across warm temperature (T3) does produce INT",   # bit1
                  "TS across cool temperature (T2) does produce INT",   # bit2
                  "TS across cold temperature (T1) does produce INT",   # bit3
                  "VBAT falling below the threshold to enable the OTG mode, does produce INT",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg2c_description = [
                  "VAC1 over-voltage mask flag",   # bit0
                  "VAC2 over-voltage mask flag",   # bit1
                  "Converter over-current mask flag",   # bit2
                  "IBAT over-current mask flag",   # bit3
                  "IBUS over-current mask flag",   # bit4
                  "VBAT over-voltage mask flag",   # bit5
                  "VBUS over-voltage mask flag",   # bit6
                  "IBAT regulation mask flag"   # bit7
                ]

reg2c_bits_set = [
                  "entering VAC1 OVP does NOT produce INT",   # bit0
                  "entering VAC2 OVP does NOT produce INT",   # bit1
                  "Converter OCP fault does NOT produce INT",   # bit2
                  "IBAT OCP fault does NOT produce INT",   # bit3
                  "IBUS OCP fault does NOT produce INT",   # bit4
                  "entering VBAT OVP does NOT produce INT",   # bit5
                  "entering VBUS OVP does NOT produce INT",   # bit6
                  "enter or exit IBAT regulation does NOT produce INT"   # bit7
                ]

reg2c_bits_unset = [
                  "entering VAC1 OVP does produce INT",   # bit0
                  "entering VAC2 OVP does produce INT",   # bit1
                  "Converter OCP fault does produce INT",   # bit2
                  "IBAT OCP fault does produce INT",   # bit3
                  "IBUS OCP fault does produce INT",   # bit4
                  "entering VBAT OVP does produce INT",   # bit5
                  "entering VBUS OVP does produce INT",   # bit6
                  "enter or exit IBAT regulation does produce INT"   # bit7
                ]

reg2d_description = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC thermal shutdown mask flag",   # bit2
                  "RESERVED",   # bit3
                  "OTG under-voltage mask flag",   # bit4
                  "OTG over-voltage mask flag",   # bit5
                  "VSYS over-voltage mask flag",   # bit6
                  "VSYS short circuit mask flag"   # bit7
                ]

reg2d_bits_set = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TSHUT does NOT produce INT",   # bit2
                  "n/a",   # bit3
                  "OTG VBUS under voltage fault does NOT produce INT",   # bit4
                  "OTG VBUS over-voltage fault does NOT produce INT",   # bit5
                  "System over-voltage fault does NOT produce INT",   # bit6
                  "System short fault does NOT produce INT"   # bit7
                ]

reg2d_bits_unset = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TSHUT does produce INT",   # bit2
                  "n/a",   # bit3
                  "OTG VBUS under voltage fault does produce INT",   # bit4
                  "OTG VBUS over-voltage fault does produce INT",   # bit5
                  "System over-voltage fault does produce INT",   # bit6
                  "System short fault does produce INT"   # bit7
                ]

reg2f_description = [
                  "RESERVED",   # bit0
                  "TDIE ADC control",   # bit1
                  "TS ADC control",   # bit2
                  "VSYS ADC control",   # bit3
                  "VBAT ADC control",   # bit4
                  "VBUS ADC control",   # bit5
                  "IBAT ADC control",   # bit6
                  "IBUS ADC control"   # bit7
                ]

reg2f_bits_set = [
                  "n/a",   # bit0
                  "Disable",   # bit1
                  "Disable",   # bit2
                  "Disable",   # bit3
                  "Disable",   # bit4
                  "Disable",   # bit5
                  "Disable",   # bit6
                  "Disable"   # bit7
                ]

reg2f_bits_unset = [
                  "n/a",   # bit0
                  "Enable (Default)",   # bit1
                  "Enable (Default)",   # bit2
                  "Enable (Default)",   # bit3
                  "Enable (Default)",   # bit4
                  "Enable (Default)",   # bit5
                  "Enable (Default)",   # bit6
                  "Enable (Default)"   # bit7
                ]

reg30_description = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "RESERVED",   # bit2
                  "RESERVED",   # bit3
                  "VAC1 ADC Control",   # bit4
                  "VAC2 ADC Control",   # bit5
                  "D- ADC Control",   # bit6
                  "D+ ADC Control"   # bit7
                ]

reg30_bits_set = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "n/a",   # bit2
                  "n/a",   # bit3
                  "Disable",   # bit4
                  "Disable",   # bit5
                  "Disable",   # bit6
                  "Disable"   # bit7
                ]

reg30_bits_unset = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "n/a",   # bit2
                  "n/a",   # bit3
                  "Enable (Default)",   # bit4
                  "Enable (Default)",   # bit5
                  "Enable (Default)",   # bit6
                  "Enable (Default)"   # bit7
                ]

regXX_description = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regXX_bits_set = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regXX_bits_unset = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

