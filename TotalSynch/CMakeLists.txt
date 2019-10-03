cmake_minimum_required(VERSION 3.15)
project(TotalSynch)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(TotalSynch
        CFSElement.cpp
        CFSElement.h
        help.txt
        main.cpp
        MD5.cpp
        MD5.h
        settings.h
        Utils.cpp
        Utils.h)
