		SWF Editor for PHP
					2008/08/08- yoya@awm.jp

* introduce

  This is a PHP extension for edit SWF file. 

* require

 - php on unix
 - zlib
 - libpng and giflib (optional)

* example

 - replace image id = 1 jpeg data
 $swf = new SWFEditor();
 $swf->input($swfdata);
 $swf->replaceBitmapData(1, $jpegdata);
 echo $swf->output();
 
 - other sample code
   http://git.sourceforge.jp/view?p=swfed/swfed.git;a=tree;f=sample

* demo

 - http://awm.jp/~yoya/php/swfed/
 - http://diary.awm.jp/~yoya/?20080331#200803311 replace jpeg
 - http://diary.awm.jp/~yoya/?20080514#200805141 replace alpha
 - http://diary.awm.jp/~yoya/?20101027#201010270 adjust bitmap
 - http://diary.awm.jp/~yoya/?20101130#201011300 action bytecode
 - http://diary.awm.jp/~yoya/?20110203#201102030 replace shape
 - http://diary.awm.jp/~yoya/?20110319#201103190 replace movieclip
 - http://diary.awm.jp/~yoya/?20110623#201106231 replace bitmap

* download

 - http://sourceforge.jp/projects/swfed/files/ release 
 - git://git.sourceforge.jp/gitroot/swfed/swfed.git current 

* install

 - see install.txt

* more information

 - http://sourceforge.jp/projects/swfed/wiki/
