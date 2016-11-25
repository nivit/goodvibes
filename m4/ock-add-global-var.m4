# ock-add-global-var.m4
#
# Copyright (C) 2016 Arnaud Rebillout <elboulangero@gmail.com>
# 
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# OCK_ADD_GLOBAL_VAR(VARIABLE, VALUE)
# -------------------------------------------------------
# Create a variable at a very global scope. It can be used everywhere:
#  - in the configure.ac
#  - in C files (AC_DEFINE)
#  - in output files (AC_SUBST)
AC_DEFUN([OCK_ADD_GLOBAL_VAR], [
  $1=$2
  AC_DEFINE($1,$2,$3)
  AC_SUBST($1)
])
