# gv-feature.m4
#
# Copyright (C) 2016 Arnaud Rebillout <elboulangero@gmail.com>
# 
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# GV_FEATURE_ENABLE(FEATURE, FEATURE-NAME, DEFAULT-VALUE)
# -------------------------------------------------------
# Allow user to decide whether an optional feature should be built.
# This macro simply sets the variable 'enable_FEATURE' to:
#  - 'yes'           if the feature was enabled by user.
#  - 'no'            if the feature was disabled by user.
#  - 'DEFAULT-VALUE' if the feature was not set by user.
#                    Possible values are: 'yes', 'no', 'auto'.
#                    'auto' makes sense only if GV_FEATURE_PKG_CHECK()
#		     is invoked afterward.
# Moreover, this macro outputs a line to report what's going on.
AC_DEFUN([GV_FEATURE_ENABLE], [
  AC_MSG_CHECKING([whether to enable $2])
  AC_ARG_ENABLE([$1], AS_HELP_STRING([--enable-translit($1, [_], [-])], [Enable $2]))
  AS_IF([test "$enable_$1" = ""], [enable_$1=$3])
  AC_MSG_RESULT([$enable_$1])
])

# GV_FEATURE_PKG_CHECK(FEATURE, VARIABLE-PREFIX, MODULES)
# -------------------------------------------------------
# Check modules needed for a given feature.
# Assume the existence of the variable 'enable_FEATURE',
# which value dictates the behavior of this macro:
#  - 'auto' invoke PKG_CHECK_EXISTS() and modify the variable
#           'enable_FEATURE' depending on the result.
#  - 'yes'  invoke PKG_CHECK_MODULES().
#  - *      do nothing.
# Moreover, this macro outputs lines to report what's going on
# (this is done by the underlying PKG_CHECK calls).
AC_DEFUN([GV_FEATURE_PKG_CHECK], [
  AS_IF([test "$enable_$1" = "auto"], [
    AC_MSG_CHECKING([for $2 existence])
    PKG_CHECK_EXISTS([$3], [enable_$1=yes], [enable_$1=no])
    AC_MSG_RESULT([$enable_$1])
  ])
  AS_IF([test "$enable_$1" = "yes"], [
    PKG_CHECK_MODULES([$2], [$3])
  ])
])
