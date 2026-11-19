		SWF Editor for PHP
					2008/08/08- yoya@awm.jp

* introduce

  This is a PHP extension for edit SWF file. 

* require

 - php on unix
 - zlib
 - libpng and giflib

* example

 - replace image id = 1 jpeg data
 $swf = new SWFEditor();
 $swf->input($swfdata);
 $swf->replaceBitmapData(1, $jpegdata);
 echo $swf->output();
 
 - other sample code
   https://osdn.jp/projects/swfed/scm/git/swfed/tree/master/sample/

* messages and errors

 - swfInfo() (and the other print output) goes through the PHP output,
   so ob_start() can catch it.
 - SWFEditor::getLastError() returns the error messages of the last
   SWFEditor method call (one per line), or null.
 - php.ini swfed.error_output decides where the error messages go:
     stderr  : the process's stderr (default)
     warning : E_WARNING
     off     : nowhere (only getLastError())

 if ($swf->input($swfdata) === false) {
     error_log(SWFEditor::getLastError());
 }


* demo

 - http://app.awm.jp/swfed/
 - http://diary.awm.jp/~yoya/?20080331#200803311 replace jpeg
 - http://diary.awm.jp/~yoya/?20080514#200805141 replace alpha
 - http://diary.awm.jp/~yoya/?20101027#201010270 adjust bitmap
 - http://diary.awm.jp/~yoya/?20101130#201011300 action bytecode
 - http://diary.awm.jp/~yoya/?20110203#201102030 replace shape
 - http://diary.awm.jp/~yoya/?20110319#201103190 replace movieclip
 - http://diary.awm.jp/~yoya/?20110623#201106231 replace bitmap

* download

 - https://osdn.jp/projects/swfed/releases/ release 
 - git://git.osdn.jp/gitroot/swfed/swfed.git

* install

 - see install.txt

* more information

 - https://osdn.jp/projects/swfed/wiki/
