		SWF Editor for PHP
					2008/08/08- yoya@awm.jp

* introduce

  this is php extension for edit SWF file. 

* require

 - php on unix
 - zlib
 - libpng and giflib (optional)

* example

 - replace image id = 1 jpeg data
 $swf = new SWFEditor(); 
 $swf->input($swfdata); 
 $swf->replaceJpegData(1, $jpegdata); 
 echo $swf->output(); 
 
 - other sample code
   http://svn.sourceforge.jp/svnroot/swfed/trunk/sample/

* demo

 - http://awm.jp/~yoya/php/swfed/
 - http://diary.awm.jp/~yoya/?20080331#200803311 replace jpeg
 - http://diary.awm.jp/~yoya/?20080514#200805141 replace alpha
 - http://diary.awm.jp/~yoya/?20101027#201010270 adjust bitmap
 - http://diary.awm.jp/~yoya/?20101130#201011300 action bytecode

* download

 - http://sourceforge.jp/projects/swfed/files/ release 
 - http://svn.sourceforge.jp/svnroot/swfed/trunk/ current 

* install

 - see install.txt

* more information

 - http://sourceforge.jp/projects/swfed/wiki/
