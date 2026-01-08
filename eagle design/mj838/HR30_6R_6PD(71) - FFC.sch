<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="yes"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="16" fill="1" visible="no" active="no"/>
<layer number="3" name="Route3" color="17" fill="1" visible="no" active="no"/>
<layer number="4" name="Route4" color="18" fill="1" visible="no" active="no"/>
<layer number="5" name="Route5" color="19" fill="1" visible="no" active="no"/>
<layer number="6" name="Route6" color="25" fill="1" visible="no" active="no"/>
<layer number="7" name="Route7" color="26" fill="1" visible="no" active="no"/>
<layer number="8" name="Route8" color="27" fill="1" visible="no" active="no"/>
<layer number="9" name="Route9" color="28" fill="1" visible="no" active="no"/>
<layer number="10" name="Route10" color="29" fill="1" visible="no" active="no"/>
<layer number="11" name="Route11" color="30" fill="1" visible="no" active="no"/>
<layer number="12" name="Route12" color="20" fill="1" visible="no" active="no"/>
<layer number="13" name="Route13" color="21" fill="1" visible="no" active="no"/>
<layer number="14" name="Route14" color="22" fill="1" visible="no" active="no"/>
<layer number="15" name="Route15" color="23" fill="1" visible="no" active="no"/>
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
<library name="HR30_6R_6PD(71) - Receptacle TH FFC">
<packages>
<package name="TH">
<pad name="6" x="-0.9" y="1.55" drill="0.9"/>
<pad name="5" x="-1.8" y="0" drill="0.9"/>
<pad name="4" x="-0.9" y="-1.55" drill="0.9"/>
<pad name="3" x="0.9" y="-1.55" drill="0.9"/>
<pad name="2" x="1.8" y="0" drill="0.9"/>
<pad name="1" x="0.9" y="1.55" drill="0.9"/>
<text x="1.6" y="2.35" size="0.5" layer="21" font="vector" align="center">1</text>
<text x="2.85" y="0" size="0.5" layer="21" font="vector" align="center">2</text>
<text x="1.6" y="-2.35" size="0.5" layer="21" font="vector" align="center">3</text>
<text x="-1.6" y="-2.35" size="0.5" layer="21" font="vector" align="center">4</text>
<text x="-2.85" y="0" size="0.5" layer="21" font="vector" align="center">5</text>
<text x="-1.6" y="2.35" size="0.5" layer="21" font="vector" align="center">6</text>
<circle x="0" y="0" radius="3.335415625" width="0.2" layer="21"/>
<text x="0" y="3.75" size="0.5" layer="25" font="vector" align="center">&gt;NAME</text>
<text x="0" y="-3.75" size="0.5" layer="27" font="vector" align="center">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="HR30_6R_6PD(71)">
<pin name="6" x="7.62" y="7.62" length="middle" rot="R180"/>
<pin name="5" x="7.62" y="5.08" length="middle" rot="R180"/>
<pin name="4" x="7.62" y="2.54" length="middle" rot="R180"/>
<pin name="3" x="7.62" y="-2.54" length="middle" rot="R180"/>
<pin name="2" x="7.62" y="-5.08" length="middle" rot="R180"/>
<pin name="1" x="7.62" y="-7.62" length="middle" rot="R180"/>
<text x="-6.35" y="3.81" size="1.778" layer="94" align="center">1</text>
<text x="-3.81" y="0" size="1.778" layer="94" align="center">2</text>
<text x="-6.35" y="-3.81" size="1.778" layer="94" align="center">3</text>
<text x="-11.43" y="-3.81" size="1.778" layer="94" align="center">4</text>
<text x="-13.97" y="0" size="1.778" layer="94" align="center">5</text>
<text x="-11.43" y="3.81" size="1.778" layer="94" align="center">6</text>
<circle x="-8.89" y="0" radius="6.35" width="0.254" layer="94"/>
<text x="-8.89" y="0" size="0.8128" layer="94" align="center">connector
inside view</text>
<wire x1="2.54" y1="10.16" x2="-17.78" y2="10.16" width="0.254" layer="94"/>
<wire x1="-17.78" y1="10.16" x2="-17.78" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-17.78" y1="-10.16" x2="2.54" y2="-10.16" width="0.254" layer="94"/>
<wire x1="2.54" y1="-10.16" x2="2.54" y2="10.16" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="HR30_6R_6PD(71)_FFC" prefix="CONN" uservalue="yes">
<gates>
<gate name="G$1" symbol="HR30_6R_6PD(71)" x="0" y="0"/>
</gates>
<devices>
<device name="" package="TH">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="FC1-10-02-T  - FFC Cable Contacts">
<packages>
<package name="SMD">
<smd name="T1" x="0" y="-4.5" dx="4" dy="0.7" layer="1"/>
<smd name="T2" x="0" y="-3.5" dx="4" dy="0.7" layer="1"/>
<smd name="T3" x="0" y="-2.5" dx="4" dy="0.7" layer="1"/>
<smd name="T4" x="0" y="-1.5" dx="4" dy="0.7" layer="1"/>
<smd name="T5" x="0" y="-0.5" dx="4" dy="0.7" layer="1"/>
<smd name="T6" x="0" y="0.5" dx="4" dy="0.7" layer="1"/>
<smd name="B6" x="0" y="0.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B5" x="0" y="-0.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B4" x="0" y="-1.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B3" x="0" y="-2.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B2" x="0" y="-3.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B1" x="0" y="-4.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="T7" x="0" y="1.5" dx="4" dy="0.7" layer="1"/>
<smd name="T8" x="0" y="2.5" dx="4" dy="0.7" layer="1"/>
<smd name="T9" x="0" y="3.5" dx="4" dy="0.7" layer="1"/>
<smd name="T10" x="0" y="4.5" dx="4" dy="0.7" layer="1"/>
<smd name="B7" x="0" y="1.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B8" x="0" y="2.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B9" x="0" y="3.5" dx="4" dy="0.7" layer="16" rot="R180"/>
<smd name="B10" x="0" y="4.5" dx="4" dy="0.7" layer="16" rot="R180"/>
</package>
</packages>
<symbols>
<symbol name="FC1-10-02-T-FFC_CABLE_CONTACTS">
<pin name="P$1" x="-7.62" y="-12.7" length="middle"/>
<pin name="P$2" x="-7.62" y="-10.16" length="middle"/>
<pin name="P$3" x="-7.62" y="-7.62" length="middle"/>
<pin name="P$4" x="-7.62" y="-5.08" length="middle"/>
<pin name="P$5" x="-7.62" y="-2.54" length="middle"/>
<pin name="P$6" x="-7.62" y="2.54" length="middle"/>
<wire x1="5.08" y1="15.24" x2="-2.54" y2="15.24" width="0.254" layer="94"/>
<wire x1="-2.54" y1="15.24" x2="-2.54" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-15.24" x2="5.08" y2="-15.24" width="0.254" layer="94"/>
<wire x1="5.08" y1="-15.24" x2="5.08" y2="15.24" width="0.254" layer="94"/>
<text x="0" y="17.78" size="1.27" layer="95" font="vector" align="center">&gt;NAME</text>
<text x="0" y="-17.78" size="1.27" layer="95" font="vector" align="center">&gt;VALUE</text>
<pin name="P$7" x="-7.62" y="5.08" length="middle"/>
<pin name="P$8" x="-7.62" y="7.62" length="middle"/>
<pin name="P$9" x="-7.62" y="10.16" length="middle"/>
<pin name="P$10" x="-7.62" y="12.7" length="middle"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="FC1-10-02-T-FFC_CABLE_CONTACTS" prefix="CONN" uservalue="yes">
<gates>
<gate name="G$1" symbol="FC1-10-02-T-FFC_CABLE_CONTACTS" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMD">
<connects>
<connect gate="G$1" pin="P$1" pad="B1 T1"/>
<connect gate="G$1" pin="P$10" pad="B10 T10"/>
<connect gate="G$1" pin="P$2" pad="B2 T2"/>
<connect gate="G$1" pin="P$3" pad="B3 T3"/>
<connect gate="G$1" pin="P$4" pad="B4 T4"/>
<connect gate="G$1" pin="P$5" pad="B5 T5"/>
<connect gate="G$1" pin="P$6" pad="B6 T6"/>
<connect gate="G$1" pin="P$7" pad="B7 T7"/>
<connect gate="G$1" pin="P$8" pad="B8 T8"/>
<connect gate="G$1" pin="P$9" pad="B9 T9"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="frames" urn="urn:adsk.eagle:library:229">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="A5L-LOC" library_version="1">
<wire x1="85.09" y1="3.81" x2="85.09" y2="24.13" width="0.1016" layer="94"/>
<wire x1="85.09" y1="24.13" x2="139.065" y2="24.13" width="0.1016" layer="94"/>
<wire x1="139.065" y1="24.13" x2="180.34" y2="24.13" width="0.1016" layer="94"/>
<wire x1="170.18" y1="3.81" x2="170.18" y2="8.89" width="0.1016" layer="94"/>
<wire x1="170.18" y1="8.89" x2="180.34" y2="8.89" width="0.1016" layer="94"/>
<wire x1="170.18" y1="8.89" x2="139.065" y2="8.89" width="0.1016" layer="94"/>
<wire x1="139.065" y1="8.89" x2="139.065" y2="3.81" width="0.1016" layer="94"/>
<wire x1="139.065" y1="8.89" x2="139.065" y2="13.97" width="0.1016" layer="94"/>
<wire x1="139.065" y1="13.97" x2="180.34" y2="13.97" width="0.1016" layer="94"/>
<wire x1="139.065" y1="13.97" x2="139.065" y2="19.05" width="0.1016" layer="94"/>
<wire x1="139.065" y1="19.05" x2="180.34" y2="19.05" width="0.1016" layer="94"/>
<wire x1="139.065" y1="19.05" x2="139.065" y2="24.13" width="0.1016" layer="94"/>
<text x="140.97" y="15.24" size="2.54" layer="94">&gt;DRAWING_NAME</text>
<text x="140.97" y="10.16" size="2.286" layer="94">&gt;LAST_DATE_TIME</text>
<text x="154.305" y="5.08" size="2.54" layer="94">&gt;SHEET</text>
<text x="140.716" y="4.953" size="2.54" layer="94">Sheet:</text>
<frame x1="0" y1="0" x2="184.15" y2="133.35" columns="4" rows="4" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="A5L-LOC" prefix="FRAME" uservalue="yes" library_version="1">
<description>A5L LOC</description>
<gates>
<gate name="G$1" symbol="A5L-LOC" x="0" y="0"/>
</gates>
<devices>
<device name="">
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
<class number="0" name="GND" width="0.25" drill="0.35">
<clearance class="0" value="0.19"/>
</class>
<class number="1" name="BAT+" width="2" drill="0.35">
<clearance class="0" value="0.5"/>
<clearance class="1" value="0.19"/>
</class>
<class number="2" name="signal" width="0.3" drill="0.35">
<clearance class="0" value="0.19"/>
<clearance class="1" value="0.5"/>
<clearance class="2" value="0.19"/>
</class>
</classes>
<parts>
<part name="CONN1" library="HR30_6R_6PD(71) - Receptacle TH FFC" deviceset="HR30_6R_6PD(71)_FFC" device="" value="HR30-6R-6PD(71)"/>
<part name="CONN2" library="FC1-10-02-T  - FFC Cable Contacts" deviceset="FC1-10-02-T-FFC_CABLE_CONTACTS" device="" value="FC1-10-02-T-WT"/>
<part name="FRAME1" library="frames" library_urn="urn:adsk.eagle:library:229" deviceset="A5L-LOC" device=""/>
</parts>
<sheets>
<sheet>
<plain>
<text x="55.88" y="88.9" size="1.778" layer="95">&gt;NAME</text>
<text x="55.88" y="63.5" size="1.778" layer="96">&gt;VALUE</text>
<text x="124.46" y="55.88" size="1.27" layer="96" align="center">or
046227010100800+</text>
</plain>
<instances>
<instance part="CONN1" gate="G$1" x="71.12" y="76.2" smashed="yes"/>
<instance part="CONN2" gate="G$1" x="124.46" y="76.2" smashed="yes">
<attribute name="NAME" x="124.46" y="93.98" size="1.27" layer="95" font="vector" align="center"/>
<attribute name="VALUE" x="124.46" y="58.42" size="1.27" layer="95" font="vector" align="center"/>
</instance>
<instance part="FRAME1" gate="G$1" x="0" y="0" smashed="yes">
<attribute name="DRAWING_NAME" x="140.97" y="15.24" size="2.54" layer="94"/>
<attribute name="LAST_DATE_TIME" x="140.97" y="10.16" size="2.286" layer="94"/>
<attribute name="SHEET" x="154.305" y="5.08" size="2.54" layer="94"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<pinref part="CONN1" gate="G$1" pin="1"/>
<wire x1="116.84" y1="68.58" x2="78.74" y2="68.58" width="0.1524" layer="91"/>
<label x="86.36" y="68.58" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$3"/>
<pinref part="CONN2" gate="G$1" pin="P$1"/>
<pinref part="CONN2" gate="G$1" pin="P$2"/>
<wire x1="116.84" y1="63.5" x2="116.84" y2="66.04" width="0.1524" layer="91"/>
<wire x1="116.84" y1="66.04" x2="116.84" y2="68.58" width="0.1524" layer="91"/>
<junction x="116.84" y="66.04"/>
<junction x="116.84" y="68.58"/>
</segment>
</net>
<net name="CAN_L2" class="2">
<segment>
<pinref part="CONN1" gate="G$1" pin="2"/>
<wire x1="116.84" y1="71.12" x2="78.74" y2="71.12" width="0.1524" layer="91"/>
<label x="86.36" y="71.12" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$4"/>
</segment>
</net>
<net name="CAN_L1" class="2">
<segment>
<pinref part="CONN1" gate="G$1" pin="3"/>
<wire x1="116.84" y1="73.66" x2="78.74" y2="73.66" width="0.1524" layer="91"/>
<label x="86.36" y="73.66" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$5"/>
</segment>
</net>
<net name="CAN_H2" class="2">
<segment>
<pinref part="CONN1" gate="G$1" pin="4"/>
<wire x1="116.84" y1="78.74" x2="78.74" y2="78.74" width="0.1524" layer="91"/>
<label x="86.36" y="78.74" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$6"/>
</segment>
</net>
<net name="BAT+" class="1">
<segment>
<pinref part="CONN1" gate="G$1" pin="6"/>
<wire x1="116.84" y1="83.82" x2="78.74" y2="83.82" width="0.1524" layer="91"/>
<label x="86.36" y="83.82" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$8"/>
<pinref part="CONN2" gate="G$1" pin="P$9"/>
<wire x1="116.84" y1="86.36" x2="116.84" y2="83.82" width="0.1524" layer="91"/>
<junction x="116.84" y="83.82"/>
<pinref part="CONN2" gate="G$1" pin="P$10"/>
<wire x1="116.84" y1="88.9" x2="116.84" y2="86.36" width="0.1524" layer="91"/>
<junction x="116.84" y="86.36"/>
</segment>
</net>
<net name="CAN_H1" class="2">
<segment>
<pinref part="CONN1" gate="G$1" pin="5"/>
<wire x1="116.84" y1="81.28" x2="78.74" y2="81.28" width="0.1524" layer="91"/>
<label x="86.36" y="81.28" size="1.778" layer="95"/>
<pinref part="CONN2" gate="G$1" pin="P$7"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="6.3" minversion="6.2.2" severity="warning">
Since Version 6.2.2 text objects can contain more than one line,
which will not be processed correctly with this version.
</note>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
</compatibility>
</eagle>
