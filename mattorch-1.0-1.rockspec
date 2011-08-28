
package = "mattorch"
version = "1.0-1"

source = {
   url = "mattorch-1.0-1.tgz"
}

description = {
   summary = "Provides an interface between Matlab's matrix format and Torch7's tensor.",
   detailed = [[
         This package provides a simple wrapper around
         Matlab's library, to export/import Mat files
         from/to the torch.Tensor class (Torch7).
         Matlab matrices of all types can be imported.
   ]],
   homepage = "",
   license = "MIT/X11" -- or whatever you like
}

dependencies = {
   "lua >= 5.1",
   "xlua"
}

build = {
   type = "cmake",

   cmake = [[
         cmake_minimum_required(VERSION 2.8)

         set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

         # infer path for Torch7
         string (REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" TORCH_PREFIX "${CMAKE_INSTALL_PREFIX}" )
         message (STATUS "Found Torch7, installed in: " ${TORCH_PREFIX})

         find_package (Matlab REQUIRED)
         find_package (Torch REQUIRED)

         SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

         include_directories (${MATLAB_INCLUDE_DIR} ${TORCH_INCLUDE_DIR})
         add_library (mattorchlive STATIC mattorchlive.c)
         link_directories (${TORCH_LIBRARY_DIR})
         target_link_libraries (mattorchlive ${TORCH_LIBRARIES} ${MATLAB_LIBRARIES})

         include_directories (${MATLAB_INCLUDE_DIR} ${TORCH_INCLUDE_DIR})
         add_library (mattorch SHARED mattorch.c)
         link_directories (${TORCH_LIBRARY_DIR})
         target_link_libraries (mattorch ${TORCH_LIBRARIES} ${MATLAB_LIBRARIES})

         install_files(/lua mattorch.lua)
         install_targets(/lib mattorch)
   ]],

   variables = {
      CMAKE_INSTALL_PREFIX = "$(PREFIX)"
   }
}
