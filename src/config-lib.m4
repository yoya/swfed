dnl --with-*-dir: not given = search (auto), DIR = use that prefix,
dnl --without-png-dir / --without-gif-dir = build without PNG / GIF support.
if test -z "$PHP_ZLIB_DIR"; then
PHP_ARG_WITH(zlib-dir, for the location of ZLIB,
[  --with-zlib-dir[=DIR]   swfed: Set the path to ZLIB install prefix.], auto, no)
fi

if test -z "$PHP_PNG_DIR"; then
PHP_ARG_WITH(png-dir, for the location of LIBPNG,
[  --with-png-dir[=DIR]   swfed: Set the path to LIBPNG install prefix (--without-png-dir: no PNG support).], auto, no)
fi

if test -z "$PHP_GIF_DIR"; then
PHP_ARG_WITH(gif-dir, for the location of GIFLIB,
[  --with-gif-dir[=DIR]   swfed: Set the path to GIFLIB install prefix (--without-gif-dir: no GIF support).], auto, no)
fi

dnl swfed_find_prefix <dir|auto|yes> <header> <include subdir> <pkg-config module> <candidate prefixes...>
dnl sets SWFED_FOUND_DIR and SWFED_FOUND_INCDIR (empty if not found).
dnl With auto/yes, pkg-config is tried first, then the first candidate that has the header.
swfed_find_prefix() {
  swfed_given=$1; swfed_header=$2; swfed_sub=$3; swfed_pc=$4
  shift 4
  SWFED_FOUND_DIR=""
  SWFED_FOUND_INCDIR=""
  if test "$swfed_given" != "auto" && test "$swfed_given" != "yes"; then
    swfed_dirs="$swfed_given"
  else
    swfed_dirs=""
    swfed_pkg_config=${PKG_CONFIG:-pkg-config}
    if test -n "$swfed_pc" && $swfed_pkg_config --exists "$swfed_pc" 2>/dev/null; then
      swfed_dirs=`$swfed_pkg_config --variable=prefix "$swfed_pc" 2>/dev/null`
    fi
    swfed_dirs="$swfed_dirs $*"
  fi
  for swfed_dir in $swfed_dirs; do
    if test -f "$swfed_dir/include/$swfed_sub/$swfed_header"; then
      SWFED_FOUND_DIR="$swfed_dir"
      SWFED_FOUND_INCDIR="$swfed_dir/include/$swfed_sub"
      break
    elif test -f "$swfed_dir/include/$swfed_header"; then
      SWFED_FOUND_DIR="$swfed_dir"
      SWFED_FOUND_INCDIR="$swfed_dir/include"
      break
    fi
  done
}

dnl /opt/homebrew is Homebrew on Apple Silicon; zlib is keg-only there.
SWFED_SEARCH_DIRS="/usr/local /usr /opt/homebrew /opt/local"

#
# ZLIB (required)
#
  AC_MSG_CHECKING([for the location of zlib])
  swfed_find_prefix "$PHP_ZLIB_DIR" zlib.h zlib zlib \
      $SWFED_SEARCH_DIRS /opt/homebrew/opt/zlib /usr/local/opt/zlib
  if test -z "$SWFED_FOUND_DIR"; then
    AC_MSG_ERROR([swfed support requires ZLIB. Use --with-zlib-dir=<DIR> to specify prefix where ZLIB include and library are located (e.g. --with-zlib-dir=/opt/homebrew/opt/zlib)])
  fi
  AC_MSG_RESULT([$SWFED_FOUND_DIR])
  PHP_ADD_LIBRARY_WITH_PATH(z, $SWFED_FOUND_DIR/${PHP_LIBDIR:-lib}, SWFED_SHARED_LIBADD)
  PHP_ADD_INCLUDE($SWFED_FOUND_INCDIR)

#
# LIBPNG (optional)
#
  AC_MSG_CHECKING([for the location of libpng])
  if test "$PHP_PNG_DIR" = "no"; then
    AC_MSG_RESULT([disabled])
  else
    swfed_find_prefix "$PHP_PNG_DIR" png.h libpng libpng \
        $SWFED_SEARCH_DIRS /opt/homebrew/opt/libpng /usr/local/opt/libpng
    if test -n "$SWFED_FOUND_DIR"; then
      AC_MSG_RESULT([$SWFED_FOUND_DIR])
      AC_DEFINE(HAVE_PNG,1,[ ])
      PHP_ADD_LIBRARY_WITH_PATH(png, $SWFED_FOUND_DIR/${PHP_LIBDIR:-lib}, SWFED_SHARED_LIBADD)
      PHP_ADD_INCLUDE($SWFED_FOUND_INCDIR)
    elif test "$PHP_PNG_DIR" != "auto" && test "$PHP_PNG_DIR" != "yes"; then
      AC_MSG_ERROR([Can't find LIBPNG headers under "$PHP_PNG_DIR"])
    else
      AC_MSG_RESULT([not found])
      AC_MSG_WARN([building without PNG support. Use --with-png-dir=<DIR> to specify prefix where LIBPNG include and library are located])
    fi
  fi

#
# GIFLIB (optional)
#
  AC_MSG_CHECKING([for the location of giflib])
  if test "$PHP_GIF_DIR" = "no"; then
    AC_MSG_RESULT([disabled])
  else
    swfed_find_prefix "$PHP_GIF_DIR" gif_lib.h giflib "" \
        $SWFED_SEARCH_DIRS /opt/homebrew/opt/giflib /usr/local/opt/giflib
    if test -n "$SWFED_FOUND_DIR"; then
      AC_MSG_RESULT([$SWFED_FOUND_DIR])
      AC_DEFINE(HAVE_GIF,1,[ ])
      PHP_ADD_LIBRARY_WITH_PATH(gif, $SWFED_FOUND_DIR/${PHP_LIBDIR:-lib}, SWFED_SHARED_LIBADD)
      PHP_ADD_INCLUDE($SWFED_FOUND_INCDIR)
    elif test "$PHP_GIF_DIR" != "auto" && test "$PHP_GIF_DIR" != "yes"; then
      AC_MSG_ERROR([Can't find GIFLIB headers under "$PHP_GIF_DIR"])
    else
      AC_MSG_RESULT([not found])
      AC_MSG_WARN([building without GIF support. Use --with-gif-dir=<DIR> to specify prefix where GIFLIB include and library are located])
    fi
  fi
