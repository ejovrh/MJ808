<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.4.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="5034800600 FPC Connector">
<packages>
<package name="SMT">
<smd name="4" x="0.25" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="3" x="-0.25" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="2" x="-0.75" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="5" x="0.75" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="6" x="1.25" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="1" x="-1.25" y="1.27" dx="0.7" dy="0.3" layer="1" rot="R90"/>
<smd name="GND1" x="-2.5" y="-1.73" dx="1" dy="0.6" layer="1" rot="R90"/>
<smd name="GND2" x="2.5" y="-1.73" dx="1" dy="0.6" layer="1" rot="R90"/>
<wire x1="2.6" y1="1.67" x2="2.6" y2="-1.13" width="0.127" layer="21"/>
<wire x1="-2.6" y1="1.67" x2="-2.6" y2="-1.13" width="0.127" layer="21"/>
<wire x1="-2.6" y1="1.67" x2="2.6" y2="1.67" width="0.127" layer="21"/>
<wire x1="-2" y1="-1.73" x2="2" y2="-1.73" width="0.127" layer="21"/>
<text x="0" y="2.54" size="1" layer="21" font="vector" align="center">&gt;NAME</text>
<text x="0" y="-3.81" size="1" layer="21" font="vector" align="center">&gt;VALUE</text>
<circle x="-2.54" y="2.54" radius="0.1" width="0.3" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="5034800600">
<pin name="1" x="10.16" y="7.62" length="middle" rot="R180"/>
<pin name="2" x="10.16" y="5.08" length="middle" rot="R180"/>
<pin name="3" x="10.16" y="2.54" length="middle" rot="R180"/>
<pin name="4" x="10.16" y="0" length="middle" rot="R180"/>
<pin name="5" x="10.16" y="-2.54" length="middle" rot="R180"/>
<pin name="6" x="10.16" y="-5.08" length="middle" rot="R180"/>
<pin name="GND" x="10.16" y="-10.16" length="middle" rot="R180"/>
<wire x1="5.08" y1="10.16" x2="-2.54" y2="10.16" width="0.254" layer="94"/>
<wire x1="-2.54" y1="10.16" x2="-2.54" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-12.7" x2="5.08" y2="-12.7" width="0.254" layer="94"/>
<wire x1="5.08" y1="-12.7" x2="5.08" y2="10.16" width="0.254" layer="94"/>
<text x="-2.54" y="12.7" size="1.27" layer="95">&gt;NAME</text>
<text x="-2.54" y="-15.24" size="1.27" layer="95">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="5034800600" prefix="CONN" uservalue="yes">
<gates>
<gate name="G$1" symbol="5034800600" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMT">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="GND" pad="GND1 GND2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="SPI 1.27mm 10pin header">
<packages>
<package name="SMT-JTAG">
<text x="6.35" y="-3.81" size="1" layer="21" font="vector" align="center">&gt;NAME</text>
<text x="6.35" y="3.81" size="1" layer="21" font="vector" align="center">&gt;VALUE</text>
<smd name="2_GND" x="-2.54" y="1.955" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="4_VCC" x="-1.27" y="1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="6_SPI_RST" x="0" y="1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="8_NC" x="1.27" y="1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="10_GND" x="2.54" y="1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="9_SPI_MOSI" x="2.54" y="-1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="7_NC" x="1.27" y="-1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="5_NC" x="0" y="-1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="3_SPI_MISO" x="-1.27" y="-1.95" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<smd name="1_SPI_SCK" x="-2.54" y="-1.94" dx="2.4" dy="0.76" layer="1" rot="R270"/>
<wire x1="-3" y1="-4.59" x2="3" y2="-4.59" width="0.127" layer="21"/>
<wire x1="3" y1="-4.59" x2="3" y2="-2.48" width="0.127" layer="21"/>
<wire x1="-3" y1="-4.59" x2="-3" y2="-2.48" width="0.127" layer="21"/>
<wire x1="-3.81" y1="2.54" x2="-6.08" y2="2.54" width="0.127" layer="21"/>
<wire x1="-6.08" y1="2.54" x2="-6.08" y2="-2.54" width="0.127" layer="21"/>
<wire x1="-6.08" y1="-2.54" x2="-3.81" y2="-2.54" width="0.127" layer="21"/>
<wire x1="3.81" y1="2.54" x2="6.08" y2="2.54" width="0.127" layer="21"/>
<wire x1="6.08" y1="2.54" x2="6.08" y2="-2.54" width="0.127" layer="21"/>
<wire x1="6.08" y1="-2.54" x2="3.81" y2="-2.54" width="0.127" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="10-PIN-SPI">
<pin name="SPI_SCK" x="12.7" y="7.62" length="middle" rot="R180"/>
<pin name="GND" x="12.7" y="-7.62" length="middle" direction="pwr" rot="R180"/>
<pin name="SPI_MOSI" x="12.7" y="5.08" length="middle" rot="R180"/>
<pin name="VCC" x="12.7" y="-5.08" length="middle" rot="R180"/>
<pin name="SPI_MISO" x="12.7" y="2.54" length="middle" rot="R180"/>
<pin name="RESET" x="12.7" y="0" length="middle" rot="R180"/>
<wire x1="7.62" y1="12.7" x2="-5.08" y2="12.7" width="0.254" layer="94"/>
<wire x1="-5.08" y1="12.7" x2="-5.08" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-12.7" x2="7.62" y2="-12.7" width="0.254" layer="94"/>
<wire x1="7.62" y1="-12.7" x2="7.62" y2="12.7" width="0.254" layer="94"/>
<text x="-2.54" y="15.24" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-17.78" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="SPI-1.27-10-PIN-HEADER" prefix="CONN">
<gates>
<gate name="G$1" symbol="10-PIN-SPI" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMT-JTAG">
<connects>
<connect gate="G$1" pin="GND" pad="2_GND 10_GND"/>
<connect gate="G$1" pin="RESET" pad="6_SPI_RST"/>
<connect gate="G$1" pin="SPI_MISO" pad="3_SPI_MISO"/>
<connect gate="G$1" pin="SPI_MOSI" pad="9_SPI_MOSI"/>
<connect gate="G$1" pin="SPI_SCK" pad="1_SPI_SCK"/>
<connect gate="G$1" pin="VCC" pad="4_VCC"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="MJ8X8" library="5034800600 FPC Connector" deviceset="5034800600" device="" value="lamp"/>
<part name="ATMEL_ICE" library="SPI 1.27mm 10pin header" deviceset="SPI-1.27-10-PIN-HEADER" device="" value="programmer"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="MJ8X8" gate="G$1" x="53.34" y="-7.62" smashed="yes">
<attribute name="NAME" x="50.8" y="5.08" size="1.27" layer="95"/>
<attribute name="VALUE" x="50.8" y="-22.86" size="1.27" layer="95"/>
</instance>
<instance part="ATMEL_ICE" gate="G$1" x="48.26" y="63.5" smashed="yes">
<attribute name="NAME" x="45.72" y="78.74" size="1.778" layer="95"/>
<attribute name="VALUE" x="45.72" y="45.72" size="1.778" layer="96"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<wire x1="63.5" y1="0" x2="78.74" y2="0" width="0.1524" layer="91"/>
<label x="78.74" y="0" size="1.778" layer="95"/>
<pinref part="MJ8X8" gate="G$1" pin="1"/>
</segment>
<segment>
<pinref part="MJ8X8" gate="G$1" pin="GND"/>
<wire x1="63.5" y1="-17.78" x2="78.74" y2="-17.78" width="0.1524" layer="91"/>
<label x="78.74" y="-17.78" size="1.778" layer="95"/>
</segment>
<segment>
<wire x1="60.96" y1="55.88" x2="76.2" y2="55.88" width="0.1524" layer="91"/>
<label x="76.2" y="55.88" size="1.778" layer="95"/>
<pinref part="ATMEL_ICE" gate="G$1" pin="GND"/>
</segment>
</net>
<net name="ICSP_MOSI" class="0">
<segment>
<wire x1="63.5" y1="-2.54" x2="78.74" y2="-2.54" width="0.1524" layer="91"/>
<label x="78.74" y="-2.54" size="1.778" layer="95"/>
<pinref part="MJ8X8" gate="G$1" pin="2"/>
</segment>
<segment>
<pinref part="ATMEL_ICE" gate="G$1" pin="SPI_MOSI"/>
<wire x1="60.96" y1="68.58" x2="76.2" y2="68.58" width="0.1524" layer="91"/>
<label x="76.2" y="68.58" size="1.778" layer="95"/>
</segment>
</net>
<net name="ICSP_MISO" class="0">
<segment>
<wire x1="63.5" y1="-5.08" x2="78.74" y2="-5.08" width="0.1524" layer="91"/>
<label x="78.74" y="-5.08" size="1.778" layer="95"/>
<pinref part="MJ8X8" gate="G$1" pin="3"/>
</segment>
<segment>
<wire x1="60.96" y1="66.04" x2="76.2" y2="66.04" width="0.1524" layer="91"/>
<label x="76.2" y="66.04" size="1.778" layer="95"/>
<pinref part="ATMEL_ICE" gate="G$1" pin="SPI_MISO"/>
</segment>
</net>
<net name="ICSP_SCK" class="0">
<segment>
<wire x1="63.5" y1="-7.62" x2="78.74" y2="-7.62" width="0.1524" layer="91"/>
<label x="78.74" y="-7.62" size="1.778" layer="95"/>
<pinref part="MJ8X8" gate="G$1" pin="4"/>
</segment>
<segment>
<wire x1="60.96" y1="71.12" x2="76.2" y2="71.12" width="0.1524" layer="91"/>
<label x="76.2" y="71.12" size="1.778" layer="95"/>
<pinref part="ATMEL_ICE" gate="G$1" pin="SPI_SCK"/>
</segment>
</net>
<net name="DEBUGWIRE/RESET" class="0">
<segment>
<wire x1="63.5" y1="-12.7" x2="78.74" y2="-12.7" width="0.1524" layer="91"/>
<label x="78.74" y="-12.7" size="1.778" layer="95"/>
<pinref part="MJ8X8" gate="G$1" pin="6"/>
</segment>
<segment>
<pinref part="ATMEL_ICE" gate="G$1" pin="RESET"/>
<wire x1="60.96" y1="63.5" x2="76.2" y2="63.5" width="0.1524" layer="91"/>
<label x="76.2" y="63.5" size="1.778" layer="95"/>
</segment>
</net>
<net name="5V0" class="0">
<segment>
<pinref part="MJ8X8" gate="G$1" pin="5"/>
<wire x1="63.5" y1="-10.16" x2="78.74" y2="-10.16" width="0.1524" layer="91"/>
<label x="78.74" y="-10.16" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="ATMEL_ICE" gate="G$1" pin="VCC"/>
<wire x1="60.96" y1="58.42" x2="76.2" y2="58.42" width="0.1524" layer="91"/>
<label x="76.2" y="58.42" size="1.778" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
