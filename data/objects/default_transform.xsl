ƒ<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:tei="http://www.tei-c.org/ns/1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="html"/>
  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:teiHeader"/>
  <xsl:template match="tei:text">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:lb">
    <!--<xsl:if test="count(preceding-sibling::node()[2][name()='milestone']/rend) = 0">-->
    <br/>
    <!-- </xsl:if>-->
  </xsl:template>
  <xsl:template match="tei:hi">
    <xsl:choose>
      <xsl:when test="@rend='bold'">
        <b>
          <xsl:apply-templates/>
        </b>s
      </xsl:when>
      <xsl:when test="@rend='italic'">
        <i>
          <xsl:apply-templates/>
        </i>
      </xsl:when>
      <xsl:when test="@rend='ital'">
        <i>
          <xsl:apply-templates/>
        </i>
      </xsl:when>
      <xsl:when test="@rend='mono'">
        <code>
          <xsl:apply-templates/>
        </code>
      </xsl:when>
      <xsl:when test="@rend='roman'">
        <span class="normal">
          <xsl:apply-templates/>
        </span>
      </xsl:when>
      <xsl:when test="@rend='smallcaps'">
        <span class="sc">
          <xsl:apply-templates/>
        </span>
      </xsl:when>
      <xsl:when test="@rend='center;cap'">
        <span class="sc">
          <xsl:apply-templates/>
        </span>
      </xsl:when>
      <xsl:when test="@rend='sc'">
        <span class="sc">
          <xsl:apply-templates/>
        </span>
      </xsl:when>
      <xsl:when test="@rend='sub' or @rend='subscript'">
        <sub>
          <xsl:apply-templates/>
        </sub>
      </xsl:when>
      <xsl:when test="@rend='sup' or @rend='superscript'">
        <sup>
          <xsl:apply-templates/>
        </sup>
      </xsl:when>
      <xsl:when test="@rend='sup;ul'">
        <sup>
          <u>
            <xsl:apply-templates/>
          </u>
        </sup>
      </xsl:when>
      <xsl:when test="@rend='sup;double-underline'">
        <sup>
          <u>
            <xsl:apply-templates/>
          </u>
        </sup>
      </xsl:when>
      <xsl:when test="@rend='ul;sup' or @rend='sup;ul'">
        <sup>
          <u>
            <xsl:apply-templates/>
          </u>
        </sup>
      </xsl:when>
      <xsl:when test="@rend='ul;overbar'">
        <u>
          <xsl:apply-templates/>
        </u>
      </xsl:when>
      <xsl:when test="@rend='underline' or @rend='ul'">
        <u>
          <xsl:apply-templates/>
        </u>
      </xsl:when>
      <xsl:when test="@rend='circled' or @rend='vertical-line'">
        <xsl:apply-templates/>
      </xsl:when>
      <xsl:when test="@rend='double-underline'">
        <span style="border-bottom: 3px double;">
          <xsl:apply-templates/>
        </span>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="tei:fw">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:pb">
    <!-- added by lisa -->
    <xsl:if test="count(preceding-sibling::tei:p[1]) = 0 and count(preceding-sibling::tei:milestone[1][@unit['page']]) = 0 ">
      <br/>
    </xsl:if>
    <br/>
    <!-- end add by lisa -->
    <span class="norm header">
      <xsl:variable name="number" select="@n"/>
      <div>
        <xsl:attribute name="data-paged-viewer-page">
          <xsl:value-of select="translate($number, translate($number, '0123456789', ''), '')"/>
        </xsl:attribute>
        <xsl:value-of select="concat('Page ', $number)"/>
      </div>
    </span>
    <br/>
  </xsl:template>
  <xsl:template match="tei:ab">
    <xsl:apply-templates/>
    <br/>
  </xsl:template>
  <xsl:template match="tei:supplied">[<xsl:apply-templates/>]</xsl:template>
  <xsl:template match="tei:figDesc"/>
  <xsl:template match="comment()"/>
  <xsl:template match="tei:calendar">
    <xsl:if test=". != 'Muslim'">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>
  <!--<xsl:template match="tei:*[local-name()='em']">
      <xsl:variable name="filename" select="//div/@facs"/>
      <a name="{$filename}{@count}"/>
      <xsl:if test="@previous">
         <a href="#{$filename}{@previous}">
            <img border="0" alt="previous hit" src="b_inprev.gif"/>
         </a>
      </xsl:if>
      <span class="hitsection">
         <span class="subhit">
            <xsl:apply-templates/>
         </span>
      </span>
      <xsl:if test="@next">
         <a href="#{$filename}{@next}">
            <img border="0" alt="next hit" src="b_innext.gif"/>
         </a>
      </xsl:if>
   </xsl:template>-->
  <xsl:template match="tei:space">
    <xsl:choose>
      <xsl:when test="@extent">
        <xsl:call-template name="for.loop">
          <xsl:with-param name="i">1</xsl:with-param>
          <xsl:with-param name="count">
            <xsl:value-of select="@extent"/>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="tei:abbr">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:orig">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:corr">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="tei:unclear">[<xsl:apply-templates/>]</xsl:template>
  <xsl:template match="tei:expan"/>
  <xsl:template match="tei:reg"/>
  <xsl:template match="tei:sic"/>
  <xsl:template match="tei:del">
    <span style="text-decoration: line-through;">
      <xsl:apply-templates/>
    </span>
  </xsl:template>
  <!-- milestone tag -->
  <xsl:template match="tei:milestone">
    <xsl:choose>
      <xsl:when test="@unit='section' and @rend">
        <xsl:if test="@rend = 'half-line'">
          <hr align="left" style="height:1px;width:50%;"/>
        </xsl:if>
        <xsl:if test="@rend = 'line' or @rend = 'vertical-line'">
          <hr style="height:1px;"/>
        </xsl:if>
        <xsl:if test="@rend = '2 lines' or @rend = 'double-underline'">
          <hr style="height:1px;"/>
          <hr style="height:1px;"/>
        </xsl:if>
        <xsl:if test="@rend = '3 lines'">
          <hr style="height:1px;"/>
          <hr style="height:1px;"/>
          <hr style="height:1px;"/>
        </xsl:if>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
  <!-- gap tag -->
  <xsl:template match="tei:gap">
    <xsl:choose>
      <xsl:when test="@extent">[<xsl:call-template name="for.loop"><xsl:with-param name="i">1</xsl:with-param><xsl:with-param name="count"><xsl:value-of select="@extent"/></xsl:with-param></xsl:call-template>]</xsl:when>
    </xsl:choose>
  </xsl:template>
  <xsl:template name="for.loop">
    <xsl:param name="i"/>
    <xsl:param name="count"/>
    <!--begin_: Line_by_Line_Output -->
    <xsl:if test="$i &lt;= $count">&#160;</xsl:if>
    <!--begin_: RepeatTheLoopUntilFinished-->
    <xsl:if test="$i &lt;= $count">
      <xsl:call-template name="for.loop">
        <xsl:with-param name="i">
          <xsl:value-of select="$i + 1"/>
        </xsl:with-param>
        <xsl:with-param name="count">
          <xsl:value-of select="$count"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
  <xsl:template match="tei:note">
    <p>
      <xsl:apply-templates/>
    </p>
  </xsl:template>
  <xsl:template match="tei:p">
    <xsl:if test="count(preceding-sibling::tei:fw[1])=0">
      <p>
        <xsl:apply-templates/>
      </p>
    </xsl:if>
    <xsl:if test="count(preceding-sibling::tei:fw[1])=1">
      <xsl:apply-templates/>
    </xsl:if>
    <br/>
  </xsl:template>
</xsl:stylesheet>
