if test -z "$PHP_ZLIB_DIR"; then
PHP_ARG_WITH(zlib-dir, for the location of ZLIB,
[  --with-zlib-dir[=DIR]   swfed: Set the path to ZLIB install prefix.], no, no)
fi

if test -z "$PHP_PNG_DIR"; then
PHP_ARG_WITH(png-dir, for the location of LIBPNG,
[  --with-png-dir[=DIR]   swfed: Set the path to LIBPNG install prefix.], no, no)
fi

if test -z "$PHP_GIF_DIR"; then
PHP_ARG_WITH(gif-dir, for the location of GIFLIB,
[  --with-gif-dir[=DIR]   swfed: Set the path to GIFLIB install prefix.], no, no)
fi


#
# ZLIB
#
  if test "$PHP_ZLIB_DIR" != "no" && test "$PHP_ZLIB_DIR" != "yes"; then
    if test -f "$PHP_ZLIB_DIR/include/zlib/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include/zlib"
    elif test -f "$PHP_ZLIB_DIR/include/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include"
    else
      AC_MSG_ERROR([Can't find ZLIB headers under "$PHP_ZLIB_DIR"])
    fi
  else
    for i in /usr/local /usr; do
      if test -f "$i/include/zlib/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include/zlib"
      elif test -f "$i/include/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include"
      fi
    done
  fi

  dnl # zlib
  AC_MSG_CHECKING([for the location of zlib])
  if test "$PHP_ZLIB_DIR" = "no"; then
    AC_MSG_ERROR([swfed support requires ZLIB. Use --with-zlib-dir=<DIR> to specify prefix where ZLIB include and library are located])
  else
    AC_MSG_RESULT([$PHP_ZLIB_DIR])
    if test "z$PHP_LIBDIR" != "z"; then
    dnl PHP5+
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_ZLIB_DIR/$PHP_LIBDIR, SWFED_SHARED_LIBADD)
    else
    dnl PHP4
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_ZLIB_DIR/lib, SWFED_SHARED_LIBADD)
    fi
    PHP_ADD_INCLUDE($PHP_ZLIB_INCDIR)
  fi

#
# LIBPNG
#
  if test "$PHP_PNG_DIR" != "no" && test "$PHP_PNG_DIR" != "yes"; then
    if test -f "$PHP_PNG_DIR/include/libpng/png.h"; then
      AC_DEFINE(HAVE_PNG,1,[ ])
      PHP_PNG_DIR="$PHP_PNG_DIR"
      PHP_PNG_INCDIR="$PHP_PNG_DIR/include/libpng"
    elif test -f "$PHP_PNG_DIR/include/png.h"; then
      AC_DEFINE(HAVE_PNG,1,[ ])
      PHP_PNG_DIR="$PHP_PNG_DIR"
      PHP_PNG_INCDIR="$PHP_PNG_DIR/include"
    else
      AC_MSG_ERROR([Can't find LIBPNG headers under "$PHP_PNG_DIR"])
    fi
  else
    for i in /usr/local /usr; do
      if test -f "$i/include/libpng/png.h"; then
	AC_DEFINE(HAVE_PNG,1,[ ])
        PHP_PNG_DIR="$i"
        PHP_PNG_INCDIR="$i/include/libpng"
      elif test -f "$i/include/png.h"; then
	AC_DEFINE(HAVE_PNG,1,[ ])
        PHP_PNG_DIR="$i"
        PHP_PNG_INCDIR="$i/include"
      fi
    done
  fi

  dnl # libpng
  AC_MSG_CHECKING([for the location of libpng])
  if test "$PHP_PNG_DIR" = "no"; then
    AC_MSG_RESULT([png.h not found.])
  else
    AC_MSG_RESULT([$PHP_PNG_DIR])
    if test "z$PHP_LIBDIR" != "z"; then
    dnl PHP5+
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_PNG_DIR/$PHP_LIBDIR, SWFED_SHARED_LIBADD)
    else
    dnl PHP4
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_PNG_DIR/lib, SWFED_SHARED_LIBADD)
    fi
    PHP_ADD_INCLUDE($PHP_PNG_INCDIR)
  fi

#
# GIFLIB
#
  if test "$PHP_GIF_DIR" != "no" && test "$PHP_GIF_DIR" != "yes"; then
    if test -f "$PHP_GIF_DIR/include/giflib/gif_lib.h"; then
      AC_DEFINE(HAVE_GIF,1,[ ])
      PHP_GIF_DIR="$PHP_GIF_DIR"
      PHP_GIF_INCDIR="$PHP_GIF_DIR/include/giflib"
    elif test -f "$PHP_GIF_DIR/include/gif_lib.h"; then
      AC_DEFINE(HAVE_GIF,1,[ ])
      PHP_GIF_DIR="$PHP_GIF_DIR"
      PHP_GIF_INCDIR="$PHP_GIF_DIR/include"
    else
      AC_MSG_ERROR([Can't find GIFLIB headers under "$PHP_GIF_DIR"])
    fi
  else
    for i in /usr/local /usr; do
      if test -f "$i/include/giflib/gif_lib.h"; then
	AC_DEFINE(HAVE_GIF,1,[ ])
        PHP_GIF_DIR="$i"
        PHP_GIF_INCDIR="$i/include/giflib"
      elif test -f "$i/include/gif_lib.h"; then
	AC_DEFINE(HAVE_GIF,1,[ ])
        PHP_GIF_DIR="$i"
        PHP_GIF_INCDIR="$i/include"
      fi
    done
  fi

  dnl # giflib
  AC_MSG_CHECKING([for the location of giflib])
  if test "$PHP_GIF_DIR" = "no"; then
    AC_MSG_RESULT([gif_lib.h not found.])
  else
    AC_MSG_RESULT([$PHP_GIF_DIR])
    if test "z$PHP_LIBDIR" != "z"; then
    dnl PHP5+
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_GIF_DIR/$PHP_LIBDIR, SWFED_SHARED_LIBADD)
    else
    dnl PHP4
      PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_GIF_DIR/lib, SWFED_SHARED_LIBADD)
    fi
    PHP_ADD_INCLUDE($PHP_GIF_INCDIR)
  fi
