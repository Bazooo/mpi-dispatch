cmake_minimum_required(VERSION 3.16)
project(mpi_dispatch)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpicxx)

add_executable(mpi_dispatch main.cpp)