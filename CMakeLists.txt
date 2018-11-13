cmake_minimum_required(VERSION 3.0)
project(gofer2 C CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Modules")

add_executable (gofer2 src/main.cpp src/utility.cpp src/datastructures_go.cpp 
                src/datastructures_flat.cpp src/datastructures_hierarchical.cpp
                src/term.cpp src/test.cpp src/set_memberships.cpp 
                src/handle_enrichment.cpp src/handle_translation.cpp 
                src/handle_get_sets.cpp src/BSlogger.hpp)

include_directories(src)

# Boost
find_package (Boost REQUIRED COMPONENTS system filesystem chrono thread program_options)
include_directories(${Boost_INCLUDE_DIR})

# OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# cpprestsdk
find_package(cpprest REQUIRED)
include_directories(${CPPREST_INCLUDE_DIR})

# Threads
find_package(Threads REQUIRED)

# Link
if (BUILD_STATIC)
  target_link_libraries (gofer2 -static ${CMAKE_THREAD_LIBS_INIT} 
                         ${CPPREST_LIBRARIES} ${Boost_LIBRARIES} 
                         ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY}) 
else()
  target_link_libraries (gofer2 ${CPPREST_LIBRARIES} ${Boost_LIBRARIES} 
                         ${CMAKE_THREAD_LIBS_INIT}
                         ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY}) 
endif()
