<?xml version="1.0"?>
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xi="http://www.w3.org/2001/XInclude">
   <xsl:param name="OSD_SIZE" select="1.33"/>
   <xsl:param name="ICON_SMALL" select="32"/>
   <xsl:param name="ICON_MEDIUM" select="32"/>
   <xsl:param name="ICON_BIG" select="64"/>
   <xsl:param name="OSD_USE_OVERLAY">yes</xsl:param>
   <xsl:template match="node()|@*">
       <xsl:copy>
           <xsl:apply-templates select="node()|@*"/>
       </xsl:copy>
   </xsl:template>
   <xsl:template match="gui[2]">
       <xsl:copy>
           <xsl:apply-templates select="@*"/>
           <xsl:text disable-output-escaping="yes">&lt;![CDATA[</xsl:text>
               <!-- Remove the quit() img -->
               <xsl:value-of select="replace(., '&lt;img src=''gui_quit'' onclick=''quit\(\)''&gt;&lt;text&gt;Quit&lt;/text&gt;&lt;/img&gt;\n\t\t\t\t', '')" disable-output-escaping="yes"/>
           <xsl:text disable-output-escaping="yes">]]&gt;</xsl:text>
         </xsl:copy>
   </xsl:template>
   <xsl:include href="osd_minimum.xslt"/>
   <xsl:template match="/config/navit/graphics">
      <graphics type="cocoa_carplay" />
   </xsl:template>
   <xsl:template match="/config/navit[1]">
      <xsl:copy>
         <xsl:copy-of select="@*"/>
         <xsl:attribute name="timeout">86400</xsl:attribute>
         <xsl:attribute name="tunnel_extrapolation">10</xsl:attribute>
         <xsl:attribute name="tunnel_nightlayout">1</xsl:attribute>
         <xsl:apply-templates/>
      </xsl:copy>
   </xsl:template>
   <xsl:template match="/config/navit/vehicle[1]">
      <xsl:copy><xsl:copy-of select="@*[not(name()='gpsd_query')]"/>
      <xsl:attribute name="source">iphone:</xsl:attribute>
      <xsl:attribute name="follow">1</xsl:attribute>
      <xsl:attribute name="active">0</xsl:attribute>
      <xsl:apply-templates/></xsl:copy>
   </xsl:template>
   <xsl:template match="/config/navit/vehicle[2]">
      <xsl:copy><xsl:copy-of select="@*[not(name()='gpsd_query')]"/>
      <xsl:attribute name="source">demo://</xsl:attribute>
      <xsl:attribute name="follow">1</xsl:attribute>
      <xsl:attribute name="active">1</xsl:attribute>
      <xsl:attribute name="enabled">1</xsl:attribute>
      <xsl:attribute name="speed">30</xsl:attribute>
      <xsl:apply-templates/></xsl:copy>
   </xsl:template>
   <xsl:template match="/config/navit/speech">
      <xsl:copy>
         <xsl:copy-of select="@*[not(name()='data')]"/>
         <xsl:attribute name="type">iphone</xsl:attribute>
         <xsl:apply-templates/>
      </xsl:copy>
   </xsl:template>
   <xsl:template match="/config/navit/mapset/xi:include">
      <!-- Be aware that *.bin will only load the first map found, not all maps ending by .bin! -->
      <map type="binfile" enabled="yes" data="$NAVIT_USER_DATADIR/*.bin" />
   </xsl:template>
   <xsl:template match="@*|node()">
      <xsl:copy><xsl:apply-templates select="@*|node()"/></xsl:copy>
   </xsl:template>
</xsl:transform>
