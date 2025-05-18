# BT Headup Plugin

The BT Headup Plugin \<https://www.eisenzelt.de/\> enables Navit to show navigation data on the BT Headup device. It is a projection device for trucks that can be used to project navigation data on a surface like a bonnet or even on the street in front of the vehicle.
It supports Linux and iOS devices currently. The hardware needed can be build using the documentation at \<https://www.eisenzelt.de/\>.

## Quick Start

- Build Navit from source and supply the cmake argument ```-Dplugin/btheadup=true``` on the commandline.

- Start navit.

- Power on the BT Headup device.

- On Linux you have to find the serial number of the BT Headup Device. This is advertised by the Device Information Service. 

  ![image-20250216141830308](/Volumes/UserData/Library/Application Support/typora-user-images/image-20250216141830308.png)
``` xml
<headup type="btheadup" deviceserial="10490315433771926085"/>
```

- On iOS you need the UUID used by the phone for the device. Start with a random UUID in navit.xml:
``` xml
<headup type="btheadup" deviceserial="E80E7548-BB10-1F64-565E-DEADBEAFA55A"/>
```
Then check the log file of navit:

![image-20250216143736010](/Volumes/UserData/Library/Application Support/typora-user-images/image-20250216143736010.png)

Enter the ID found in the logfile and update navit.xml on the iPhone.
``` xml
<headup type="btheadup" deviceserial="E80E7548-BB10-1F64-565E-2FBA117704036"/>
```

Restart Navit. It should connect to the device and show some data.

![image-20250216145354514](/Volumes/UserData/Library/Application Support/typora-user-images/image-20250216145354514.png)

## Add OSM map to your mapset

Move the downloaded map to the directory of your choice, and add it to the active mapset (see \[\[Configuration\]\]) in navit.xml with a line similar to the following:

``` xml
<mapset>
 <map type="binfile" enabled="yes" data="/path/to/your/map/my_downloaded_map.bin" />
</mapset>
```

## Topographic Maps

Navit will display elevation/height lines but the required data is not included in most OSM derived maps.

Navit compatible maps with height lines can be created by feeding the output of Phyghtmap \<http://wiki.openstreetmap.org/wiki/Phyghtmap\> to Navit's maptool. Alternatively the SRTM data can be downloaded in osm.xml format <http://geoweb.hft-stuttgart.de/SRTM/srtm_as_osm/>, avoiding the Phygtmap step. The information can be either merged with OSM derived maps or used in a separate layer.

Many Garmin type maps such as <http://www.wanderreitkarte.de/garmin_de.php> also have the height lines information but routing will not work with them.

## Processing OSM Maps yourself

You can create your own Navit binfiles from OSM data very easily using \[\[maptool\]\], the conversion program which installs alongside Navit. ''maptool'' can process both OpenStreetMap XML Data files (*.osm files) and OpenStreetMap Protobuf Data files (*.pbf files) Follow these steps to process your own maps.

### Download your own OSM data

OSM data can be downloaded from a variety of sources. OpenStreetMap XML Data files are regular textfiles, easily editable in any text editor. OpenStreetMap Protobuf Data files are binary files, which take up less space (so are quicker to download and process) but are not editable.

- OpenStreetMap XML Data
  - [Geofabrik](http://download.geofabrik.de/osm/) provides pre-processed OpenStreetMap XML Data files of almost all countries, and all continents. This method is probably the easiest way of downloading OpenStreetMap XML Data for an entire country or continent. Note that the OSM files are bzipped
  - [planet.openstreetmap.org](http://planet.openstreetmap.org) hosts the complete data set (the whole world). You can use [Osmosis](http://wiki.openstreetmap.org/index.php/Osmosis) to cut it into smaller chunks.
  - [OpenStreetMap ReadOnly (XAPI)](http://wiki.openstreetmap.org/wiki/Xapi) The API allows to get the data of a specific bounding box, so that download managers can be used. For example: wget -O map.osm "<http://xapi.openstreetmap.org/api/0.6/map?bbox=11.4,48.7,11.6,48.9>"
  - [OpenStreetMap (visual)](http://www.openstreetmap.org/export) allows you to select a small rectangular area and download the selection as OpenStreetMap XML Data.
- '''OpenStreetMap Protobuf Data'''
  - \[<http://download.geofabrik.de/osm/> Geofabrik\] provides pre-processed OpenStreetMap Protobuf Data files of almost all countries, and all continents.

### Convert OSM data to Navit binfile

The following examples assume that you have installed Navit system-wide. If this is not the case, you will need to provide an absolute path to the ''maptool'' executable, which is in the navit/maptool folder.

Please also note, that maptool uses country multipolygon relations. So it's a good idea to include the whole country boundary to your dataset. You can use the josm editor to download the country boundary relation and save it as osm file. Then this file can be concatenated with your sub-country level excerpt.

#### From .osm

``` bash
cat my_OSM_map.osm | maptool my_Navit_map.bin
```

Or

``` bash
maptool -i my_OSM_map.osm my_Navit_map.bin
```

Or for multiple OSM data files use the \<tt\>--dedupe-ways\</tt\> option to avoid duplication of way data if a way occurs multiple times in the OSM maps.

``` bash
cat my_OSM_map1.osm my_OSM_map2.osm my_OSM_map3.osm | maptool --dedupe-ways my_Navit_map.bin
```

#### From .bz2

``` bash
bzcat my_OSM_map.osm.bz2 | maptool my_Navit_map.bin
```

#### From .pbf

``` bash
maptool --protobuf -i my_OSM_map.osm.pbf my_Navit_map.bin
```

## Processing the whole Planet

The OpenStreetMap wiki \[<http://wiki.openstreetmap.org/index.php/Planet.osm> Planet.osm\] page lists mirrors where Planet.osm can be downloaded. There are also downloads of smaller areas such as the UK and parts of Europe. These smaller excerpts are a lot quicker to download and process.

In case you want the whole planet.osm (24GB in December 2012), it is even possible to process planet.osm. It will take about 7 hours , requires \> 1GB of main memory and about 30 GB disk space for result and temp files - planet.bin is currently (as of December 2012) 9.6GB:

``` bash
bzcat planet.osm.bz2 | maptool -6 my_Navit_map.bin
```

Please note -6 option (long name --64bit) used above. It should be used always if output bin file grows above 4GB, or generated file will not work at all. Using that option on smaller files slightly increases their size and makes them unreadable by some unzip versions.

## Tips

- To enable a map you have downloaded refer \[\[OpenStreetMap#Adding_an_OSM_map_to_your_mapset\| adding OSM map to navit.xml\]\]
- If you don't see any map data in Navit (assuming your map is properly specified in navit.xml) using the Internal GUI click anywhere on the screen to bring up the menu. Click on "Actions" and then "Town". Type in the name of a town that should be within your map data. Select your town from the list that appears. This will bring up a sub-menu where you can click "View On Map". Note that if you have a GPS receiver you can also just wait till you get a satellite lock.
- To avoid changing navit.xml if you update your maps and the maps have different file names use the wildcard (\*.bin) in your navit.xml file. For example:

``` xml
<map type="binfile" enabled="yes" data="/media/mmc2/maps/*.bin"/>
```
