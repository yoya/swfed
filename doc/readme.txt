		SWF Editor for PHP
					2008/08/08- yoya@awm.jp

* introduce

  This is a PHP extension for edit SWF file. 

* require

 - PHP 7.2 or later, on a Unix-like OS
 - zlib
 - libpng, giflib and liblzma (optional: without them PNG / GIF / LZMA
   compressed SWF (ZWS) are not supported)

* example

 - replace image id = 1 jpeg data
 $swf = new SWFEditor();
 $swf->input($swfdata);
 $swf->replaceBitmapData(1, $jpegdata);
 echo $swf->output();
 
 - other sample code
   https://github.com/yoya/swfed/tree/master/sample
 - API reference (every method, its arguments and return values)
   ../README.md


* demo (old pages, may no longer be available)

 - http://app.awm.jp/swfed/
 - http://diary.awm.jp/~yoya/?20080331#200803311 replace jpeg
 - http://diary.awm.jp/~yoya/?20080514#200805141 replace alpha
 - http://diary.awm.jp/~yoya/?20101027#201010270 adjust bitmap
 - http://diary.awm.jp/~yoya/?20101130#201011300 action bytecode
 - http://diary.awm.jp/~yoya/?20110203#201102030 replace shape
 - http://diary.awm.jp/~yoya/?20110319#201103190 replace movieclip
 - http://diary.awm.jp/~yoya/?20110623#201106231 replace bitmap

* download

 - https://github.com/yoya/swfed

* install

 - see install.txt

* more information

 - https://github.com/yoya/swfed (README.md)
 - https://github.com/yoya/swfed/issues
