dnl $Id$
dnl config.m4 for extension swfed

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(swfed, for swfed support,
dnl Make sure that the comment is aligned:
dnl [  --with-swfed             Include swfed support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(swfed, whether to enable swfed support,
Make sure that the comment is aligned:
[  --enable-swfed           Enable swfed support])

if test "$PHP_SWFED" != "no"; then
  dnl Write more examples of tests here...

  sinclude(config-lib.m4)

  dnl # --with-swfed -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/swfed.h"  # you most likely want to change this
  dnl if test -r $PHP_SWFED/$SEARCH_FOR; then # path given as parameter
  dnl   SWFED_DIR=$PHP_SWFED
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for swfed files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SWFED_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SWFED_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the swfed distribution])
  dnl fi

  dnl # --with-swfed -> add include path
  dnl PHP_ADD_INCLUDE($SWFED_DIR/include)

  dnl # --with-swfed -> check for lib and symbol presence
  dnl LIBNAME=swfed # you may want to change this
  dnl LIBSYMBOL=swfed # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SWFED_DIR/lib, SWFED_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SWFEDLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong swfed lib version or lib not found])
  dnl ],[
  dnl   -L$SWFED_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(SWFED_SHARED_LIBADD)

  PHP_NEW_EXTENSION(swfed, swfed.c swf_object.c swf_header.c swf_rect.c swf_tag.c swf_tag_jpeg.c swf_tag_edit.c swf_tag_action.c swf_tag_lossless.c swf_tag_sound.c swf_tag_sprite.c swf_tag_shape.c swf_rgb.c swf_rgba.c swf_argb.c swf_xrgb.c swf_action.c swf_jpeg.c bitstream.c jpeg_segment.c swf_png.c swf_gif.c  swf_shape_with_style.c swf_styles.c swf_fill_style_array.c swf_fill_style.c swf_morph_shape_with_style.c swf_fill_style_solid.c swf_fill_style_gradient.c swf_fill_style_bitmap.c swf_line_style_array.c swf_line_style.c swf_styles_count.c swf_shape_record.c swf_shape_record_end.c swf_shape_record_setup.c swf_shape_record_edge.c swf_gradient.c swf_matrix.c swf_gradient_record.c swf_debug.c, $ext_shared)
fi

