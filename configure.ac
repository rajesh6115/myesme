# Autoconf Minimum Version Required Check
AC_PREREQ([2.68])
# Initilise Autoconf with project Name and Version Details
AC_INIT([libsmpp], [1.0.0], [sahoorajesh.d@gmail.com], [libsmpp], [https://github.com/rajesh6115])
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

# libmysqlclient library finding
#mysql_CPPFLAGS=`mysql_config --cflags`
#mysql_LDFLAGS=`mysql_config --libs`
# Generate These Files for Sucessfully Build
AC_CONFIG_FILES([
                Makefile
                src/Makefile
])
AC_OUTPUT

