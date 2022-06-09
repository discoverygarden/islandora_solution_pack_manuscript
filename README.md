# Manuscript Solution Pack [![Build Status](https://travis-ci.org/discoverygarden/islandora_solution_pack_manuscript.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_solution_pack_manuscript)

## Introduction

Allows users to create and view Manuscripts. Including the upload of TEI and XSLT and CSS documents. Users will be able to view transformed manuscript TEI (via the upload XSLT) side by side with the image(s) of the manuscript (via the Open Seadragon viewer). Users will also be able to browse Manuscripts via Box / Folder hierarchies as defined by their record in an associated finding aid.

The [Connecticut Digital Archive](http://ctdigitalarchive.org/) has put together an [informative user documentation guide](http://www.slideshare.net/CTDigitalArchive/how-to-use-the-manuscript-content-model) that outlines the functionality of this module.

### Key Features

* Ability to upload TEI
* Ability to upload XSLT (1.0) / CSS
* Ability to associate XSLT / CSS with a Finding Aid Object
* Rendering of TEI as HTML (XSLT)
* Open Sea Dragon Viewer display of Manuscript image Content
* Side by side view of Transformed TEI and Open Sea Dragon Viewer
* Navigation of manuscripts by box / folder meta-data (SOLR driven)

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Islandora Paged Content](https://github.com/Islandora/islandora_paged_content)
* [jstree](https://github.com/vakata/jstree)

This module has the following recommended (optional) modules/libraries:

* [Islandora Large Image Solution Pack](https://github.com/Islandora/islandora_solution_pack_large_image)
* [Islandora OCR](https://github.com/Islandora/islandora_ocr)
* [Islandora OpenSeadragon](https://github.com/Islandora/islandora_openseadragon)
* [Islandora SOLR Search](https://github.com/Islandora/islandora_solr_search)
* [Islandora Paged TEI Seadragon Viewer](https://github.com/discoverygarden/islandora_paged_tei_seadragon)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

Download/clone the [jsTree](https://github.com/vakata/jstree) library to `sites/all/libraries/jstree`. The module has been tested with the 3.0.0 release of jsTree.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## FAQ

### Q. What elements are necessary in finding aid EAD metadata?

A. Components (`c`, `c01`, `c02`, `c03`, _etc_) *MUST* have `id` attributes unique to the given XML document in order to reliably produce links and relationships. Components *MUST* have a `level` as one of:
* `series`
* `subseries`
* `file`

Additionally, as of writing, the only supported types of containers inside of components are boxes and folders. Folder entries *MAY* be associatited to boxes using the `parent` attribute, to target the `id` given to a box. Alternatively, associations will be made by iterating containers and producing a new association for each "box" encountered.

A minimal example of the structure we require:
```xml
<ead xmlns="urn:isbn:1-931666-22-9">
  <eadheader>
    <eadid>example-id</eadid>
    <filedesc>
      <titlestmt>
        <titleproper>Example Collection</titleproper>
      </titlestmt>
    </filedesc>
  </eadheader>
  <archdesc level="collection">
    <did>
      <unittitle>Example Collection</unittitle>
    </did>
    <dsc>
      <c01 id="alpha" level="series">
        <!--
          "bravo" makes use of the "parent" attribute to associate a folder
          with a box.
        -->
        <did>
          <unittitle>Alpha</unittitle>
        </did>
        <c02 id="bravo" level="file">
          <did>
            <unittitle>Bravo</unittitle>
            <container id="container-one" type="box">1</container>
            <container parent="container-one" type="folder">1</container>
          </did>
        </c02>
        <!--
          "charlie" relates containers by associating boxes and folders as they
          occur in document order.
        -->
        <c02 id="charlie" level="file">
          <did>
            <unittitle>Charlie</unittitle>
            <container type="boxes">2-3</container>
            <container type="box">4</container>
            <container type="folder">1</container>
            <container type="box">5</container>
            <container type="folders">1-7</container>
          </did>
        </c02>
      </c01>
    </dsc>
  </archdesc>
</ead>
```

In `bravo`, we have one logical container:
* folder 1 from box 1

In `charlie`, we have three logical containers:
* boxes 2 to 3
* folder 1 from box 4
* folders 1 to 7 from box 5

Do note that the code tries not to make any assumptions about the numbering of boxes or folders. Folders could either be numbered sequentially across boxes (in which case specifying a range of folders could make sense when specifying a range of boxes) or specific to a box. Additionally, pluralization of types is largely ignored.

### Q. How are links generated from the Finding Aid to digitized objects?

A. Links will be generated from the EAD rendering to digitized objects in one or more of two ways:

1) If the child objects have Solr metadata which points to the EAD object, box identifier, folder identifier, and component id, search queries will be formulated from the Finding Aid to the matching digital objects.  This is configured in the settings form, under the heading "Link Objects by Query".

2) If the DAOs in the EAD have links in the xlink namespace which point to paths identifying the digital objects, these URIs can be embedded (with an optional prefix) within the Finding Aid display.  Examples could be where the DAO's href might point to a link resolver, a DOI, relative or absolute URI, or a Fedora PID.  This is configured in the settings form, under the heading "Link Objects by DAO xlink".

## Maintainers/Sponsors
Current maintainers:

* [discoverygarden](https://github.com/discoverygarden)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
