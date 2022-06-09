<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
  xmlns="http://www.w3.org/1999/xhtml"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:ead="urn:isbn:1-931666-22-9"
  xmlns:xlink="http://www.w3.org/1999/xlink"
  xmlns:php="http://php.net/xsl"
  xsl:extension-element-prefixes="php"
>
  <xsl:param name="container_string">Containers</xsl:param>

  <xsl:param name="call_query_link" />
  <xsl:param name="call_direct_link" />
  <xsl:param name="direct_link_prefix" />

  <xsl:template match="/">
    <div class="ead">
      <xsl:apply-templates select="//ead:archdesc"/>
    </div>
  </xsl:template>

  <xsl:template match="ead:scopecontent">
    <xsl:if test="ead:head">
      <h3>
        <xsl:apply-templates select="ead:head"/>
      </h3>
    </xsl:if>
    <xsl:apply-templates select="ead:p"/>
  </xsl:template>

  <xsl:template match="ead:processinfo"/>

  <!--
    Helper template to allow the use of IDs from EAD.

    IDs generated with generate-id() will be different between different
    renderings of the document.
  -->
  <xsl:template name="get_id">
    <xsl:param name="element" select="current()"/>
    <xsl:choose>
      <xsl:when test="$element[@id]">
        <xsl:value-of select="$element/@id"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="generate-id($element)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- General display -->
  <xsl:template match="ead:c | ead:c01 | ead:c02 | ead:c03">
    <fieldset>
      <xsl:attribute name="class">
        <xsl:text>ead-component collapsible collapsed </xsl:text>
        <xsl:value-of select="concat('ead-component-', local-name())"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="concat('ead-component-type-', @level)"/>
      </xsl:attribute>
      <legend>
        <span class="fieldset-legend">
          <xsl:apply-templates select="ead:did/ead:unittitle"/>
          <xsl:text> </xsl:text>
          <xsl:value-of select="ead:did/ead:unitdate"/>
        </span>
      </legend>
      <div class="fieldset-wrapper">
        <!-- This id should be on the fieldset semantically but is here to appease Drupal. -->
        <xsl:attribute name="id">
          <xsl:call-template name="get_id"/>
        </xsl:attribute>
        <xsl:apply-templates/>
      </div>
    </fieldset>
  </xsl:template>

  <xsl:template match="ead:did">
    <xsl:variable name="contents">
      <xsl:call-template name="archdesc_did"/>
      <xsl:call-template name="container"/>
    </xsl:variable>
    <xsl:if test="normalize-space($contents)">
      <dl>
        <xsl:copy-of select="$contents"/>
      </dl>
      <xsl:if test="count(ead:dao[@xlink:href])">
        <xsl:variable name="xlinks">
          <xsl:call-template name="ead_dao_xlink" />
        </xsl:variable>
        <xsl:if test="normalize-space($xlinks)">
          <ul class="ead_daos">
            <xsl:copy-of select="$xlinks" />
          </ul>
        </xsl:if>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <!-- Handle top level did. -->
  <xsl:template name="archdesc_did">
    <xsl:if test="not(ead:container[@parent])">
      <xsl:for-each select="*">
        <p>
          <xsl:apply-templates select="."/>
        </p>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <!-- build definition list containing container searches. -->
  <xsl:template name="container">
    <xsl:variable name="contents">
      <xsl:choose>
        <xsl:when test="ead:container[@parent]">
          <xsl:apply-templates select="ead:container[@parent]" mode="parent"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="flat_container"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:if test="normalize-space($contents)">
      <dt>
        <xsl:value-of select="$container_string"/>
      </dt>
      <xsl:copy-of select="$contents"/>
    </xsl:if>
  </xsl:template>

  <xsl:template name="flat_container">
    <xsl:variable name="query_url">
      <xsl:if test="$call_query_link = 'true'">
        <xsl:value-of select="php:function('islandora_manuscript_build_flat_query_url', ead:container)" />
      </xsl:if>
    </xsl:variable>
    <dd>
      <xsl:choose>
      <xsl:when test="normalize-space($query_url)">
        <a class="ead-external-link">
          <xsl:attribute name="href">
            <xsl:value-of select="$query_url"/>
          </xsl:attribute>
          <xsl:apply-templates select="ead:container[1]" mode="flat_text"/>
        </a>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="ead:container[1]" mode="flat_text"/>
      </xsl:otherwise>
      </xsl:choose>
    </dd>
  </xsl:template>
  <xsl:template match="ead:container" mode="flat_text">
    <xsl:value-of select="@type"/>
    <xsl:text> </xsl:text>
    <xsl:apply-templates/>
    <xsl:variable name="sibling_content">
      <xsl:apply-templates select="following-sibling::ead:container[1]" mode="flat_text"/>
    </xsl:variable>
    <xsl:if test="normalize-space($sibling_content)">
      <xsl:text>, </xsl:text>
      <xsl:copy-of select="$sibling_content"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="ead:container" mode="parent">
    <xsl:variable name="containers" select="//ead:container"/>
    <xsl:variable name="query_url">
      <xsl:if test="$call_query_link = 'true'">
        <xsl:value-of select="php:function('islandora_manuscript_build_parented_query_url', current(), $containers)" />
      </xsl:if>
    </xsl:variable>
    <dd>
      <xsl:choose>
      <xsl:when test="normalize-space($query_url)">
        <a class="ead-external-link">
          <xsl:attribute name="href">
            <xsl:copy-of select="$query_url"/>
          </xsl:attribute>
          <xsl:apply-templates select="." mode="parent_text"/>
        </a>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="." mode="parent_text"/>
      </xsl:otherwise>
      </xsl:choose>
    </dd>
  </xsl:template>

  <xsl:template match="ead:container" mode="parent_text">
    <xsl:variable name="parent" select="@parent"/>
    <xsl:variable name="parents">
      <xsl:apply-templates select="//ead:container[$parent = @id]" mode="parent_text"/>
    </xsl:variable>
    <xsl:if test="normalize-space($parents) != ''">
      <xsl:value-of select="$parents"/>
      <xsl:text>, </xsl:text>
    </xsl:if>
    <xsl:value-of select="@type"/>
    <xsl:text> </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template name="ead_dao_xlink">
    <xsl:for-each select="ead:dao[@xlink:href]">
    <xsl:variable name="direct_url">
      <xsl:if test="$call_direct_link = 'true'">
        <!--
        N.b.: unused but reserved for future use if logic becomes more complex than a simple prefix
        <xsl:value-of select="php:function('islandora_manuscript_build_direct_url', @xlink:href)" />
        -->
        <xsl:value-of select="$direct_link_prefix" /><xsl:value-of select="@xlink:href" />
      </xsl:if>
    </xsl:variable>
    <li>
      <xsl:choose>
      <xsl:when test="normalize-space($direct_url)">
        <a class="ead-external-link">
          <xsl:attribute name="href">
            <xsl:value-of select="$direct_url" />
          </xsl:attribute>
          <xsl:value-of select="ead:daodesc" />
        </a>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
        <xsl:value-of select="ead:daodesc" />
      </xsl:choose>
    </li>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="text()" mode="did_list"/>
  <!-- end of did/definition list stuff -->

  <xsl:template match="ead:p">
    <p>
      <xsl:apply-templates/>
    </p>
  </xsl:template>

  <xsl:template match="ead:extref">
    <a class="ead-external-link">
      <xsl:attribute name="href">
        <xsl:value-of select="@xlink:href"/>
      </xsl:attribute>
      <xsl:value-of select="text()"/>
    </a>
  </xsl:template>
  <!-- end of general display stuff -->
</xsl:stylesheet>
