<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="yes"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="dots" multiple="1" display="yes" altdistance="0.01" altunitdist="inch" altunit="inch"/>
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
<layer number="20" name="Dimension" color="24" fill="1" visible="no" active="no"/>
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
<library name="MCP2561-2 - High-Speed CAN Transceiver">
<packages>
<package name="DFN10">
<smd name="GND" x="0" y="0" dx="1.5" dy="2.4" layer="1" rot="R90"/>
<smd name="TXD" x="-0.975" y="-1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="VSS" x="-0.325" y="-1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="VDD" x="0.325" y="-1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="RXD" x="0.975" y="-1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="STBY" x="-0.975" y="1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="CANH" x="-0.325" y="1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="CANL" x="0.325" y="1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="SPLIT" x="0.975" y="1.55" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<wire x1="1.5" y1="-1.5" x2="1.5" y2="1.5" width="0.1016" layer="21"/>
<wire x1="-1.5" y1="1.5" x2="-1.5" y2="-1.5" width="0.1016" layer="21"/>
<circle x="-2" y="-1" radius="0.05" width="0.3" layer="21"/>
<text x="-3" y="0" size="1" layer="25" font="vector" rot="R90" align="center">&gt;NAME</text>
<text x="2.75" y="-0.25" size="1" layer="27" font="vector" rot="R90" align="center">&gt;VALUE</text>
<wire x1="1.5" y1="1.5" x2="1.8" y2="1.5" width="0.1016" layer="21"/>
<wire x1="-1.5" y1="1.5" x2="-1.8" y2="1.5" width="0.1016" layer="21"/>
<wire x1="-1.5" y1="-1.5" x2="-1.8" y2="-1.5" width="0.1016" layer="21"/>
<wire x1="1.5" y1="-1.5" x2="1.8" y2="-1.5" width="0.1016" layer="21"/>
</package>
<package name="SOIC">
<text x="-2.4" y="0" size="1" layer="25" font="vector" rot="R90" align="center">&gt;NAME</text>
<text x="2" y="0" size="1" layer="27" font="vector" rot="R90" align="center">&gt;VALUE</text>
<smd name="P$1" x="-0.975" y="-1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$2" x="-0.325" y="-1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$3" x="0.325" y="-1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$4" x="0.975" y="-1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$5" x="0.975" y="1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$6" x="0.325" y="1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$7" x="-0.325" y="1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<smd name="P$8" x="-0.975" y="1.575" dx="0.65" dy="0.35" layer="1" rot="R90"/>
<wire x1="-1.2" y1="0.8" x2="1.2" y2="0.8" width="0.127" layer="21"/>
<wire x1="1.2" y1="0.8" x2="1.2" y2="-0.8" width="0.127" layer="21"/>
<wire x1="1.2" y1="-0.8" x2="-1.2" y2="-0.8" width="0.127" layer="21"/>
<wire x1="-1.2" y1="-0.8" x2="-1.2" y2="0.8" width="0.127" layer="21"/>
<circle x="-1.4" y="-1" radius="0.1" width="0.2" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="MCP2561-2">
<pin name="TXD" x="-17.78" y="5.08" length="middle" direction="in"/>
<pin name="GND" x="-17.78" y="-7.62" length="middle" direction="pwr"/>
<pin name="VCC" x="-17.78" y="-5.08" length="middle" direction="pwr"/>
<pin name="RXD" x="-17.78" y="2.54" length="middle" direction="out"/>
<pin name="STBY" x="15.24" y="-5.08" length="middle" rot="R180"/>
<pin name="CANH" x="15.24" y="5.08" length="middle" rot="R180"/>
<pin name="CANL" x="15.24" y="2.54" length="middle" rot="R180"/>
<pin name="SPLIT" x="15.24" y="-7.62" length="middle" rot="R180"/>
<wire x1="-12.7" y1="-12.7" x2="10.16" y2="-12.7" width="0.254" layer="94"/>
<wire x1="10.16" y1="-12.7" x2="10.16" y2="10.16" width="0.254" layer="94"/>
<wire x1="10.16" y1="10.16" x2="-12.7" y2="10.16" width="0.254" layer="94"/>
<wire x1="-12.7" y1="10.16" x2="-12.7" y2="-12.7" width="0.254" layer="94"/>
<text x="-12.7" y="15.24" size="2.54" layer="95">&gt;NAME</text>
<text x="-12.7" y="-17.78" size="2.54" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="MCP2561-2" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="MCP2561-2" x="0" y="0"/>
</gates>
<devices>
<device name="" package="DFN10">
<connects>
<connect gate="G$1" pin="CANH" pad="CANH"/>
<connect gate="G$1" pin="CANL" pad="CANL"/>
<connect gate="G$1" pin="GND" pad="GND VSS"/>
<connect gate="G$1" pin="RXD" pad="RXD"/>
<connect gate="G$1" pin="SPLIT" pad="SPLIT"/>
<connect gate="G$1" pin="STBY" pad="STBY"/>
<connect gate="G$1" pin="TXD" pad="TXD"/>
<connect gate="G$1" pin="VCC" pad="VDD"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="SOIC" package="SOIC">
<connects>
<connect gate="G$1" pin="CANH" pad="P$7"/>
<connect gate="G$1" pin="CANL" pad="P$6"/>
<connect gate="G$1" pin="GND" pad="P$2"/>
<connect gate="G$1" pin="RXD" pad="P$4"/>
<connect gate="G$1" pin="SPLIT" pad="P$5"/>
<connect gate="G$1" pin="STBY" pad="P$8"/>
<connect gate="G$1" pin="TXD" pad="P$1"/>
<connect gate="G$1" pin="VCC" pad="P$3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AMB2621 - BLE Module">
<packages>
<package name="SMD">
<wire x1="-4" y1="6" x2="4" y2="6" width="0.127" layer="21"/>
<wire x1="4" y1="6" x2="4" y2="-5" width="0.127" layer="21"/>
<wire x1="4" y1="-5" x2="-4" y2="-5" width="0.127" layer="21"/>
<wire x1="-4" y1="-5" x2="-4" y2="6" width="0.127" layer="21"/>
<polygon width="0.127" layer="39">
<vertex x="4" y="6"/>
<vertex x="-2" y="6"/>
<vertex x="-2" y="2.75"/>
<vertex x="4" y="2.75"/>
</polygon>
<polygon width="0.127" layer="40">
<vertex x="-2" y="6"/>
<vertex x="4" y="6"/>
<vertex x="4" y="2.75"/>
<vertex x="-2" y="2.75"/>
</polygon>
<smd name="1_RF" x="-4" y="4.1" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="2_GND" x="-4" y="2.9" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="3_SWD-CLK" x="-4" y="1.7" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="4_SWD-IO" x="-4" y="0.5" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="5_RESET" x="-4" y="-0.7" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="6_BOOT" x="-4" y="-1.9" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="7_VDD" x="-4" y="-3.1" dx="1" dy="0.8" layer="1" rot="R180"/>
<smd name="8_OPMODE" x="-1.8" y="-5" dx="1" dy="0.8" layer="1" rot="R270"/>
<smd name="9_RESERVED" x="-0.6" y="-5" dx="1" dy="0.8" layer="1" rot="R270"/>
<smd name="10_LED1" x="0.6" y="-5" dx="1" dy="0.8" layer="1" rot="R270"/>
<smd name="11_LED2" x="1.8" y="-5" dx="1" dy="0.8" layer="1" rot="R270"/>
<smd name="12_UART_TX" x="4" y="-3.1" dx="1" dy="0.8" layer="1"/>
<smd name="13_UART_RX" x="4" y="-1.9" dx="1" dy="0.8" layer="1"/>
<smd name="14_RESERVED" x="4" y="-0.7" dx="1" dy="0.8" layer="1"/>
<smd name="15_RESERVED" x="4" y="0.5" dx="1" dy="0.8" layer="1"/>
<smd name="16_WAKE" x="4" y="1.7" dx="1" dy="0.8" layer="1"/>
<smd name="17_GND" x="4" y="2.9" dx="1" dy="0.8" layer="1"/>
<text x="0" y="7" size="1" layer="25" font="vector" rot="R180" align="center">&gt;NAME</text>
<text x="0" y="-7" size="1" layer="27" font="vector" rot="R180" align="center">&gt;VALUE</text>
<polygon width="0.127" layer="29">
<vertex x="-2" y="6"/>
<vertex x="4" y="6"/>
<vertex x="4" y="2.75"/>
<vertex x="-2" y="2.75"/>
</polygon>
<polygon width="0.127" layer="30">
<vertex x="-2" y="6"/>
<vertex x="4" y="6"/>
<vertex x="4" y="2.75"/>
<vertex x="-2" y="2.75"/>
</polygon>
</package>
</packages>
<symbols>
<symbol name="AMB2621">
<pin name="RF" x="17.78" y="10.16" length="middle" rot="R180"/>
<pin name="GND" x="-17.78" y="-12.7" length="middle" direction="pwr"/>
<pin name="SWD-CLK" x="17.78" y="-5.08" length="middle" direction="in" rot="R180"/>
<pin name="SWD-IO" x="17.78" y="-7.62" length="middle" rot="R180"/>
<pin name="RESET" x="17.78" y="-10.16" length="middle" direction="in" rot="R180"/>
<pin name="BOOT" x="17.78" y="-12.7" length="middle" direction="in" rot="R180"/>
<pin name="VDD" x="-17.78" y="-10.16" length="middle" direction="pwr"/>
<pin name="OPMODE" x="-17.78" y="2.54" length="middle" direction="in"/>
<pin name="LED1" x="-17.78" y="-5.08" length="middle" direction="out"/>
<pin name="LED2" x="-17.78" y="-2.54" length="middle" direction="out"/>
<pin name="UART_TX" x="-17.78" y="10.16" length="middle" direction="out"/>
<pin name="UART_RX" x="-17.78" y="7.62" length="middle" direction="in"/>
<pin name="WAKE" x="-17.78" y="5.08" length="middle" direction="in"/>
<wire x1="-12.7" y1="12.7" x2="-12.7" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-15.24" x2="12.7" y2="-15.24" width="0.254" layer="94"/>
<wire x1="12.7" y1="-15.24" x2="12.7" y2="12.7" width="0.254" layer="94"/>
<wire x1="12.7" y1="12.7" x2="-12.7" y2="12.7" width="0.254" layer="94"/>
<text x="0" y="15.24" size="1" layer="95" font="vector" align="center">&gt;NAME</text>
<text x="0" y="-17.78" size="1" layer="96" font="vector" align="center">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="AMB2621" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="AMB2621" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMD">
<connects>
<connect gate="G$1" pin="BOOT" pad="6_BOOT"/>
<connect gate="G$1" pin="GND" pad="2_GND 17_GND"/>
<connect gate="G$1" pin="LED1" pad="10_LED1"/>
<connect gate="G$1" pin="LED2" pad="11_LED2"/>
<connect gate="G$1" pin="OPMODE" pad="8_OPMODE"/>
<connect gate="G$1" pin="RESET" pad="5_RESET"/>
<connect gate="G$1" pin="RF" pad="1_RF"/>
<connect gate="G$1" pin="SWD-CLK" pad="3_SWD-CLK"/>
<connect gate="G$1" pin="SWD-IO" pad="4_SWD-IO"/>
<connect gate="G$1" pin="UART_RX" pad="13_UART_RX"/>
<connect gate="G$1" pin="UART_TX" pad="12_UART_TX"/>
<connect gate="G$1" pin="VDD" pad="7_VDD"/>
<connect gate="G$1" pin="WAKE" pad="16_WAKE"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AS5601 - Programmable Contactless Encoder">
<packages>
<package name="SOIC8">
<text x="-4" y="0" size="1.27" layer="21" font="vector" rot="R90" align="center">&gt;NAME</text>
<text x="3.5" y="0" size="1.27" layer="27" font="vector" rot="R90" align="center">&gt;VALUE</text>
<wire x1="-2.6" y1="2" x2="2.6" y2="2" width="0.127" layer="21"/>
<wire x1="-2.6" y1="-2" x2="2.6" y2="-2" width="0.127" layer="21"/>
<wire x1="-2.6" y1="2" x2="-2.6" y2="-2" width="0.127" layer="21"/>
<wire x1="2.6" y1="2" x2="2.6" y2="-2" width="0.127" layer="21"/>
<wire x1="-0.4" y1="0" x2="0.4" y2="0" width="0.127" layer="21"/>
<wire x1="0" y1="0.2" x2="0" y2="-0.2" width="0.127" layer="21"/>
<smd name="P$1" x="-1.905" y="-2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$2" x="-0.635" y="-2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$3" x="0.635" y="-2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$4" x="1.905" y="-2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$5" x="1.905" y="2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$6" x="0.635" y="2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$7" x="-0.635" y="2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$8" x="-1.905" y="2.91" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<circle x="-3" y="-2.5" radius="0.1" width="0.2" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="AS5601">
<pin name="VDD5V" x="12.7" y="-7.62" length="middle" direction="pwr" rot="R180"/>
<pin name="VDD3V3" x="12.7" y="-10.16" length="middle" direction="pwr" rot="R180"/>
<pin name="PUSH" x="12.7" y="10.16" length="middle" direction="out" rot="R180"/>
<pin name="GND" x="12.7" y="-12.7" length="middle" direction="pwr" rot="R180"/>
<pin name="B" x="12.7" y="5.08" length="middle" direction="out" rot="R180"/>
<pin name="SDA" x="12.7" y="0" length="middle" rot="R180"/>
<pin name="SCL" x="12.7" y="-2.54" length="middle" direction="in" rot="R180"/>
<pin name="A" x="12.7" y="7.62" length="middle" direction="out" rot="R180"/>
<wire x1="7.62" y1="12.7" x2="7.62" y2="-15.24" width="0.254" layer="94"/>
<wire x1="7.62" y1="-15.24" x2="-5.08" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-15.24" x2="-5.08" y2="12.7" width="0.254" layer="94"/>
<wire x1="-5.08" y1="12.7" x2="7.62" y2="12.7" width="0.254" layer="94"/>
<text x="0" y="15.24" size="1.778" layer="95" align="center">&gt;NAME</text>
<text x="0" y="-17.78" size="1.778" layer="96" align="center">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="AS5601" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="AS5601" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOIC8">
<connects>
<connect gate="G$1" pin="A" pad="P$8"/>
<connect gate="G$1" pin="B" pad="P$5"/>
<connect gate="G$1" pin="GND" pad="P$4"/>
<connect gate="G$1" pin="PUSH" pad="P$3"/>
<connect gate="G$1" pin="SCL" pad="P$7"/>
<connect gate="G$1" pin="SDA" pad="P$6"/>
<connect gate="G$1" pin="VDD3V3" pad="P$2"/>
<connect gate="G$1" pin="VDD5V" pad="P$1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="4559A - PN Fet">
<packages>
<package name="SO-8">
<text x="-4" y="0" size="1" layer="25" font="vector" rot="R90" align="center">&gt;NAME</text>
<text x="4" y="0" size="1" layer="27" font="vector" rot="R90" align="center">&gt;VALUE</text>
<wire x1="2.5" y1="2" x2="2.5" y2="-2" width="0.127" layer="21"/>
<wire x1="-2.5" y1="-2" x2="-2.5" y2="2" width="0.127" layer="21"/>
<smd name="P$1" x="-1.92" y="-2.6" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$2" x="-0.65" y="-2.6" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$3" x="0.62" y="-2.6" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$4" x="1.89" y="-2.6" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$5" x="1.89" y="2.58" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$6" x="0.62" y="2.58" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$7" x="-0.65" y="2.58" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<smd name="P$8" x="-1.92" y="2.58" dx="1.55" dy="0.6" layer="1" rot="R90"/>
<wire x1="-2.5" y1="2" x2="-2.25" y2="2" width="0.127" layer="21"/>
<wire x1="-1.5" y1="2" x2="-1" y2="2" width="0.127" layer="21"/>
<wire x1="-0.25" y1="2" x2="0.25" y2="2" width="0.127" layer="21"/>
<wire x1="1" y1="2" x2="1.5" y2="2" width="0.127" layer="21"/>
<wire x1="2.25" y1="2" x2="2.5" y2="2" width="0.127" layer="21"/>
<wire x1="-2.5" y1="-2" x2="-2.25" y2="-2" width="0.127" layer="21"/>
<wire x1="-1.5" y1="-2" x2="-1" y2="-2" width="0.127" layer="21"/>
<wire x1="-0.25" y1="-2" x2="0.25" y2="-2" width="0.127" layer="21"/>
<wire x1="1" y1="-2" x2="1.5" y2="-2" width="0.127" layer="21"/>
<wire x1="2.25" y1="-2" x2="2.5" y2="-2" width="0.127" layer="21"/>
<circle x="-3" y="-3" radius="0.2" width="0.3" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="4559A">
<pin name="S1" x="-5.08" y="-10.16" length="middle" rot="R90"/>
<pin name="G1" x="-17.78" y="0" length="middle"/>
<pin name="S2" x="5.08" y="10.16" length="middle" rot="R270"/>
<pin name="G2" x="17.78" y="0" length="middle" rot="R180"/>
<pin name="D1" x="-5.08" y="10.16" length="middle" rot="R270"/>
<pin name="D2" x="5.08" y="-10.16" length="middle" rot="R90"/>
<wire x1="-12.7" y1="-5.08" x2="0" y2="-5.08" width="0.254" layer="94"/>
<wire x1="0" y1="-5.08" x2="12.7" y2="-5.08" width="0.254" layer="94"/>
<wire x1="12.7" y1="-5.08" x2="12.7" y2="5.08" width="0.254" layer="94"/>
<wire x1="12.7" y1="5.08" x2="0" y2="5.08" width="0.254" layer="94"/>
<wire x1="0" y1="5.08" x2="-12.7" y2="5.08" width="0.254" layer="94"/>
<wire x1="-12.7" y1="5.08" x2="-12.7" y2="-5.08" width="0.254" layer="94"/>
<text x="-10.16" y="-2.54" size="1.27" layer="94" font="vector" align="center">N</text>
<text x="10.16" y="-2.54" size="1.27" layer="94" font="vector" align="center">P</text>
<wire x1="0" y1="5.08" x2="0" y2="-5.08" width="0.254" layer="94" style="shortdash"/>
<text x="10.16" y="7.62" size="1.27" layer="95" font="vector" align="center">&gt;NAME</text>
<text x="10.16" y="-7.62" size="1.27" layer="96" font="vector" align="center">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="4559A" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="4559A" x="5.08" y="-12.7"/>
</gates>
<devices>
<device name="" package="SO-8">
<connects>
<connect gate="G$1" pin="D1" pad="P$7 P$8"/>
<connect gate="G$1" pin="D2" pad="P$5 P$6"/>
<connect gate="G$1" pin="G1" pad="P$2"/>
<connect gate="G$1" pin="G2" pad="P$4"/>
<connect gate="G$1" pin="S1" pad="P$1"/>
<connect gate="G$1" pin="S2" pad="P$3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="PIC33">
<packages>
<package name="QFN44">
<wire x1="-4" y1="4" x2="4" y2="4" width="0.127" layer="21"/>
<wire x1="4" y1="4" x2="4" y2="-4" width="0.127" layer="21"/>
<wire x1="4" y1="-4" x2="-4" y2="-4" width="0.127" layer="21"/>
<wire x1="-4" y1="-4" x2="-4" y2="4" width="0.127" layer="21"/>
<text x="0" y="5.7" size="1" layer="25" font="vector" align="center">&gt;NAME</text>
<text x="0" y="-6.05" size="1" layer="27" font="vector" align="center">&gt;VALUE</text>
<circle x="-5" y="5" radius="0.2" width="0.3" layer="21"/>
<smd name="P$1" x="-4.55" y="3.25" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$2" x="-4.55" y="2.6" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$3" x="-4.55" y="1.95" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$4" x="-4.55" y="1.3" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$5" x="-4.55" y="0.65" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$6" x="-4.55" y="0" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$7" x="-4.55" y="-0.65" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$8" x="-4.55" y="-1.3" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$9" x="-4.55" y="-1.95" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$10" x="-4.55" y="-2.6" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$11" x="-4.55" y="-3.25" dx="0.35" dy="0.85" layer="1" rot="R90"/>
<smd name="P$12" x="-3.25" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$13" x="-2.6" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$14" x="-1.95" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$15" x="-1.3" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$16" x="-0.65" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$17" x="0" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$18" x="0.65" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$19" x="1.3" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$20" x="1.95" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$21" x="2.6" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$22" x="3.25" y="-4.55" dx="0.35" dy="0.85" layer="1" rot="R180"/>
<smd name="P$23" x="4.55" y="-3.25" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$24" x="4.55" y="-2.6" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$25" x="4.55" y="-1.95" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$26" x="4.55" y="-1.3" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$27" x="4.55" y="-0.65" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$28" x="4.55" y="0" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$29" x="4.55" y="0.65" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$30" x="4.55" y="1.3" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$31" x="4.55" y="1.95" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$32" x="4.55" y="2.6" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$33" x="4.55" y="3.25" dx="0.35" dy="0.85" layer="1" rot="R270"/>
<smd name="P$34" x="3.25" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$35" x="2.6" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$36" x="1.95" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$37" x="1.3" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$38" x="0.65" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$39" x="0" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$40" x="-0.65" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$41" x="-1.3" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$42" x="-1.95" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$43" x="-2.6" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$44" x="-3.25" y="4.55" dx="0.35" dy="0.85" layer="1"/>
<smd name="P$45" x="0" y="0" dx="6.6" dy="6.6" layer="1"/>
</package>
</packages>
<symbols>
<symbol name="PIC33">
<pin name="P$1" x="43.18" y="60.96" length="middle" rot="R180"/>
<pin name="P$2" x="43.18" y="58.42" length="middle" rot="R180"/>
<pin name="P$3" x="43.18" y="55.88" length="middle" rot="R180"/>
<pin name="P$4" x="43.18" y="53.34" length="middle" rot="R180"/>
<pin name="P$5" x="43.18" y="50.8" length="middle" rot="R180"/>
<pin name="P$6" x="43.18" y="48.26" length="middle" rot="R180"/>
<pin name="P$7" x="43.18" y="45.72" length="middle" rot="R180"/>
<pin name="P$8" x="43.18" y="43.18" length="middle" rot="R180"/>
<pin name="P$9" x="43.18" y="40.64" length="middle" rot="R180"/>
<pin name="P$10" x="43.18" y="38.1" length="middle" rot="R180"/>
<pin name="P$11" x="43.18" y="35.56" length="middle" rot="R180"/>
<pin name="P$12" x="43.18" y="27.94" length="middle" rot="R180"/>
<pin name="P$13" x="43.18" y="25.4" length="middle" rot="R180"/>
<pin name="P$14" x="43.18" y="22.86" length="middle" rot="R180"/>
<pin name="P$15" x="43.18" y="20.32" length="middle" rot="R180"/>
<pin name="P$16" x="43.18" y="17.78" length="middle" rot="R180"/>
<pin name="P$17" x="43.18" y="15.24" length="middle" rot="R180"/>
<pin name="P$18" x="43.18" y="12.7" length="middle" rot="R180"/>
<pin name="P$19" x="43.18" y="10.16" length="middle" rot="R180"/>
<pin name="P$20" x="43.18" y="7.62" length="middle" rot="R180"/>
<pin name="P$21" x="43.18" y="5.08" length="middle" rot="R180"/>
<pin name="P$22" x="43.18" y="2.54" length="middle" rot="R180"/>
<pin name="P$23" x="43.18" y="-5.08" length="middle" rot="R180"/>
<pin name="P$24" x="43.18" y="-7.62" length="middle" rot="R180"/>
<pin name="P$25" x="43.18" y="-10.16" length="middle" rot="R180"/>
<pin name="P$26" x="43.18" y="-12.7" length="middle" rot="R180"/>
<pin name="P$27" x="43.18" y="-15.24" length="middle" rot="R180"/>
<pin name="P$28" x="43.18" y="-17.78" length="middle" rot="R180"/>
<pin name="P$29" x="43.18" y="-20.32" length="middle" rot="R180"/>
<pin name="P$30" x="43.18" y="-22.86" length="middle" rot="R180"/>
<pin name="P$31" x="43.18" y="-25.4" length="middle" rot="R180"/>
<pin name="P$32" x="43.18" y="-27.94" length="middle" rot="R180"/>
<pin name="P$33" x="43.18" y="-30.48" length="middle" rot="R180"/>
<pin name="P$34" x="43.18" y="-38.1" length="middle" rot="R180"/>
<pin name="P$35" x="43.18" y="-40.64" length="middle" rot="R180"/>
<pin name="P$36" x="43.18" y="-43.18" length="middle" rot="R180"/>
<pin name="P$37" x="43.18" y="-45.72" length="middle" rot="R180"/>
<pin name="P$38" x="43.18" y="-48.26" length="middle" rot="R180"/>
<pin name="P$39" x="43.18" y="-50.8" length="middle" rot="R180"/>
<pin name="P$40" x="43.18" y="-53.34" length="middle" rot="R180"/>
<pin name="P$41" x="43.18" y="-55.88" length="middle" rot="R180"/>
<pin name="P$42" x="43.18" y="-58.42" length="middle" rot="R180"/>
<pin name="P$43" x="43.18" y="-60.96" length="middle" rot="R180"/>
<pin name="P$44" x="43.18" y="-63.5" length="middle" rot="R180"/>
<wire x1="38.1" y1="63.5" x2="38.1" y2="30.48" width="0.254" layer="94"/>
<wire x1="38.1" y1="30.48" x2="38.1" y2="-2.54" width="0.254" layer="94"/>
<wire x1="38.1" y1="-2.54" x2="38.1" y2="-35.56" width="0.254" layer="94"/>
<wire x1="38.1" y1="-35.56" x2="38.1" y2="-66.04" width="0.254" layer="94"/>
<wire x1="38.1" y1="-66.04" x2="-2.54" y2="-66.04" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-66.04" x2="-2.54" y2="-35.56" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-35.56" x2="-2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="-2.54" y2="30.48" width="0.254" layer="94"/>
<wire x1="-2.54" y1="30.48" x2="-2.54" y2="63.5" width="0.254" layer="94"/>
<wire x1="-2.54" y1="63.5" x2="38.1" y2="63.5" width="0.254" layer="94"/>
<wire x1="38.1" y1="-35.56" x2="-2.54" y2="-35.56" width="0.254" layer="94" style="longdash"/>
<wire x1="38.1" y1="-2.54" x2="-2.54" y2="-2.54" width="0.254" layer="94" style="longdash"/>
<wire x1="38.1" y1="30.48" x2="-2.54" y2="30.48" width="0.254" layer="94" style="longdash"/>
<text x="0" y="-38.1" size="1.778" layer="94" font="vector" align="center">N</text>
<text x="0" y="27.94" size="1.778" layer="94" font="vector" align="center">S</text>
<text x="0" y="60.96" size="1.778" layer="94" font="vector" align="center">W</text>
<text x="0" y="-5.08" size="1.778" layer="94" font="vector" align="center">E</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="PIC33" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="PIC33" x="0" y="0"/>
</gates>
<devices>
<device name="" package="QFN44">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$10" pad="P$10"/>
<connect gate="G$1" pin="P$11" pad="P$11"/>
<connect gate="G$1" pin="P$12" pad="P$12"/>
<connect gate="G$1" pin="P$13" pad="P$13"/>
<connect gate="G$1" pin="P$14" pad="P$14"/>
<connect gate="G$1" pin="P$15" pad="P$15"/>
<connect gate="G$1" pin="P$16" pad="P$16"/>
<connect gate="G$1" pin="P$17" pad="P$17"/>
<connect gate="G$1" pin="P$18" pad="P$18"/>
<connect gate="G$1" pin="P$19" pad="P$19"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
<connect gate="G$1" pin="P$20" pad="P$20"/>
<connect gate="G$1" pin="P$21" pad="P$21"/>
<connect gate="G$1" pin="P$22" pad="P$22"/>
<connect gate="G$1" pin="P$23" pad="P$23"/>
<connect gate="G$1" pin="P$24" pad="P$24"/>
<connect gate="G$1" pin="P$25" pad="P$25"/>
<connect gate="G$1" pin="P$26" pad="P$26"/>
<connect gate="G$1" pin="P$27" pad="P$27"/>
<connect gate="G$1" pin="P$28" pad="P$28"/>
<connect gate="G$1" pin="P$29" pad="P$29"/>
<connect gate="G$1" pin="P$3" pad="P$3"/>
<connect gate="G$1" pin="P$30" pad="P$30"/>
<connect gate="G$1" pin="P$31" pad="P$31"/>
<connect gate="G$1" pin="P$32" pad="P$32"/>
<connect gate="G$1" pin="P$33" pad="P$33"/>
<connect gate="G$1" pin="P$34" pad="P$34"/>
<connect gate="G$1" pin="P$35" pad="P$35"/>
<connect gate="G$1" pin="P$36" pad="P$36"/>
<connect gate="G$1" pin="P$37" pad="P$37"/>
<connect gate="G$1" pin="P$38" pad="P$38"/>
<connect gate="G$1" pin="P$39" pad="P$39"/>
<connect gate="G$1" pin="P$4" pad="P$4"/>
<connect gate="G$1" pin="P$40" pad="P$40"/>
<connect gate="G$1" pin="P$41" pad="P$41"/>
<connect gate="G$1" pin="P$42" pad="P$42"/>
<connect gate="G$1" pin="P$43" pad="P$43"/>
<connect gate="G$1" pin="P$44" pad="P$44"/>
<connect gate="G$1" pin="P$5" pad="P$5"/>
<connect gate="G$1" pin="P$6" pad="P$6"/>
<connect gate="G$1" pin="P$7" pad="P$7"/>
<connect gate="G$1" pin="P$8" pad="P$8"/>
<connect gate="G$1" pin="P$9" pad="P$9"/>
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
<class number="0" name="GND" width="0" drill="0">
</class>
<class number="1" name="motor" width="0" drill="0">
</class>
<class number="2" name="Vcc" width="0.25" drill="0.25">
<clearance class="2" value="0.25"/>
</class>
</classes>
<parts>
<part name="IC1" library="MCP2561-2 - High-Speed CAN Transceiver" deviceset="MCP2561-2" device="SOIC"/>
<part name="IC2" library="AMB2621 - BLE Module" deviceset="AMB2621" device=""/>
<part name="IC3" library="AS5601 - Programmable Contactless Encoder" deviceset="AS5601" device=""/>
<part name="IC4" library="4559A - PN Fet" deviceset="4559A" device=""/>
<part name="IC5" library="4559A - PN Fet" deviceset="4559A" device=""/>
<part name="IC6" library="PIC33" deviceset="PIC33" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="IC1" gate="G$1" x="68.58" y="33.02" smashed="yes">
<attribute name="NAME" x="55.88" y="48.26" size="2.54" layer="95"/>
<attribute name="VALUE" x="55.88" y="15.24" size="2.54" layer="96"/>
</instance>
<instance part="IC2" gate="G$1" x="88.9" y="-96.52" smashed="yes">
<attribute name="NAME" x="88.9" y="-81.28" size="1" layer="95" font="vector" align="center"/>
<attribute name="VALUE" x="88.9" y="-114.3" size="1" layer="96" font="vector" align="center"/>
</instance>
<instance part="IC3" gate="G$1" x="157.48" y="-25.4" smashed="yes">
<attribute name="NAME" x="157.48" y="-10.16" size="1.778" layer="95" align="center"/>
<attribute name="VALUE" x="157.48" y="-43.18" size="1.778" layer="96" align="center"/>
</instance>
<instance part="IC4" gate="G$1" x="60.96" y="-15.24" smashed="yes">
<attribute name="NAME" x="71.12" y="-7.62" size="1.27" layer="95" font="vector" align="center"/>
<attribute name="VALUE" x="71.12" y="-22.86" size="1.27" layer="96" font="vector" align="center"/>
</instance>
<instance part="IC5" gate="G$1" x="58.42" y="-48.26" smashed="yes">
<attribute name="NAME" x="68.58" y="-40.64" size="1.27" layer="95" font="vector" align="center"/>
<attribute name="VALUE" x="68.58" y="-55.88" size="1.27" layer="96" font="vector" align="center"/>
</instance>
<instance part="IC6" gate="G$1" x="-93.98" y="-25.4" smashed="yes"/>
</instances>
<busses>
</busses>
<nets>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
