register_name:str = [
                    'REG00', 'REG01', 'REG03', 'REG05', 'REG06', 'REG08', 'REG09', 'REG0A', 'REG0B', 'REG0D',  # index 0 to 9
                    'REG0E', 'REG0F', 'REG10', 'REG11', 'REG12', 'REG13', 'REG14', 'REG15', 'REG16', 'REG17',      # index 10 to 19
                    'REG18', 'REG19', 'REG1B', 'REG1C', 'REG1D', 'REG1E', 'REG1F', 'REG20', 'REG21', 'REG22',      # index 20 to 29
                    'REG23', 'REG24', 'REG25', 'REG26', 'REG27', 'REG28', 'REG29', 'REG2A', 'REG2B', 'REG2C',      # index 30 to 39
                    'REG2D', 'REG2E', 'REG2F', 'REG30', 'REG31', 'REG33', 'REG35', 'REG37', 'REG39', 'REG3B',      # index 40 to 49
                    'REG3D', 'REG3F', 'REG41', 'REG43', 'REG45', 'REG47', 'REG48', 'PG', 'IRQ', 'STAT'            # index 50 to 59
                    ]

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
                        "ADC Function disable 0", # 2Fh
                        "ADC Function disable 1", # 30h
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

register_offset:int = [
                      2500, 0, 0, 0, 0, 0, 0, 0, 2800, 0,  # REG00 to REG0D
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG0E to REG17
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG18 to REG22
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG23 to REG2C
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        # REG2D to REG3B
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0         # REG3D to REG48, along with PG, IRQ, STAT
                      ]

register_step_size: int = [
                          250, 10, 10, 100, 10, 40, 40, -1, 10, -1, # REG00 to REG0D
                          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG0E to REG17
                          -1, 10, -1, -1, -1, -1, -1, -1, -1, -1,   # REG18 to REG22
                          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   # REG23 to REG2C
                          -1, -1, -1, -1, 1, 1, 1, 1, 1, 1,         # REG2D to REG3B
                          1, 0.0976563, 0.5, 1, 1, -1, -1, -1, -1, -1   # REG3D to REG48, along with PG, IRQ, STAT
                          ]

reg08_bit_names: str = [
                  "IPRECHG:0",   # bit0
                  "IPRECHG:1",   # bit1
                  "IPRECHG:2",   # bit2
                  "IPRECHG:3",   # bit3
                  "IPRECHG:4",   # bit4
                  "IPRECHG:5",   # bit5
                  "VBAT_LOWV:0",   # bit6
                  "VBAT_LOWV:1"   # bit7
                ]

reg08_description: str = [
                  "Precharge current limit",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "Battery voltage thresholds for the transition from precharge to fast charge, which is defined as a ratio of battery regulation limit (VREG)",   # bit6
                  ""   # bit7
                ]

reg08_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg08_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg08_bits_VBAT_LOWV = [
                  "15%*VREG",   # 0
                  "62.2%*VREG",   # 1
                  "66.7%*VREG",   # 2
                  "71.4%*VREG",   # 3
                    ]

reg09_bit_names: str = [
                  "ITERM:0",   # bit0
                  "ITERM:1",   # bit1
                  "ITERM:2",   # bit2
                  "ITERM:3",   # bit3
                  "ITERM:4",   # bit4
                  "STOP_WD_CHG",   # bit5
                  "REG_RST",   # bit6
                  "RESERVED"   # bit7
                ]

reg09_description: str = [
                  "Termination current",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "Defines whether a watchdog timer expiration will disable charging",   # bit5
                  "Reset registers to default values and reset timer",   # bit6
                  "RESERVED"   # bit7
                ]

reg09_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "WD timer expiration sets EN_CHG=0",   # bit5
                  "Reset",   # bit6
                  "n/a"   # bit7
                ]

reg09_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "WD timer expiration keeps existing EN_CHG setting",   # bit5
                  " Not reset",   # bit6
                  "n/a"   # bit7
                ]

reg0a_bit_names: str = [
                  "VRECHG:0",   # bit0
                  "VRECHG:1",   # bit1
                  "VRECHG:2",   # bit2
                  "VRECHG:3",   # bit3
                  "TRECHG:0",   # bit4
                  "TRECHG:1",   # bit5
                  "CELL:0",   # bit6
                  "CELL:1"   # bit7
                ]

reg0a_description: str = [
                  "Battery Recharge Threshold Offset (Below VREG)",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "Battery recharge deglich time",   # bit4
                  "",   # bit5
                  "At POR, the charger reads the PROG pin resistance to determine the battery cell count and update this CELL bits accordingly",   # bit6
                  ""   # bit7
                ]

reg0a_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg0a_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ":"   # bit7
                ]

reg0a_bits_CELL: str = [
                  "1s",   # 0
                  "2s",   # 1
                  "3s",   # 2
                  "4s",   # 3
                    ]

reg0a_bits_TRECHG: str = [
                  "64ms",   # 0
                  "256ms",   # 1
                  "1024ms (default)",   # 2
                  "2048ms",   # 3
                  "",   # 4
                  "",   # 5
                  "",   # 6
                  ""   # 7
                    ]

reg0d_bit_names: str = [
                  "IOTG:0",   # bit0
                  "IOTG:1",   # bit1
                  "IOTG:2",   # bit2
                  "IOTG:3",   # bit3
                  "IOTG:4",   # bit4
                  "IOTG:5",   # bit5
                  "IOTG:6",   # bit6
                  "PRECHG_TMR"   # bit7
                ]

reg0d_description: str = [
                  "OTG current limit",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  "Pre-charge safety timer setting"   # bit7
                ]

reg0d_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  "0.5 hrs"   # bit7
                ]

reg0d_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  "2 hrs (default)"   # bit7
                ]

reg0e_bit_names: str = [
                  "TMR2X_EN",   # bit0
                  "CHG_TMR:0",   # bit1
                  "CHG_TMR:1",   # bit2
                  "EN_CHG_TMR",   # bit3
                  "EN_PRECHG_TMR",   # bit4
                  "EN_TRICHG_TMR",   # bit5
                  "TOPOFF_TMR:0",   # bit6
                  "TOPOFF_TMR:1"   # bit7
                ]

reg0e_description: str = [
                  "TMR2X_EN",   # bit0
                  "Fast charge timer setting",   # bit1
                  "",   # bit2
                  "enable fast charge timer",   # bit3
                  "enable pre-charge timer",   # bit4
                  "enable trickle charge timer (fixed as 1hr)",   # bit5
                  "Top-off timer control",   # bit6
                  ""   # bit7
                ]

reg0e_bits_set:str = [
                  "pre-/trickle-/fast-charge timer slowed by 2X during input DPM or thermal regulation (default)",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "enabled (default)",   # bit3
                  "enabled (default)",   # bit4
                  "enabled (default)",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg0e_bits_unset: str = [
                  "pre-/trickle-/fast-charge timer NOT slowed by 2X during input DPM or thermal regulation",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "disabled",   # bit3
                  "disabled",   # bit4
                  "disabled",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg0e_bits_TOPOFF_TMR: str = [
                  "disabled (default)",   # 0
                  "15 mins",   # 1
                  "30 mins",   # 2
                  "45 mins",   # 3
                    ]

reg0e_bits_CHG_TMR: str = [
                  "5 hrs",   # 0
                  "8 hrs",   # 1
                  "12 hrs (default)",   # 2
                  "24 hrs",   # 3
                    ]

reg0f_bit_names: str = [
                  "EN_BACKUP",   # bit0
                  "EN_TERM",   # bit1
                  "EN_HIZ",   # bit2
                  "FORCE_ICO",   # bit3
                  "EN_ICO",   # bit4
                  "EN_CHG",   # bit5
                  "FORCE_IBATDIS",   # bit6
                  "EN_AUTO_IBATDIS"   # bit7
                ]

reg0f_description: str = [
                    "enables backup mode where OTG automatically engages when VBUS droops below voltage set in REG10",   # bit0
                    "Termination",   # bit1
                    "HI Z mode",   # bit2
                    "Force start input current optimizer",   # bit3
                    "Input Current Optimizer (ICO)",   # bit4
                    "Charger enable Configuration",   # bit5
                    "Force a battery discharging current",   # bit6
                    "enable the auto battery discharging during the battery OVP fault"   # bit7
                    ]

reg0f_bits_set:str = [
                  "enable",   # bit0
                  "enable (default)",   # bit1
                  "enable",   # bit2
                  "Force ICO start",   # bit3
                  "enable ICO",   # bit4
                  "Charge enable (default)",   # bit5
                  "Force the charger to apply a discharging current on BAT regardless the battery OVP status",   # bit6
                  "The charger will apply a discharging current on BAT during battery OVP"   # bit7
                ]

reg0f_bits_unset: str = [
                  "disable (default)",   # bit0
                  "disable",   # bit1
                  "disable (default)",   # bit2
                  "Do NOT force ICO (default)",   # bit3
                  "disable ICO (default)",   # bit4
                  "Charge disable",   # bit5
                  "IDLE (default)",   # bit6
                  "The charger will NOT apply a discharging current on BAT during battery OVP"   # bit7
                ]

reg10_bit_names: str = [
                  "WATCHDOG:0",   # bit0
                  "WATCHDOG:1",   # bit1
                  "WATCHDOG:2",   # bit2
                  "WD_RST",   # bit3
                  "VAC_OVP:0",   # bit4
                  "VAC_OVP:1",   # bit5
                  "VBUS_BACKUP:0",   # bit6
                  "VBUS_BACKUP:1"   # bit7
                ]

reg10_description: str = [
                  "Watchdog timer settings",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "I2C watch dog timer reset",   # bit3
                  "VAC_OVP thresholds",   # bit4
                  "",   # bit5
                  "The thresholds to trigger the backup mode, defined as a ratio of VINDPM",   # bit6
                  ""   # bit7
                ]

reg10_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "Reset (this bit goes back to 0 after timer resets)",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg10_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "Normal (default)",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg10_bits_VBUS_BACKUP: str = [
                  "40%*VINDPM",   # 0
                  "60%*VINDPM",   # 1
                  "80%*VINDPM (default)",   # 2
                  "100%*VINDPM",   # 3
                    ]

reg10_bits_VAC_OVP: str = [
                  "26V",   # 0
                  "22V",   # 1
                  "12V",   # 2
                  "7V (default)",   # 3
                    ]

reg10_bits_WATCHDOG: str = [
                  "disable",   # 0
                  "0.5s",   # 1
                  "1s",   # 2
                  "2s",   # 3
                  "20s",   # 4
                  "40s (default)",   # 2
                  "80s",   # 6
                  "160s"   # 7
                    ]

reg11_bit_names: str = [
                  "SDRV_DLY",   # bit0
                  "SDRV_CTRL:0",   # bit1
                  "SDRV_CTRL:1",   # bit2
                  "HVDCP_EN",   # bit3
                  "EN_9V",   # bit4
                  "EN_12V",   # bit5
                  "AUTO_INDET_EN",   # bit6
                  "FORCE_INDET"   # bit7
                ]

reg11_description: str = [
                  "Delay time added to the taking action in bit [2:1] of the SFET control",   # bit0
                  "SFET control",   # bit1
                  "",   # bit2
                  "High voltage DCP enable",   # bit3
                  "EN_9V HVDC",   # bit4
                  "EN_12V HVDC",   # bit5
                  "Automatic D+/D- Detection enable",   # bit6
                  "Force D+/D- detection"   # bit7
                ]

reg11_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "enable HVDCP handshake",   # bit3
                  "enable 9V mode in HVDCP",   # bit4
                  "enable 12V mode in HVDCP",   # bit5
                  "enable D+/D- detection when VBUS is plugged-in (default)",   # bit6
                  "Force D+/D- algorithm, when D+/D- detection is done, this bit will be reset to 0"   # bit7
                ]

reg11_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "disable HVDCP handshake (default)",   # bit3
                  "disable 9V mode in HVDCP (default)",   # bit4
                  "enable 12V mode in HVDCP",   # bit5
                  "enable D+/D- detection when VBUS is plugged-in (default)",   # bit6
                  "Force D+/D- algorithm, when D+/D- detection is done, this bit will be reset to 0"   # bit7
                ]

reg11_bits_SDRV_CTRL: str = [
                  "IDLE (default)",   # 0
                  "Shutdown Mode",   # 1
                  "Ship Mode",   # 2
                  "System Power Reset",   # 3
                    ]

reg12_bit_names: str = [
                  "DIS_FWD_OOA",   # bit0
                  "DIS_OTG_OOA",   # bit1
                  "DIS_LDO",   # bit2
                  "WKUP_DLY",   # bit3
                  "PFM_FWD_DIS",   # bit4
                  "PFM_OTG_DIS",   # bit5
                  "EN_OTG",   # bit6
                  "DIS_ACDRV"   # bit7
                ]

reg12_description: str = [
                  "disable OOA in forward mode",   # bit0
                  "disable OOA in OTG mode",   # bit1
                  "disable BATFET LDO mode in pre-charge stage",   # bit2
                  "When wake up the device from ship mode, how much time (tSM_EXIT) is required to pull low the QON pin",   # bit3
                  "disable PFM in forward mode",   # bit4
                  "disable PFM in OTG mode",   # bit5
                  "OTG mode control",   # bit6
                  "When this bit is set, the charger will force both EN_ACDRV1=0 and EN_ACDRV2=0"   # bit7
                ]

reg12_bits_set:str = [
                  "disable",   # bit0
                  "disable",   # bit1
                  "disable",   # bit2
                  "15ms",   # bit3
                  "disable",   # bit4
                  "disable",   # bit5
                  "OTG enable",   # bit6
                  "force"   # bit7
                ]

reg12_bits_unset: str = [
                  "enable (default)",   # bit0
                  "enable (default)",   # bit1
                  "enable (default)",   # bit2
                  "1s (default)",   # bit3
                  "enable (default)",   # bit4
                  "enable (default)",   # bit5
                  "OTG disable (default)",   # bit6
                  "NOT force"   # bit7
                ]

reg13_bit_names: str = [
                  "EN_IBUS_OCP",   # bit0
                  "FORCE_VINDPM_DET",   # bit1
                  "DIS_VOTG_UVP",   # bit2
                  "DIS_VSYS_SHORT",   # bit3
                  "DIS_STAT",   # bit4
                  "PWM_FREQ",   # bit5
                  "EN_ACDRV1",   # bit6
                  "EN_ACDRV2"   # bit7
                ]

reg13_description: str = [
                  "enable IBUS_OCP in forward mode",   # bit0
                  "Force VINDPM detection",   # bit1
                  "disable OTG mode VOTG UVP hiccup protection",   # bit2
                  "disable forward mode VSYS short hiccup protection",   # bit3
                  "disable the STAT pin outpu",   # bit4
                  "Switching frequency selection, this bit POR default value is based on the PROG pin strapping",   # bit5
                  "External ACFET1-RBFET1 gate driver control",   # bit6
                  "External ACFET2-RBFET2 gate driver control"   # bit7
                ]

reg13_bits_set:str = [
                  "enable (default)",   # bit0
                  "Force the converter stop switching, and ADC measures the VBUS voltage without input current, then the charger updates the VINDPM register accordingly",   # bit1
                  "disable",   # bit2
                  "disable",   # bit3
                  "disable",   # bit4
                  "750 kHz",   # bit5
                  "turn on",   # bit6
                  "turn on"   # bit7
                ]

reg13_bits_unset: str = [
                  "disable",   # bit0
                  "Do NOT force VINDPM detection (default)",   # bit1
                  "enable (default)",   # bit2
                  "enable (default)",   # bit3
                  "enable (default)",   # bit4
                  "1.5 MHz",   # bit5
                  "turn off (default)",   # bit6
                  "turn off (default)"   # bit7
                ]

reg14_bit_names: str = [
                  "EN_BATOC",   # bit0
                  "EN_EXTILIM",   # bit1
                  "EN_IINDPM",   # bit2
                  "IBAT_REG:0",   # bit3
                  "IBAT_REG:1",   # bit4
                  "EN_IBAT",   # bit5
                  "RESERVED",   # bit6
                  "SFET_PRESENT"   # bit7
                ]

reg14_description: str = [
                  "enable the battery discharging current OCP",   # bit0
                  "enable the external ILIM_HIZ pin input current regulation",   # bit1
                  "enable the internal IINDPM register input current regulation",   # bit2
                  "Battery discharging current regulation in OTG mode",   # bit3
                  "",   # bit4
                  "IBAT discharge current sensing enable",   # bit5
                  "RESERVED",   # bit6
                  "FET is populated"   # bit7
                ]

reg14_bits_set:str = [
                  "enable",   # bit0
                  "enable (default)",   # bit1
                  "enable (default)",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "enable the IBAT discharge sensing at battery only or OTG condition",   # bit5
                  "n/a",   # bit6
                  "Ship FET populated"   # bit7
                ]

reg14_bits_unset: str = [
                  "disable (default)",   # bit0
                  "disable",   # bit1
                  "disable",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "disable the IBAT discharge sensing at battery only or OTG condition (default)",   # bit5
                  "n/a",   # bit6
                  "No ship FET populated"   # bit7
                ]

reg14_bits_IBAT_REG: str = [
                  "3A",   # 0
                  "4A",   # 1
                  "5A",   # 2
                  "disable (default)",   # 3
                    ]

reg15_bit_names: str = [
                  "EN_MPPT",   # bit0
                  "VOC_RATE:0",   # bit1
                  "VOC_RATE:1",   # bit2
                  "VOC_DLY:0",   # bit3
                  "VOC_DLY:1",   # bit4
                  "VOC_PCT:0",   # bit5
                  "VOC_PCT:1",   # bit6
                  "VOC_PCT:2"   # bit7
                ]

reg15_description: str = [
                  "enable the MPPT to measure the VBUS open circuit voltage",   # bit0
                  "The time interval two VBUS open circuit voltage measurements",   # bit1
                  "",   # bit2
                  "After the converter stops switching, the time delay before the VOC is measured",   # bit3
                  "",   # bit4
                  "To set the VINDPM as a percentage of the VBUS open circuit voltage when the VOC measurement is done",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg15_bits_set:str = [
                  "enable",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg15_bits_unset: str = [
                  "disable (default)",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg15_bits_VOC_PCT: str = [
                  "0.5625",   # 0
                  "0.625",   # 1
                  "0.6875",   # 2
                  "0.75",   # 3
                  "0.8125",   # 4
                  "0.875 (default)",   # 5
                  "0.9375",   # 6
                  "1"   # 7
                    ]

reg15_bits_VOC_DLY: str = [
                  "50ms",   # 0
                  "300ms (default)",   # 1
                  "2s",   # 2
                  "5s",   # 3
                    ]

reg15_bits_VOC_RATE: str = [
                  "30s",   # 0
                  "2mins (default)",   # 1
                  "10mins",   # 2
                  "30mins",   # 3
                    ]

reg16_bit_names: str = [
                  "BKUP_ACFET1_ON",   # bit0
                  "VAC2_PD_EN",   # bit1
                  "VAC1_PD_EN",   # bit2
                  "VBUS_PD_EN",   # bit3
                  "TSHUT:0",   # bit4
                  "TSHUT:1",   # bit5
                  "TREG:0",   # bit6
                  "TREG:1"   # bit7
                ]

reg16_description: str = [
                  "if charger in backup mode, ACFET1 is off. Setting this bit to 1 clears the EN_BACKUP bit, sets DIS_ACDRV=0 and EN_ACDRV1=1 to turn on ACFET1",   # bit0
                  "enable VAC2 pull down resistor",   # bit1
                  "enable VAC1 pull down resistor",   # bit2
                  "enable VBUS pull down resistor (6k Ohm)",   # bit3
                  "Thermal shutdown thresholds",   # bit4
                  "",   # bit5
                  "Thermal regulation thresholds",   # bit6
                  ""   # bit7
                ]

reg16_bits_set:str = [
                  "To turn on ACFET1 in backup mode",   # bit0
                  "enable",   # bit1
                  "enable",   # bit2
                  "enable",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg16_bits_unset: str = [
                  "IDLE (default)",   # bit0
                  "disable (default)",   # bit1
                  "disable (default)",   # bit2
                  "disable (default)",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg16_bits_TREG: str = [
                  "60°C",   # 0
                  "80°C",   # 1
                  "100°C",   # 2
                  "120°C (default)",   # 3
                    ]

reg16_bits_TSHUT: str = [
                  "150°C (default)",   # 0
                  "130°C",   # 1
                  "120°C",   # 2
                  "85°C",   # 3
                    ]

reg17_bit_names: str = [
                  "RESERVED",   # bit0
                  "JEITA_ISETC:0",   # bit1
                  "JEITA_ISETC:1",   # bit2
                  "JEITA_ISETH:0",   # bit3
                  "JEITA_ISETH:1",   # bit4
                  "JEITA_VSET:0",   # bit5
                  "JEITA_VSET:1",   # bit6
                  "JEITA_VSET:2"   # bit7
                ]

reg17_description: str = [
                  "RESERVED",   # bit0
                  "JEITA low temperature range (TCOLD – TCOOL) charge current setting",   # bit1
                  "",   # bit2
                  "JEITA high temperature range (TWARN – THOT) charge current setting",   # bit3
                  "",   # bit4
                  "JEITA high temperature range (TWARN – THOT) charge voltage setting",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg17_bits_set:str = [
                  "n/a",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg17_bits_unset: str = [
                  "n/a",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg17_bits_JEITA_VSET: str = [
                  "Charge Suspend",   # 0
                  "Set VREG to VREG-800mV",   # 1
                  "Set VREG to VREG-600mV",   # 2
                  "Set VREG to VREG-400mV (default)",   # 3
                  "Set VREG to VREG-300mV",   # 4
                  "Set VREG to VREG-200mV",   # 5
                  "Set VREG to VREG-100mV",   # 6
                  "VREG unchanged"   # 7
                    ]

reg17_bits_JEITA_ISETH: str = [
                  "Charge Suspend",   # 0
                  "Set ICHG to 20%* ICHG",   # 1
                  "Set ICHG to 40%* ICHG",   # 2
                  "ICHG unchanged (default)",   # 3
                    ]

reg17_bits_JEITA_ISETC: str = [
                  "Charge Suspend",   # 0
                  "Set ICHG to 20%* ICHG (default)",   # 1
                  "Set ICHG to 40%* ICHG",   # 2
                  "ICHG unchanged",   # 3
                    ]

reg18_bit_names: str = [
                  "TS_IGNORE",   # bit0
                  "BCOLD",   # bit1
                  "BHOT:0",   # bit2
                  "BHOT:1",   # bit3
                  "TS_WARM:0",   # bit4
                  "TS_WARM:1",   # bit5
                  "TS_COOL:0",   # bit6
                  "TS_COOL:1"   # bit7
                ]

reg18_description: str = [
                  "Ignore TS feedback, charger considers TS is always good to allow charging and OTG, all four TS status bits always stay at 0x0000",   # bit0
                  "OTG mode TS COLD temperature threshold",   # bit1
                  "OTG mode TS HOT temperature threshold",   # bit2
                  "",   # bit3
                  "JEITA VT3 comparator voltage falling threshold percentage of REGN. Temperature in brackets is achieved with 103AT NTC thermistor",   # bit4
                  "",   # bit5
                  "JEITA VT2 comparator voltage rising threshold percentage of REGN. Temperature in brackets is achieved with103AT NTC thermistor",   # bit6
                  ""   # bit7
                ]

reg18_bits_set:str = [
                  "Ignore",   # bit0
                  "-20°C",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg18_bits_unset: str = [
                  "NOT ignore (default)",   # bit0
                  "-10°C (default)",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg18_bits_TS_COOL: str = [
                  "71.1% (5°C)",   # 0
                  "68.4% (default) (10°C)",   # 1
                  "65.5% (15°C)",   # 2
                  "62.4% (20°C)",   # 3
                    ]

reg18_bits_TS_WARM: str = [
                  "48.4% (40°C)",   # 0
                  "44.8% (default) (45°C)",   # 1
                  "41.2% (50°C)",   # 2
                  "37.7% (55°C)",   # 3
                    ]

reg18_bits_BHOT: str = [
                  "37.7% (55°C)",   # 0
                  "60°C (default)",   # 1
                  "65°C",   # 2
                  "disable",   # 3
                    ]

reg1b_bit_names: str = [
                  "VBUS_PRESENT_STAT",   # bit0
                  "AC1_PRESENT_STAT",   # bit1
                  "AC2_PRESENT_STAT",   # bit2
                  "PG_STAT",   # bit3
                  "RESERVED",   # bit4
                  "WD_STAT",   # bit5
                  "VINDPM_STAT ",   # bit6
                  "IINDPM_STAT"   # bit7
                ]

reg1b_description: str = [
                  "VBUS present status",   # bit0
                  "VAC1 insert status",   # bit1
                  "VAC2 insert status",   # bit2
                  "Power Good Status",   # bit3
                  "RESERVED",   # bit4
                  "I2C watch dog timer status",   # bit5
                  "VINDPM status (forward mode) or VOTG status (OTG mode)",   # bit6
                  "IINDPM status (forward mode) or IOTG status (OTG mode)"   # bit7
                ]

reg1b_bits_set:str = [
                  "VBUS present (above present threshold)",   # bit0
                  "VAC1 present (above present threshold)",   # bit1
                  "VAC2 present (above present threshold)",   # bit2
                  "Power good",   # bit3
                  "n/a",   # bit4
                  "WD timer expired",   # bit5
                  "In VINDPM regulation or VOTG regualtion",   # bit6
                  "In IINDPM regulation or IOTG regulation"   # bit7
                ]

reg1b_bits_unset: str = [
                  "VBUS NOT present",   # bit0
                  "VAC1 NOT present",   # bit1
                  "VAC2 NOT present",   # bit2
                  "NOT in power good status",   # bit3
                  "n/a",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg1c_bit_names: str = [
                  "BC1.2_DONE_STAT",   # bit0
                  "VBUS_STAT:0",   # bit1
                  "VBUS_STAT:1",   # bit2
                  "VBUS_STAT:2",   # bit3
                  "VBUS_STAT:3",   # bit4
                  "CHG_STAT:0",   # bit5
                  "CHG_STAT:1",   # bit6
                  "CHG_STAT:2"   # bit7
                ]

reg1c_description: str = [
                  "BC1.2 status bit",   # bit0
                  "VBUS status bits",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "Charge Status bits",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg1c_bits_set:str = [
                  "BC1.2 or non-standard detection complete",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg1c_bits_unset: str = [
                  "BC1.2 or non-standard detection NOT complete",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg1c_bits_CHG_STAT: str = [
                  "Not Charging",   # 0
                  "Trickle Charge",   # 1
                  "Pre-charge",   # 2
                  "Fast charge (CC mode)",   # 3
                  "Taper Charge (CV mode)",   # 4
                  "Reserved",   # 5
                  "Top-off Timer Active Charging",   # 6
                  "Charge Termination Done"   # 7
                    ]

reg1c_bits_VBUS_STAT: str = [
                  "No Input or BHOT or BCOLD in OTG mode",   # 0
                  "USB SDP (500mA)",   # 1
                  "USB CDP (1.5A)",   # 2
                  "USB DCP (3.25A)",   # 3
                  "Adjustable High Voltage DCP (HVDCP) (1.5A)",   # 4
                  "Unknown adaptor (3A)",   # 5
                  "Non-Standard Adapter (1A/2A/2.1A/2.4A)",   # 6
                  "In OTG mode",   # 7
                  "Not qualified adaptor",   # 8
                  "Reserved",   # 9
                  "Reserved",   # 10
                  "Device directly powered from VBUS",   # 11
                  "Backup Mode",   # 12
                  "Reserved",   # 13
                  "Reserved",   # 14
                  "Reserved"   # 15
                    ]

reg1d_bit_names: str = [
                  "VBAT_PRESENT_STAT",   # bit0
                  "DPDM_STAT",   # bit1
                  "TREG_STAT",   # bit2
                  "RESERVED",   # bit3
                  "RESERVED",   # bit4
                  "RESERVED",   # bit5
                  "ICO_STAT:0",   # bit6
                  "ICO_STAT:1"   # bit7
                ]

reg1d_description: str = [
                  "Battery present status (VBAT > VBAT_UVLOZ)",   # bit0
                  "D+/D- detection status bits",   # bit1
                  "IC thermal regulation status",   # bit2
                  "RESERVED",   # bit3
                  "RESERVED",   # bit4
                  "RESERVED",   # bit5
                  "Input Current Optimizer (ICO) status",   # bit6
                  ""   # bit7
                ]

reg1d_bits_set:str = [
                  "VBAT present",   # bit0
                  "The D+/D- detection is ongoing",   # bit1
                  "Device in thermal regulation",   # bit2
                  "n/a",   # bit3
                  "n/a",   # bit4
                  "n/a",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg1d_bits_unset: str = [
                  "VBAT NOT present",   # bit0
                  "The D+/D- detection is NOT started yet, or the detection is done",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "n/a",   # bit4
                  "n/a",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg1d_bits_ICO_STAT: str = [
                  "ICO disabled",   # 0
                  "ICO optimization in progress",   # 1
                  "Maximum input current detected",   # 2
                  "Reserved",   # 3
                    ]

reg1e_bit_names: str = [
                  "RESERVED",   # bit0
                  "PRECHG_TMR_STAT",   # bit1
                  "TRICHG_TMR_STAT",   # bit2
                  "CHG_TMR_STAT",   # bit3
                  "VSYS_STAT",   # bit4
                  "ADC_DONE_STAT",   # bit5
                  "ACRB1_STAT",   # bit6
                  "ACRB2_STAT"   # bit7
                ]

reg1e_description: str = [
                  "RESERVED",   # bit0
                  "Pre-charge timer status",   # bit1
                  "Trickle charge timer status",   # bit2
                  "Fast charge timer status",   # bit3
                  "VSYS Regulation Status (forward mode)",   # bit4
                  "ADC Conversion Status (in one-shot mode only)",   # bit5
                  "The ACFET1-RBFET1 status",   # bit6
                  "The ACFET2-RBFET2 status"   # bit7
                ]

reg1e_bits_set:str = [
                  "n/a",   # bit0
                  "Safety timer expired",   # bit1
                  "Safety timer expired",   # bit2
                  "Safety timer expired",   # bit3
                  "In VSYSMIN regulation (VBAT < VSYSMIN)",   # bit4
                  "Conversion complete",   # bit5
                  "ACFET1-RBFET1 is placed",   # bit6
                  "ACFET2-RBFET2 is placed"   # bit7
                ]

reg1e_bits_unset: str = [
                  "n/a",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Not in VSYSMIN regulation (VBAT > VSYSMIN)",   # bit4
                  "Conversion NOT complete",   # bit5
                  "ACFET1-RBFET1 is NOT placed",   # bit6
                  "ACFET2-RBFET2 is NOT placed"   # bit7
                ]

reg1f_bit_names: str = [
                  "TS_HOT_STAT",   # bit0
                  "TS_WARM_STAT",   # bit1
                  "TS_COOL_STAT",   # bit2
                  "TS_COLD_STAT",   # bit3
                  "VBATOTG_LOW_STAT",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg1f_description: str = [
                  "The TS temperature is in the hot range, higher than T5",   # bit0
                  "The TS temperature is in the warm range, between T3 and T5",   # bit1
                  "The TS temperature is in the cool range, between T1 and T2",   # bit2
                  "The TS temperature is in the cold range, lower than T1",   # bit3
                  "The battery voltage is too low to enable OTG mode",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg1f_bits_set:str = [
                  "TS status is in hot range",   # bit0
                  "TS status is in warm range",   # bit1
                  "TS status is in cool range",   # bit2
                  "TS status is in cold range",   # bit3
                  "The battery volage is too low to enable the OTG operation",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg1f_bits_unset: str = [
                  "TS status is NOT in hot range",   # bit0
                  "TS status is NOT in warm range",   # bit1
                  "TS status is NOT in cool range",   # bit2
                  "TS status is NOT in cold range",   # bit3
                  "The battery voltage is high enough to enable the OTG operation",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg20_bit_names: str = [
                  "VAC1_OVP_STAT",   # bit0
                  "VAC2_OVP_STAT",   # bit1
                  "CONV_OCP_STAT",   # bit2
                  "IBAT_OCP_STAT",   # bit3
                  "IBUS_OCP_STAT",   # bit4
                  "VBAT_OVP_STAT",   # bit5
                  "IBAT_REG_STAT",   # bit6
                  "IBAT_REG_STAT"   # bit7
                ]

reg20_description: str = [
                  "VAC1 over-voltage status",   # bit0
                  "VAC2 over-voltage status",   # bit1
                  "Converter over current status",   # bit2
                  "IBAT over-current status",   # bit3
                  "IBUS over-current status",   # bit4
                  "VBAT over-voltage status",   # bit5
                  "VBUS over-voltage status",   # bit6
                  "IBAT regulation status"   # bit7
                ]

reg20_bits_set:str = [
                  "Device in over voltage protection",   # bit0
                  "Device in over voltage protection",   # bit1
                  "Converter in over current protection",   # bit2
                  "Device in over current protectio",   # bit3
                  "Device in over current protection",   # bit4
                  "Device in over voltage protection",   # bit5
                  "Device in over voltage protection",   # bit6
                  "Device in battery discharging current regulation"   # bit7
                ]

reg20_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg21_bit_names: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "TSHUT_STAT",   # bit2
                  "RESERVED",   # bit3
                  "OTG_UVP_STAT",   # bit4
                  "OTG_OVP_STAT",   # bit5
                  "VSYS_OVP_STAT",   # bit6
                  "VSYS_SHORT_STAT"   # bit7
                ]

reg21_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC temperature shutdown status",   # bit2
                  "RESERVED",   # bit3
                  "OTG under voltage status",   # bit4
                  "OTG over voltage status",   # bit5
                  "VSYS over-voltage status",   # bit6
                  "VSYS short circuit status"   # bit7
                ]

reg21_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Device in thermal shutdown protection",   # bit2
                  "n/a",   # bit3
                  "Device in OTG under voltage",   # bit4
                  "Device in OTG over-voltage",   # bit5
                  "Device in SYS over-voltage protection",   # bit6
                  "Device in SYS short circuit protection"   # bit7
                ]

reg21_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg22_bit_names: str = [
                  "VBUS_PRESENT_FLAG",   # bit0
                  "AC1_PRESENT_FLAG",   # bit1
                  "AC2_PRESENT_FLAG",   # bit2
                  "PG_FLAG",   # bit3
                  "POORSRC_FLAG",   # bit4
                  "WD_FLAG",   # bit5
                  "VINDPM_FLAG",   # bit6
                  "IINDPM_FLAG"   # bit7
                ]

reg22_description: str = [
                  "VBUS present flag",   # bit0
                  "VAC1 present flag",   # bit1
                  "VAC2 present flag",   # bit2
                  "Power good flag",   # bit3
                  "Poor source detection flag",   # bit4
                  "I2C watchdog timer flag",   # bit5
                  "VINDPM / VOTG Flag",   # bit6
                  "IINDPM / IOTG flag"   # bit7
                ]

reg22_bits_set:str = [
                  "VBUS present status changed",   # bit0
                  "VAC1 present status changed",   # bit1
                  "VAC2 present status changed",   # bit2
                  "Any change in PG_STAT even (adapter good qualification or adapter good going away)",   # bit3
                  "Poor source status rising edge detected",   # bit4
                  "WD timer signal rising edge detected",   # bit5
                  "VINDPM / VOTG regulation signal rising edge detected",   # bit6
                  "IINDPM / IOTG signal rising edge detected"   # bit7
                ]

reg22_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg23_bit_names: str = [
                  "BC1.2_DONE_FLAG",   # bit0
                  "VBAT_PRESENT_FLAG",   # bit1
                  "TREG_FLAG",   # bit2
                  "RESERVED",   # bit3
                  "VBUS_FLAG",   # bit4
                  "RESERVED",   # bit5
                  "ICO_FLAG",   # bit6
                  "CHG_FLAG"   # bit7
                ]

reg23_description: str = [
                  "BC1.2 status Flag",   # bit0
                  "BC1.2 status Flag",   # bit1
                  "IC thermal regulation flag",   # bit2
                  "RESERVED",   # bit3
                  "VBUS status flag",   # bit4
                  "RESERVED",   # bit5
                  "ICO status flag",   # bit6
                  "Charge status flag"   # bit7
                ]

reg23_bits_set:str = [
                  "BC1.2 detection status changed",   # bit0
                  "VBAT present status change",   # bit1
                  "TREG signal rising threshold detected",   # bit2
                  "n/a",   # bit3
                  "VBUS status changed",   # bit4
                  "n/a",   # bit5
                  "ICO status changed",   # bit6
                  "Charge status changed"   # bit7
                ]

reg23_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "n/a",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg24_bit_names: str = [
                  "TOPOFF_TMR_FLAG",   # bit0
                  "PRECHG_TMR_FLAG",   # bit1
                  "TRICHG_TMR_FLAG",   # bit2
                  "CHG_TMR_FLAG",   # bit3
                  "VSYS_FLAG",   # bit4
                  "ADC_DONE_FLAG",   # bit5
                  "DPDM_DONE_FLAG",   # bit6
                  "RESERVED"   # bit7
                ]

reg24_description: str = [
                  "Top off timer flag",   # bit0
                  "Pre-charge timer flag",   # bit1
                  "Trickle charge timer flag",   # bit2
                  "Fast charge timer flag",   # bit3
                  "VSYSMIN regulation flag",   # bit4
                  "ADC conversion flag (only in one-shot mode)",   # bit5
                  "D+/D- detection is done flag",   # bit6
                  "RESERVED"   # bit7
                ]

reg24_bits_set:str = [
                  "Top off timer expired rising edge detected",   # bit0
                  "Pre-charge timer expired rising edge detected",   # bit1
                  "Trickle charger timer expired rising edge detected",   # bit2
                  "Fast charge timer expired rising edge detected",   # bit3
                  "Entered or existed VSYSMIN regulation",   # bit4
                  "Conversion completed",   # bit5
                  "D+/D- detection is completed",   # bit6
                  "n/a"   # bit7
                ]

reg24_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Conversion NOT completed",   # bit5
                  "D+/D- detection is NOT started or still ongoing",   # bit6
                  "n/a"   # bit7
                ]

reg25_bit_names: str = [
                  "TS_HOT_FLAG",   # bit0
                  "TS_WARM_FLAG",   # bit1
                  "TS_COOL_FLAG",   # bit2
                  "TS_COLD_FLAG",   # bit3
                  "VBATOTG_LOW_FLAG",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg25_description: str = [
                  "TS hot temperature flag",   # bit0
                  "TS warm temperature flag",   # bit1
                  "TS cool temperature flag",   # bit2
                  "TS cold temperature flag",   # bit3
                  "VBAT too low to enable OTG flag",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg25_bits_set:str = [
                  "TS across hot temperature (T5) is detected",   # bit0
                  "TS across warm temperature (T3) is detected",   # bit1
                  "TS across cool temperature (T2) is detected",   # bit2
                  "TS across cold temperature (T1) is detected",   # bit3
                  "VBAT falls below the threshold to enable the OTG mode",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg25_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg26_bit_names: str = [
                  "VAC1_OVP_FLAG",   # bit0
                  "VAC2_OVP_FLAG",   # bit1
                  "CONV_OCP_FLAG",   # bit2
                  "IBAT_OCP_FLAG",   # bit3
                  "IBUS_OCP_FLAG",   # bit4
                  "VBAT_OVP_FLAG",   # bit5
                  "VBUS_OVP_FLAG",   # bit6
                  "IBAT_REG_FLAG"   # bit7
                ]

reg26_description: str = [
                  "VAC1 over-voltage flag",   # bit0
                  "VAC2 over-voltage flag",   # bit1
                  "Converter over-current flag",   # bit2
                  "IBAT over-current flag",   # bit3
                  "IBUS over-current flag",   # bit4
                  "VBAT over-voltage flag",   # bit5
                  "VBUS over-voltage flag",   # bit6
                  "IBAT regulation flag"   # bit7
                ]

reg26_bits_set:str = [
                  "Enter VAC1 OVP",   # bit0
                  "Enter VAC2 OVP",   # bit1
                  "Enter converter OCP",   # bit2
                  "Enter discharged OCP",   # bit3
                  "Enter IBUS OCP",   # bit4
                  "Enter VBAT OVP",   # bit5
                  "Enter VBUS OVP",   # bit6
                  "Enter or exit IBAT regulation"   # bit7
                ]

reg26_bits_unset: str = [
                  "Normal",   # bit0
                  "Normal",   # bit1
                  "Normal",   # bit2
                  "Normal",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg27_bit_names: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "TSHUT_FLAG",   # bit2
                  "RESERVED",   # bit3
                  "OTG_UVP_FLAG",   # bit4
                  "OTG_OVP_FLAG",   # bit5
                  "VSYS_OVP_FLAG",   # bit6
                  "VSYS_SHORT_FLAG"   # bit7
                ]

reg27_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC thermal shutdown flag",   # bit2
                  "RESERVED",   # bit3
                  "OTG under-voltage flag",   # bit4
                  "OTG over-voltage flag",   # bit5
                  "VSYS over-voltage flag",   # bit6
                  "VSYS short circuit flag"   # bit7
                ]

reg27_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TS shutdown signal rising threshold detected",   # bit2
                  "n/a",   # bit3
                  "Stop OTG due to VBUS under-voltage",   # bit4
                  "Stop OTG due to VBUS over voltage",   # bit5
                  "Stop switching due to system over-voltage",   # bit6
                  "Stop switching due to system short"   # bit7
                ]

reg27_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Normal",   # bit2
                  "n/a",   # bit3
                  "Normal",   # bit4
                  "Normal",   # bit5
                  "Normal",   # bit6
                  "Normal"   # bit7
                ]

reg28_bit_names: str = [
                  "VBUS_PRESENT_MASK",   # bit0
                  "AC1_PRESENT_MASK",   # bit1
                  "AC2_PRESENT_MASK",   # bit2
                  "PG_MASK",   # bit3
                  "POORSRC_MASK",   # bit4
                  "WD_MASK",   # bit5
                  "VINDPM_MASK",   # bit6
                  "IINDPM_MASK"   # bit7
                ]

reg28_description: str = [
                  "VBUS present mask flag",   # bit0
                  "VAC1 present mask flag",   # bit1
                  "VAC2 present mask flag",   # bit2
                  "Power Good mask flag",   # bit3
                  "Poor source detection mask flag",   # bit4
                  "I2C watch dog timer mask flag",   # bit5
                  "VINDPM / VOTG mask flag",   # bit6
                  "IINDPM / IOTG mask flag"   # bit7
                ]

reg28_bits_set:str = [
                  "VBUS present status change does NOT produce INT",   # bit0
                  "VAC1 present status change does NOT produce INT",   # bit1
                  "VAC2 present status change does NOT produce INT",   # bit2
                  "PG toggle does NOT produce INT",   # bit3
                  "Poor source detected does NOT produce INT",   # bit4
                  "I2C watch dog timer expired does produce INT pulse",   # bit5
                  "Enter VINDPM / VOTG does NOT produce INT pulse",   # bit6
                  "Enter IINDPM / IOTG does NOT produce INT pulse"   # bit7
                ]

reg28_bits_unset: str = [
                  "VBUS present status change does produce INT",   # bit0
                  "VAC1 present status change does produce INT",   # bit1
                  "VAC2 present status change does produce INT",   # bit2
                  "PG toggle does produce INT",   # bit3
                  "Poor source detected does produce INT",   # bit4
                  "I2C watch dog timer expired does produce INT pulse",   # bit5
                  "Enter VINDPM / VOTG does produce INT pulse",   # bit6
                  "Enter IINDPM / IOTG does produce INT pulse"   # bit7
                ]

reg29_bit_names: str = [
                  "BC1.2_DONE_MASK",   # bit0
                  "VBAT_PRESENT_MASK",   # bit1
                  "TREG_MASK",   # bit2
                  "RESERVED",   # bit3
                  "VBUS_MASK",   # bit4
                  "RESERVED",   # bit5
                  "ICO_MASK",   # bit6
                  "CHG_MASK"   # bit7
                ]

reg29_description: str = [
                  "BC1.2 status mask flag",   # bit0
                  "VBAT present mask flag",   # bit1
                  "IC thermal regulation mask flag",   # bit2
                  "RESERVED",   # bit3
                  "VBUS status mask flag",   # bit4
                  "RESERVED",   # bit5
                  "ICO status mask flag",   # bit6
                  "Charge status mask flag"   # bit7
                ]

reg29_bits_set:str = [
                  "BC1.2 status change does NOT produce INT",   # bit0
                  "VBAT present status change does NOT produce INT",   # bit1
                  "entering TREG does NOT produce INT",   # bit2
                  "n/a",   # bit3
                  "VBUS status change does NOT produce INT",   # bit4
                  "n/a",   # bit5
                  "ICO status change does NOT produce INT",   # bit6
                  "ICO status change does NOT produce INT"   # bit7
                ]

reg29_bits_unset: str = [
                  "BC1.2 status change does produce INT",   # bit0
                  "VBAT present status change does produce INT",   # bit1
                  "entering TREG does produce INT",   # bit2
                  "n/a",   # bit3
                  "VBUS status change does produce INT",   # bit4
                  "n/a",   # bit5
                  "ICO status change does produce INT",   # bit6
                  "ICO status change does produce INT"   # bit7
                ]

reg2a_bit_names: str = [
                  "TOPOFF_TMR_MASK",   # bit0
                  "PRECHG_TMR_MASK",   # bit1
                  "TRICHG_TMR_MASK",   # bit2
                  "CHG_TMR_MASK",   # bit3
                  "VSYS_MASK",   # bit4
                  "ADC_DONE_MASK",   # bit5
                  "DPDM_DONE_MASK",   # bit6
                  "RESERVED"   # bit7
                ]

reg2a_description: str = [
                  "Top off timer mask flag",   # bit0
                  "Pre-charge timer mask flag",   # bit1
                  "Trickle charge timer mask flag",   # bit2
                  "Fast charge timer mask flag",   # bit3
                  "VSYS min regulation mask flag",   # bit4
                  "ADC conversion mask flag (only in one-shot mode)",   # bit5
                  "D+/D- detection is done mask flag",   # bit6
                  "RESERVED"   # bit7
                ]

reg2a_bits_set:str = [
                  "Top off timer expire does NOT produce INT",   # bit0
                  "Pre-charge timer expire does NOT produce INT",   # bit1
                  "Trickle charge timer expire does NOT produce INT",   # bit2
                  "Fast charge timer expire does NOT produce INT",   # bit3
                  "enter or exit VSYSMIN regulation does NOT produce INT pulse",   # bit4
                  "ADC conversion done does NOT produce INT pulse",   # bit5
                  "D+/D- detection done does NOT produce INT pulse",   # bit6
                  "n/a"   # bit7
                ]

reg2a_bits_unset: str = [
                  "Top off timer expire does produce INT",   # bit0
                  "Pre-charge timer expire does produce INT",   # bit1
                  "Trickle charge timer expire does produce INT",   # bit2
                  "Fast charge timer expire does produce INT",   # bit3
                  "enter or exit VSYSMIN regulation does produce INT pulse",   # bit4
                  "ADC conversion done does produce INT pulse",   # bit5
                  "D+/D- detection done does produce INT pulse",   # bit6
                  "n/a"   # bit7
                ]

reg2b_bit_names: str = [
                  "TS_HOT_MASK",   # bit0
                  "TS_WARM_MASK",   # bit1
                  "TS_COOL_MASK",   # bit2
                  "TS_COLD_MASK",   # bit3
                  "VBATOTG_LOW_MASK",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg2b_description: str = [
                  "TS hot temperature interrupt mask",   # bit0
                  "TS warm temperature interrupt mask",   # bit1
                  "TS cool temperature interrupt mask",   # bit2
                  "VBAT too low to enable OTG mask",   # bit3
                  "VBAT too low to enable OTG mask",   # bit4
                  "RESERVED",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg2b_bits_set:str = [
                  "TS across hot temperature (T5) does NOT produce INT",   # bit0
                  "TS across warm temperature (T3) does NOT produce INT",   # bit1
                  "TS across cool temperature (T2) does NOT produce INT",   # bit2
                  "TS across cold temperature (T1) does NOT produce INT",   # bit3
                  "VBAT falling below the threshold to enable the OTG mode, does NOT produce INT",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg2b_bits_unset: str = [
                  "TS across hot temperature (T5) does produce INT",   # bit0
                  "TS across warm temperature (T3) does produce INT",   # bit1
                  "TS across cool temperature (T2) does produce INT",   # bit2
                  "TS across cold temperature (T1) does produce INT",   # bit3
                  "VBAT falling below the threshold to enable the OTG mode, does produce INT",   # bit4
                  "n/a",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg2c_bit_names: str = [
                  "VAC1_OVP_MASK",   # bit0
                  "VAC2_OVP_MASK",   # bit1
                  "CONV_OCP_MASK",   # bit2
                  "IBAT_OCP_MASK",   # bit3
                  "IBUS_OCP_MASK",   # bit4
                  "VBAT_OVP_MASK",   # bit5
                  "VBUS_OVP_MASK",   # bit6
                  "IBAT_REG_MASK"   # bit7
                ]

reg2c_description: str = [
                  "VAC1 over-voltage mask flag",   # bit0
                  "VAC2 over-voltage mask flag",   # bit1
                  "Converter over-current mask flag",   # bit2
                  "IBAT over-current mask flag",   # bit3
                  "IBUS over-current mask flag",   # bit4
                  "VBAT over-voltage mask flag",   # bit5
                  "VBUS over-voltage mask flag",   # bit6
                  "IBAT regulation mask flag"   # bit7
                ]

reg2c_bits_set:str = [
                  "entering VAC1 OVP does NOT produce INT",   # bit0
                  "entering VAC2 OVP does NOT produce INT",   # bit1
                  "Converter OCP fault does NOT produce INT",   # bit2
                  "IBAT OCP fault does NOT produce INT",   # bit3
                  "IBUS OCP fault does NOT produce INT",   # bit4
                  "entering VBAT OVP does NOT produce INT",   # bit5
                  "entering VBUS OVP does NOT produce INT",   # bit6
                  "enter or exit IBAT regulation does NOT produce INT"   # bit7
                ]

reg2c_bits_unset: str = [
                  "entering VAC1 OVP does produce INT",   # bit0
                  "entering VAC2 OVP does produce INT",   # bit1
                  "Converter OCP fault does produce INT",   # bit2
                  "IBAT OCP fault does produce INT",   # bit3
                  "IBUS OCP fault does produce INT",   # bit4
                  "entering VBAT OVP does produce INT",   # bit5
                  "entering VBUS OVP does produce INT",   # bit6
                  "enter or exit IBAT regulation does produce INT"   # bit7
                ]

reg2d_bit_names: str = [
                  "VAC1_OVP_MASK",   # bit0
                  "VAC2_OVP_MASK",   # bit1
                  "CONV_OCP_MASK",   # bit2
                  "IBAT_OCP_MASK",   # bit3
                  "IBUS_OCP_MASK",   # bit4
                  "VBUS_OVP_MASK",   # bit5
                  "VBUS_OVP_MASK",   # bit6
                  "IBAT_REG_MASK"   # bit7
                ]

reg2d_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "IC thermal shutdown mask flag",   # bit2
                  "RESERVED",   # bit3
                  "OTG under-voltage mask flag",   # bit4
                  "OTG over-voltage mask flag",   # bit5
                  "VSYS over-voltage mask flag",   # bit6
                  "VSYS short circuit mask flag"   # bit7
                ]

reg2d_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TSHUT does NOT produce INT",   # bit2
                  "n/a",   # bit3
                  "OTG VBUS under voltage fault does NOT produce INT",   # bit4
                  "OTG VBUS over-voltage fault does NOT produce INT",   # bit5
                  "System over-voltage fault does NOT produce INT",   # bit6
                  "System short fault does NOT produce INT"   # bit7
                ]

reg2d_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "TSHUT does produce INT",   # bit2
                  "n/a",   # bit3
                  "OTG VBUS under voltage fault does produce INT",   # bit4
                  "OTG VBUS over-voltage fault does produce INT",   # bit5
                  "System over-voltage fault does produce INT",   # bit6
                  "System short fault does produce INT"   # bit7
                ]

reg2e_bit_names: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "ADC_AVG_INIT",   # bit2
                  "ADC_AVG",   # bit3
                  "ADC_SAMPLE:0",   # bit4
                  "ADC_SAMPLE:1",   # bit5
                  "ADC_RATE",   # bit6
                  "ADC_EN"   # bit7
                ]

reg2e_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "ADC average initial value control",   # bit2
                  "ADC average control",   # bit3
                  "ADC sample speed",   # bit4
                  "ADC sample speed",   # bit5
                  "ADC conversion rate control",   # bit6
                  "ADC Control"   # bit7
                ]

reg2e_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Start average using a new ADC conversion",   # bit2
                  " Running average (not available for IBAT discharge)",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "One shot conversion",   # bit6
                  "enable"   # bit7
                ]

reg2e_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "Start average using the existing register value",   # bit2
                  "Single value",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "Continuous conversion",   # bit6
                  "disable"   # bit7
                ]

reg2e_bits_ADC_SAMPLE: str = [
                  "15 bit effective resolution",   # 0
                  "14 bit effective resolution",   # 1
                  "13 bit effective resolution",   # 2
                  "12 bit effective resolution (not recommended)",   # 3
                  "",   # 4
                  "",   # 5
                  "",   # 6
                  ""   # 7
                    ]

reg2f_bit_names: str = [
                  "RESERVED",   # bit0
                  "TDIE_ADC_DIS",   # bit1
                  "TS_ADC_DIS",   # bit2
                  "VSYS_ADC_DIS",   # bit3
                  "VBAT_ADC_DIS",   # bit4
                  "VBUS_ADC_DIS",   # bit5
                  "IBAT_ADC_DIS",   # bit6
                  "IBUS_ADC_DIS"   # bit7
                ]

reg2f_description: str = [
                  "RESERVED",   # bit0
                  "TDIE ADC control",   # bit1
                  "TS ADC control",   # bit2
                  "VSYS ADC control",   # bit3
                  "VBAT ADC control",   # bit4
                  "VBUS ADC control",   # bit5
                  "IBAT ADC control",   # bit6
                  "IBUS ADC control"   # bit7
                ]

reg2f_bits_set:str = [
                  "n/a",   # bit0
                  "disable",   # bit1
                  "disable",   # bit2
                  "disable",   # bit3
                  "disable",   # bit4
                  "disable",   # bit5
                  "disable",   # bit6
                  "disable"   # bit7
                ]

reg2f_bits_unset: str = [
                  "n/a",   # bit0
                  "enable (default)",   # bit1
                  "enable (default)",   # bit2
                  "enable (default)",   # bit3
                  "enable (default)",   # bit4
                  "enable (default)",   # bit5
                  "enable (default)",   # bit6
                  "enable (default)"   # bit7
                ]

reg30_bit_names: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "RESERVED",   # bit2
                  "RESERVED",   # bit3
                  "VAC1_ADC_DIS:0",   # bit4
                  "VAC2_ADC_DIS:1",   # bit5
                  "DM_ADC_DIS:0",   # bit6
                  "DP_ADC_DIS:1"   # bit7
                ]

reg30_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "RESERVED",   # bit2
                  "RESERVED",   # bit3
                  "VAC1 ADC Control",   # bit4
                  "VAC2 ADC Control",   # bit5
                  "D- ADC Control",   # bit6
                  "D+ ADC Control"   # bit7
                ]

reg30_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "n/a",   # bit2
                  "n/a",   # bit3
                  "disable",   # bit4
                  "disable",   # bit5
                  "disable",   # bit6
                  "disable"   # bit7
                ]

reg30_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "n/a",   # bit2
                  "n/a",   # bit3
                  "enable (default)",   # bit4
                  "enable (default)",   # bit5
                  "enable (default)",   # bit6
                  "enable (default)"   # bit7
                ]

reg47_bit_names: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "DMINUS_DAC:0",   # bit2
                  "DMINUS_DAC:1",   # bit3
                  "DMINUS_DAC:2",   # bit4
                  "DPLUS_DAC:0",   # bit5
                  "DPLUS_DAC:1",   # bit6
                  "DPLUS_DAC:2"   # bit7
                ]

reg47_description: str = [
                  "RESERVED",   # bit0
                  "RESERVED",   # bit1
                  "D- Output Driver",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "D+ Output Driver",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg47_bits_set:str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg47_bits_unset: str = [
                  "n/a",   # bit0
                  "n/a",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

reg47_bits_DPLUS_DAC: str = [
                  "HIZ",   # 0
                  "0",   # 1
                  "0.6V",   # 2
                  "1.2V",   # 3
                  "2.0V",   # 4
                  "2.7V",   # 5
                  "3.3V",   # 6
                  "D+/D- Short"   # 7
                    ]

reg47_bits_DMINUS_DAC: str = [
                  "HIZ",   # 0
                  "0",   # 1
                  "0.6V",   # 2
                  "1.2V",   # 3
                  "2.0V",   # 4
                  "2.7V",   # 5
                  "3.3V",   # 6
                  "reserved"   # 7
                    ]

reg48_bit_names: str = [
                  "DEV_REV_2:0",   # bit0
                  "DEV_REV_2:1",   # bit1
                  "DEV_REV_2:2",   # bit2
                  "PN_2:0",   # bit3
                  "PN_2:1",   # bit4
                  "PN_2:@",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg48_description: str = [
                  "Device Revision",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "Device Part Number",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "RESERVED",   # bit6
                  "RESERVED"   # bit7
                ]

reg48_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg48_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "n/a",   # bit6
                  "n/a"   # bit7
                ]

reg48_bits_PN: str = [
                  "",   # 0
                  "",   # 1
                  "",   # 2
                  "BQ25798",   # 3
                    ]

reg48_bits_DEV_REV: str = [
                  "BQ25798",   # 0
                    ]

regXX_bit_names: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regXX_description: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regXX_bits_set:str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regXX_bits_unset: str = [
                  "",   # bit0
                  "",   # bit1
                  "",   # bit2
                  "",   # bit3
                  "",   # bit4
                  "",   # bit5
                  "",   # bit6
                  ""   # bit7
                ]

regxx_bits_fields = [
                  "",   # 0
                  "",   # 1
                  "",   # 2
                  "",   # 3
                  "",   # 4
                  "",   # 5
                  "",   # 6
                  ""   # 7
                    ]
