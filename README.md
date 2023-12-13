# keywords:

Magicshine, bicycle front light, MJ808, MJ-808, MJ818, MJ-818, bicycle rear light, CAN bus

BRANCH dwg

# 0. this all is still under heavy development
front and rear lamps do work together and can be used on a bike.
logic unit and auxiliary devices are still WIP.


# 1. abstract:
this undertaking originated as an attempt to redesign the MJ808/MJ818 lights by:
- designing a custom PCB w. driver/controller ICs,
- writing firmware for a stm32 microcontroller,
- encapsulating the above in MJ808's/MJ818's original housing,
- combining:
 - the rear light (MJ818), 
 - the front light (MJ808) and 
 - a control unit into an interconnected smart bicycle light system.
 - developing further auxiliary devices which extend functionality of the lights.
	
	
# 2. light system topology (in ascii art)
**==** and **||** denote the CAN bus, power lines are omitted.

## 2.1 - current working setup
front and rear lights are interconnected via the CAN bus; the front light activates the rear light via commands transmitted over CAN.

	(front light) - (rear light)
	------------------------------------------------
	(mj808) ======  (mj818)
	
## 2.2 - WIP setup
front and rear lights are extended in their functionality by other auxiliary devices interconnected via the CAN bus
	
	(front light) - (control unit) - (rear light)
	------------------------------------------------
	(mj808) ====== (control unit)  ====== (mj818)
                      ||		||
                      ||		||		
                      ||	(future device(s))
                    (dynamo)
	
	
# 3. description of the original light
google [magicshine mj-818](https://www.google.com/search?q=magicshine+mj-818) / [magicshine mj-808](https://www.google.com/search?q=magicshine+mj-808) for an idea what the lights are and look like.
	
these are decent made-in-china bicycle lights operated off a 2S2P Li-Lion battery pack.

their functionality, apart from producing light, sucks.

	
# 4. motivation for development
light operating mode selection and high power consumption on the original lights is what i would like to have different.
	
the only reasonable way to achieve this is to start from scratch.

	
# 5. required software for DIY building
- eagle for PCB layout & schematics,
- STM32CubeIDE for the code,
- one STLink or similar,
- some cheapo arduino/stm-based CAN capable dev. board for quick and dirty CAN testing
- SMD soldering equipment and skills,
- parts.
	
	
# 6. repo structure
- /datasheets/*/: contains what the name suggests,
- /dwg/: AutoCAD drawings & STLs of various sub-components,
- /eagle designs/: contains subfolders with eagle designs (.brd, .sch & BOMs) of various sub-components:
	- mj808/: the front light,
	- mj818/: the rear light,
	- mj828/: a small handlebar mounted UI (nothing fancy), WIP,
	- mj838/ (aka. Čos): a dynamo thing, WIP,
	- mjfoo/: a development board based on the mj8x8 infractructure,
	- branch-dependant: whatever stuff is in the particular branch.
- /mj8x8/: C source code for everything, to be used with STM32CubeIDE
- /saleae/: HLA for the logic analyzer
	
# 7. hardware
look in [/datasheets/](https://github.com/ejovrh/MJ808/tree/master/datasheets) for a BOM.

core components:
- 4-layer PCB manufactured via [OSH Park](https://oshpark.com/),
- [STM32F042G6U6](https://www.st.com/en/microcontrollers-microprocessors/stm32f042g6.html) microcontroller, 
- [TCAN334](https://www.ti.com/product/TCAN334) 3.3V CAN transciever,
- [MAX16819](https://www.maximintegrated.com/en/products/power/led-drivers/MAX16819.html) & [MAX16820](https://www.maximintegrated.com/en/products/power/led-drivers/MAX16820.html) LED drivers,
- [LDK320](https://www.st.com/en/power-management/ldk320.html) 5V LDO,
- 0402 passives and
- Cree [XM-L](https://www.cree.com/led-components/products/xlamp-leds-discrete/xlamp-xm-l) and [XP-E](https://www.cree.com/led-components/products/xlamp-leds-discrete/xlamp-xp-e) high power LEDs.

when chosing components the primary criterium was size, since everything needed to fit into existing housings.
maximizing light output to the extreme was not a design factor.

# 8. software
## 8.1 general overview
the language of choice is C, written in an object-oriented fashion:
- it uses HAL. (don't like it?  -> go away.),
- common core components translate into an [abstract base class](https://github.com/ejovrh/MJ808/tree/master/MJ808/MJ808/mj8x8) (implemented via C-structs), 
- concrete implementations (e.g. [mj808](https://github.com/ejovrh/MJ808/tree/master/MJ808/MJ808/mj808) ) translate into derived classes (again C-structs),
- methods are generally implemented via function pointers ([e.g. CAN driver message operations](https://github.com/ejovrh/MJ808/blob/master/MJ808/MJ808/mj8x8/mcp2515.h)),
- behaviour is sometimes achieved with [OO interfaces](https://github.com/ejovrh/MJ808/tree/master/MJ808/MJ808/led), 
- polymorphism is implemented manually via constructors ([ctor in mj808_t](https://github.com/ejovrh/MJ808/blob/master/MJ808/MJ808/mj808/mj808.c)) and
- information is hidden by nesting C-structs and placing them into either [.c](https://github.com/ejovrh/MJ808/blob/master/MJ808/MJ808/mj808/mj808.c) or [.h](https://github.com/ejovrh/MJ808/blob/master/MJ808/MJ808/mj808/mj808.h) files.

due to memory limitations there are only hints of SOLID and design patterns to be found.

code is commented in a somewhat reasonable manner.

## 8.2 detailed description
TODO

## 8.2.1 mj808 - front light


## 8.2.2 mj818 - rear light


## 8.2.3 mj828 - UI


## 8.2.4 mjfoo - development board


## 8.2.5 mj838 - power controller



