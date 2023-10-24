# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/ESP86/Espressif/frameworks/esp-idf-v4.4.5/components/bootloader/subproject"
  "D:/ESP86/tcp/build/bootloader"
  "D:/ESP86/tcp/build/bootloader-prefix"
  "D:/ESP86/tcp/build/bootloader-prefix/tmp"
  "D:/ESP86/tcp/build/bootloader-prefix/src/bootloader-stamp"
  "D:/ESP86/tcp/build/bootloader-prefix/src"
  "D:/ESP86/tcp/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/ESP86/tcp/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
