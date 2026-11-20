dnl Check PHP version:
dnl ref) https://github.com/kjdev/php-ext-jq/blob/master/config.m4
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)[["-]].*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

dnl PHP 5 support (php5_swfed.c) was dropped after swfed 2.1
if test x"$PHP_VERSION" != "x"; then
    if test $PHP_MAJOR_VERSION -lt 7; then
        AC_MSG_ERROR([need at least PHP 7.2 or newer (use swfed 2.1 or earlier for PHP 5)])
    fi
    if test $PHP_MAJOR_VERSION -eq 7 -a $PHP_MINOR_VERSION -lt 2; then
        AC_MSG_ERROR([need at least PHP 7.2 or newer])
    fi
fi
