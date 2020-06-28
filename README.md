keywords:

Magicshine, bicycle front light, MJ808, MJ-808, MJ818, MJ-818, bicycle rear light, CAN bus, MCP2515, MCP2561


0. this all is still under development


	front and rear lamps do work together and can be used on a bike.
	logic unit and auxiliary devices are still WIP.


1. abstract:	


	this undertaking is an attempt to redesign the MJ808/MJ818 lights by:
	
		- designing a custom PCB w. driver/controller ICs,
		- writing firmware the above controller IC,
		- encapsulating the above in MJ808's/MJ818's original housing,
		- combining:
				the rear light (MJ818), 
				the front light (MJ808) and 
				a control unit into an interconnected smart bicycle light system.
		- developing further auxiliary devices which extend functionality of the lights.
	
	
2. light system topology (in ascii art):


	"==" and "||" denote the CAN bus, power lines are omitted.

	2.1 - current working setup
	
	
	front and rear lights are interconnected via the CAN bus; the front light activates the rear light.

		front light	- rear light
		------------------------------------------------
		mj808 ======  mj818
	
	2.2 - WIP setup
	
	
	front and rear lights are extended in their functionality by other auxiliary devices interconnected via the CAN bus
	
		front light	-		controlling unit		-	rear light
		------------------------------------------------
		mj808 ====== control unit w. battery ====== mj818
                       ||		||
                       ||		||		
                       ||		future device(s)
                      dynamo
	
	
3. description of the original light:


	google "magicshine mj818" / "magicshine mj808" for an idea what the lights are and look like.
	
	these are decent made-in-china bicycle lights operated off a 2S2P Li-Lion battery pack.
	their functionality, apart from producing light, sucks.

	
4. motivation:


	operating mode selection, strobing and high power consumption on the original lights is what i would like to have different.
	
	the only reasonable way to achieve this is to start from scratch.

	
5. required software for building


	- eagle for PCB layout & schematics
	- atmel studio (or keil) for the code
	- atmel ICE or something similar
	- some cheapo arduino for quick and dirty CAN testing (the arduino acts as an improvised control unit)
	- SMD soldering equipment and skills
	
	
6. repo structure


	- /datasheets/: contains what the name suggests
	- /dwg/: AutoCAD drawings & STLs of various sub-components
	- /eagle designs/: contains subfolders with eagle designs (.brd, .sch & BOMs) of various sub-components
		- cos/: a dynamo thing, WIP
		- mj808/: the front light
		- mj818/: the rear light
		- mj828/: a small handlebar mounted UI (nothing fancy), WIP
		- drawings for the atmel ICE <-> light PCB programming adapter
		
	- /eagle libraries/: contains eagle libraries of electical components used
	- /gerber/: gerber files for PCB production
	- /MJ808/: C source code for everything

	
