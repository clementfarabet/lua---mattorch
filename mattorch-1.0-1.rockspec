
package = "mattorch"
version = "1.0-1"

source = {
   url = "mattorch-1.0-1.tgz"
}

description = {
   summary = "Provides a wrapper around Matlab's lib",
   detailed = [[
         This package provides a simple wrapper around
         Matlab's library, to export/import Mat files.
         Matrices of all types are supported and imported
         exported as torch.Tensors.
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

         find_package (Matlab REQUIRED)
         find_package (Torch REQUIRED)

         SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

         include_directories (${MATLAB_INCLUDE_DIR} ${TORCH_INCLUDE_DIR})
         add_library (mattorch SHARED mattorch.c)
         link_directories (${TORCH_LIBRARY_DIR})
         target_link_libraries (mattorch ${TORCH_LIBRARIES} ${MATLAB_LIBRARIES})

         set (CMAKE_INSTALL_PREFIX ${TORCH_PREFIX})
         install_files(/share/lua/5.1/ mattorch.lua)
         install_targets(/lib/lua/5.1/ mattorch)
   ]],

   variables = {}
}
