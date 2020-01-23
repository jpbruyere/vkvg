# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindHarfBuzz
# ------------
#
# Find the HarfBuzz text shaping engine includes and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` target:
#
# ``HarfBuzz::HarfBuzz``
#   The Harfbuzz ``harfbuzz`` library, if found
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``HarfBuzz_FOUND``
#   true if the HarfBuzz headers and libraries were found
# ``HarfBuzz_INCLUDE_DIRS``
#   directories containing the Harfbuzz headers.
# ``HarfBuzz_LIBRARIES``
#   the library to link against

find_path(HarfBuzz_INCLUDE_DIR
	NAMES harfbuzz/hb.h
)
find_library(HarfBuzz_LIBRARY
	NAMES harfbuzz libharfbuzz
)

# set the user variables
set(HarfBuzz_LIBRARIES "${HarfBuzz_LIBRARY}")


set(HarfBuzz_LIBRARIES ${HarfBuzz_LIBRARY})
set(HarfBuzz_INCLUDE_DIRS ${HarfBuzz_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HarfBuzz
  DEFAULT_MSG
  HarfBuzz_LIBRARY HarfBuzz_INCLUDE_DIR)

mark_as_advanced(HarfBuzz_INCLUDE_DIR HarfBuzz_LIBRARY)

if(HarfBuzz_FOUND AND NOT TARGET HarfBuzz::HarfBuzz)
  add_library(HarfBuzz::HarfBuzz UNKNOWN IMPORTED)
  set_target_properties(HarfBuzz::HarfBuzz PROPERTIES
	IMPORTED_LOCATION "${HarfBuzz_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${HarfBuzz_INCLUDE_DIRS}")
endif()
