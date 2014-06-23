<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:ead="urn:isbn:1-931666-22-9"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:srw_dc="info:srw/schema/1/dc-schema"
  xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
  version="1.0">
  <!-- 
	Version 1.0		2014-06-14 nigel@discoverygarden.ca
	
	This stylesheet transforms EAD version 2.0 records to simple Dublin Core (DC) records, 
	based on the Library of Congress' EAD to simple DC mapping <http://www.loc.gov/ead/ag/agappb.html#sec3>
	
	This transform is for describing the finding aid only, it doesn't attempt to generate DC for the archival material.
	-->
  <xsl:output method="xml" indent="yes"/>
  <xsl:template match="/">
    <oai_dc:dc xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
      <dc:type>archival finding aid</dc:type>
      <xsl:apply-templates/>
    </oai_dc:dc>
  </xsl:template>
  <xsl:template match="ead:titleproper">
    <dc:title><xsl:value-of select='normalize-space()'/></dc:title>
  </xsl:template>
  <xsl:template match="ead:notestmt">
    <!-- Each note is a seperate description. -->
    <xsl:for-each select="//ead:note">
      <dc:description><xsl:value-of select='normalize-space()'/></dc:description>
    </xsl:for-each>
    <!-- Catch subject that are at any level within the ead:notestmt. -->
    <xsl:for-each select="//ead:subject">
      <dc:subject><xsl:value-of select='normalize-space()'/></dc:subject>
    </xsl:for-each>
  </xsl:template>
  <xsl:template match="ead:author">
    <dc:creator><xsl:value-of select='normalize-space()'/></dc:creator>
  </xsl:template>
  <xsl:template match="ead:publisher">
    <dc:publisher><xsl:value-of select='normalize-space()'/></dc:publisher>
  </xsl:template>
  <xsl:template match="ead:publicationstmt/ead:date">
    <dc:date><xsl:value-of select='normalize-space()'/></dc:date>
  </xsl:template>
  <xsl:template match="ead:eadid">
    <dc:identifier><xsl:value-of select='normalize-space()'/></dc:identifier>
  </xsl:template>
  <xsl:template match="ead:language[@langcode]">
    <dc:language><xsl:value-of select='normalize-space(@langcode)'/></dc:language>
  </xsl:template>
  <!-- Recurse Accept for the archival section -->
  <xsl:template match="ead:archdesc"/>
  <xsl:template match="*">
    <xsl:apply-templates/>
  </xsl:template>
  <!-- Don't render text unless part of matches above. -->
  <xsl:template match="text()"/>
</xsl:stylesheet>