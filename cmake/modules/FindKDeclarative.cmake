# Copyright 2012 Lamarque V. Souza <Lamarque.Souza.ext@basyskom.com>
# Find the native KDeclarative includes and library
# This module defines
#  KDECLARATIVE_INCLUDE_DIR, where to find kdeclarative.h
#  KDECLARATIVE_LIBRARIES, the libraries needed to use KDeclarative.
#  KDECLARATIVE_FOUND, If false, do not try to use KDeclarative.
# also defined, but not for general use are
#  KDECLARATIVE_LIBRARY, where to find the KDeclarative library.

#=============================================================================
# Copyright 2012 Lamarque V. Souza <Lamarque.Souza.ext@basyskom.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FIND_PATH(KDECLARATIVE_INCLUDE_DIR kdeclarative.h)

FIND_LIBRARY(KDECLARATIVE_LIBRARY NAMES kdeclarative )

# handle the QUIETLY and REQUIRED arguments and set KDECLARATIVE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KDECLARATIVE DEFAULT_MSG KDECLARATIVE_LIBRARY KDECLARATIVE_INCLUDE_DIR)

IF(KDECLARATIVE_FOUND)
  SET(KDECLARATIVE_LIBRARIES ${KDECLARATIVE_LIBRARY})
  SET(KDECLARATIVE_INCLUDE_DIRS ${KDECLARATIVE_INCLUDE_DIR})
ENDIF(KDECLARATIVE_FOUND)

MARK_AS_ADVANCED(KDECLARATIVE_LIBRARY KDECLARATIVE_INCLUDE_DIR )
