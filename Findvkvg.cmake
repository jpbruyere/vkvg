find_path(vkvg_INCLUDE_DIR vkvg.h)

find_library(vkvg_LIBRARY NAMES vkvg)

# handle the QUIETLY and REQUIRED arguments and set vkvg_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vkvg DEFAULT_MSG
  vkvg_LIBRARY  vkvg_INCLUDE_DIR)

if(vkvg_FOUND)
  set( vkvg_LIBRARIES ${vkvg_LIBRARY} )
endif()

mark_as_advanced(vkvg_INCLUDE_DIR vkvg_LIBRARY vkvg_LIBRARIES)
