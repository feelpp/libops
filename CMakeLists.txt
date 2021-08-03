cmake_minimum_required(VERSION 3.15)

# Set the project name to your project name, my project isn't very descriptive
project(libops CXX)
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
     SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install CACHE PATH "default install directory" FORCE)
 ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)


# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" ON)
option(ENABLE_TESTING "Enable Test Builds" ON)


if(ENABLE_TESTING)
  enable_testing()
#  message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
#  add_subdirectory(test)
endif()

include(GNUInstallDirs)

#
# Lua
#
find_library(MATH_LIBRARY_FOR_LUA m)
message(STATUS "${MATH_LIBRARY_FOR_LUA}")
find_package(Lua 5.2) 
message(STATUS "${LUA_LIBRARY_lua}")
message(STATUS "${LUA_INCLUDE_DIR}")
add_library(lua INTERFACE)  
target_include_directories(lua INTERFACE
      $<BUILD_INTERFACE:${LUA_INCLUDE_DIR}/>
      $<INSTALL_INTERFACE:${LUA_INCLUDE_DIR}> )
message(STATUS "[verdandi] lua libs: ${LUA_LIBRARIES}")      
target_link_libraries(lua INTERFACE ${LUA_LIBRARIES})


add_library(ops SHARED ClassOps.cxx Error.cxx)
target_compile_features(ops PUBLIC cxx_std_17)
target_link_libraries(ops PUBLIC lua)
target_include_directories(ops PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
      $<INSTALL_INTERFACE:include/ops> )

install(TARGETS ops 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

add_executable(opsexample ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp)
target_link_libraries(opsexample ops)


