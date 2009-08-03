# Configure paths for ZTHREAD
# by Eric Crahen <zthread@code-foo.com>

dnl Detect the library and include paths for ZThreads, perform some test
dnl compilations.
dnl
dnl Should be used in AC_PROG_CC mode before the swtich to C++ if any is made
dnl (eg before AC_LANG_CPLUSPLUS)
dnl
dnl --with-zthread-prefix : Skip detection, use this general path
dnl --with-zthread-exec-prefix : Skip detecting the zthread-config tool
dnl 
dnl Sets the following variables.
dnl
dnl ZTHREAD_CXXFLAGS
dnl ZTHREAD_LIBS
dnl 
AC_DEFUN([AM_PATH_ZTHREAD],
[


AC_ARG_WITH(zthread-prefix,[  --with-zthread-prefix=PFX   Prefix where ZTHREAD is installed (optional)],
            zthread_prefix="$withval", zthread_prefix="")
AC_ARG_WITH(zthread-exec-prefix,[  --with-zthread-exec-prefix=PFX Exec prefix where ZTHREAD is installed (optional)],
            zthread_exec_prefix="$withval", zthread_exec_prefix="")
AC_ARG_ENABLE(zthreadtest, [  --disable-zthreadtest       Do not try to compile and run a test ZTHREAD program],
		    , enable_zthreadtest=yes)

  if test x$zthread_exec_prefix != x ; then
     zthread_args="$zthread_args --exec-prefix=$zthread_exec_prefix"
     if test x${ZTHREAD_CONFIG+set} != xset ; then
        ZTHREAD_CONFIG=$zthread_exec_prefix/bin/zthread-config
     fi
  fi
  if test x$zthread_prefix != x ; then
     zthread_args="$zthread_args --prefix=$zthread_prefix"
     if test x${ZTHREAD_CONFIG+set} != xset ; then
        ZTHREAD_CONFIG=$zthread_prefix/bin/zthread-config
     fi
  fi

  AC_PATH_PROG(ZTHREAD_CONFIG, zthread-config, no)
  min_zthread_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for ZTHREAD - version >= $min_zthread_version)
  no_zthread=""
  if test "$ZTHREAD_CONFIG" = "no" ; then
    no_zthread=yes
  else
    ZTHREAD_CXXFLAGS=`$ZTHREAD_CONFIG $zthreadconf_args --cflags`
    ZTHREAD_LIBS=`$ZTHREAD_CONFIG $zthreadconf_args --libs`

    zthread_major_version=`$ZTHREAD_CONFIG $zthread_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    zthread_minor_version=`$ZTHREAD_CONFIG $zthread_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    zthread_micro_version=`$ZTHREAD_CONFIG $zthread_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_zthreadtest" = "xyes" ; then
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CXXFLAGS="$CXXFLAGS $ZTHREAD_CXXFLAGS -fpermissive"
      LIBS="$LIBS"

dnl
dnl Now check if the installed ZTHREAD is sufficiently new. (Also sanity
dnl checks the results of zthread-config to some extent
dnl

AC_LANG_PUSH(C++)
      rm -f conf.zthreadtest
      AC_TRY_RUN([


#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

  int major, minor, micro;
  char tmp_version[256];

  { FILE *fp = fopen("conf.zthreadtest", "a"); if ( fp ) fclose(fp); }


  /* HP/UX 9 (%@#!) writes to sscanf strings */
  strcpy(tmp_version, "$min_zthread_version");
  
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {

    printf("%s, bad version string\n", "$min_zthread_version");
    return 1;

  }

  if (($zthread_major_version > major) ||
     (($zthread_major_version == major) && ($zthread_minor_version > minor)) ||
     (($zthread_major_version == major) && ($zthread_minor_version == minor) && 
     ($zthread_micro_version >= micro))) {
      return 0;
  } else {

    printf("\n*** 'zthread-config --version' returned %d.%d.%d, but the minimum version\n", $zthread_major_version, $zthread_minor_version, $zthread_micro_version);
    printf("*** of ZThread required is %d.%d.%d. If zthread-config is correct, then it is\n", major, minor, micro);
    printf("*** best to upgrade to the required version.\n");
    printf("*** If zthread-config was wrong, set the environment variable ZTHREAD_CONFIG\n");
    printf("*** to point to the correct copy of zthread-config, and remove the file\n");
    printf("*** config.cache before re-running configure\n");

    return 1;
  }

}

],, no_zthread=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       
    CXXFLAGS="$ac_save_CXXFLAGS"
    LIBS="$ac_save_LIBS"
    fi
  fi

  if test "x$no_zthread" = x ; then

     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     

  else

     AC_MSG_RESULT(no)

     if test "$ZTHREAD_CONFIG" = "no" ; then

       echo "*** The zthread-config script installed by ZThread could not be found"
       echo "*** If ZThread was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the ZTHREAD_CONFIG environment variable to the"
       echo "*** full path to zthread-config."

     else
       if test -f conf.zthreadtest ; then
        :
       else

          echo "*** Could not run ZThread test program, checking why..."
          CXXFLAGS="$CXXFLAGS $ZTHREAD_CXXFLAGS"
          LIBS="$LIBS $ZTHREAD_LIBS"

          echo $LIBS;

          AC_TRY_LINK([#include "zthread/ZThread.h"], 
                      [ return 0; ], [
          echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ZThread or finding the wrong"
          echo "*** version of ZThread. If it is not finding ZThread, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	        echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ZThread was incorrectly installed"
          echo "*** or that you have moved ZThread since it was installed. In the latter case, you"
          echo "*** may want to edit the zthread-config script: $ZTHREAD_CONFIG" ])
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi

     ZTHREAD_CXXFLAGS=""
     ZTHREAD_LIBS=""
     ifelse([$3], , :, [$3])

  fi

  AC_SUBST(ZTHREAD_CXXFLAGS)
  AC_SUBST(ZTHREAD_LIBS)

  rm -f conf.zthreadtest

])
