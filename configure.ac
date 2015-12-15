# Autoconf Minimum Version Required Check
AC_PREREQ([2.68])
# Initilise Autoconf with project Name and Version Details
AC_INIT([myesme], [1.0.0], [sahoorajesh.d@gmail.com], [myesme], [https://github.com/rajesh6115])
# Put autotools auxiliary files in a subdir, so they don't clutter top dir:
AC_CONFIG_AUX_DIR([build-aux])
# Initialise Automake with Default version Number and Default CPP flags
AM_INIT_AUTOMAKE([1.11 -Wall -Werror ])
# Check for some Unique file in the Project
AC_CONFIG_SRCDIR([src/])
# Check for C Compiler
AC_PROG_CC
AM_PROG_CC_C_O
# Check for C++ Compiler
AC_PROG_CXX
# Check for stdc++11 support
# AC_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11"])
# Check for pthread
AC_CHECK_LIB(pthread, pthread_create,[AC_DEFINE(ENABLE_THREADS, 1, [Define this to enable threads.])],[AC_MSG_ERROR([required library pthread missing])])
# config.h will be created and default header
AC_CONFIG_HEADERS([config.h])
# all Macro in this folder
AC_CONFIG_MACRO_DIR([m4])
# AR required to create static library
AM_PROG_AR
# Check of library created using ranlib
AC_PROG_RANLIB
AC_ENABLE_SHARED
# libtool Required  
LT_PREREQ([2.2])
LT_INIT([dlopen])
AC_ENABLE_SHARED
AC_PROG_LIBTOOL(libtool)
# check for libxml2
PKG_CHECK_MODULES([libxml2], [libxml-2.0])

# check for libevent
AC_ARG_WITH([libevent], [  --with-libevent            enable libevent])
if test "$with_libevent" == "yes"
then
	AC_MSG_NOTICE([Checking for libevent2 and its requirements.])
	PKG_CHECK_MODULES([libevent], [libevent >= 2.0], with_libevent=yes, with_libevent=no)
	if test "$with_libevent" == "no"
	then
		AC_MSG_NOTICE([libevent Enabled But requirement not matching ])
		exit 1
	else
		AC_DEFINE(WITH_LIBEVENT, 1 ,[libevent is Enabled]) 
	fi
fi
# libmysqlclient library finding
#mysql_CPPFLAGS=`mysql_config --cflags`
#mysql_LDFLAGS=`mysql_config --libs`
# Generate These Files for Sucessfully Build
AC_CONFIG_FILES([
                Makefile
                src/Makefile
])
AC_OUTPUT

