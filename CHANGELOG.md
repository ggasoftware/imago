Imago OCR 2.0
----------------------------

*22 February 2013*

* Recognition quality:
    * Added adaptive preliminary filters designed for recognition of low-quality scans or even photos;
    * Recognition rate is significantly improved due to usage of highly-adaptive methods instead of strict criterias;
    * Restriction constants and recognition parameters are moved into configurable clusters, so the better recognition rate can be achieved on various non-typical image sets using machine learning approach;
    * Machine learning tool is embedded in [console application](/opensource/imago/imago_console), added ability to quickly check recognition rate on specified collection;
* Chemical features:
    * Chemical feature set is extended: bridged bonds, query features, R-groups are handled properly;
    * Superatom labels validation logic introduced which greatly increases recognition probability of complex superatom labels;
    * Abbreviation expansion support added;
* Improved interoperability:
    * Added support of various popular image formats: `JPG`, `BMP`, `DIB`, `TIFF`, `PBM`, `RAS` and others;
    * Implemented the new logging system which produces nice human-readable reports;
    * Replaced characters recognition system, the new one supports user-defined fonts;
    * Updated API and [Java](/opensource/imago/java) / [C](/opensource/imago/c) wrappers;
    * Updated build system, reduced dependencies list;
    * Fixed a lot of bugs, overall stability is significantly improved, decreased memory consumption. 

Imago OCR 1.0
-----------------------

* 28 March 2011*

* Recognizable Molecule Features
    * Single, double, triple bonds;
    * Atom labels, subscripts, isotopes, charges;
    * Superatoms;
    * Rings;
    * Stereochemistry (up- and down-bonds);
* Supported PNG image format;
* Implemented characters recognition system based on Fourier Descriptors;
* Java and C wrappers introduced;
* Ego, a Java GUI application introduced
* Improved techniques are used for molecular graph extraction.
