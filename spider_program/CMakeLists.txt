cmake_minimum_required(VERSION 3.20)
set (CMAKE_CXX_STANDARD 20)

set(PostgreSQL_ADDITIONAL_VERSIONS "16")

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)  
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0A00")
endif()

set (BOOST_ROOT "C:/boost_1_88_0") 
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/libs")
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)

#set(OPENSSL_ROOT_DIR "C:/Search engine/spider_program/spider_program")
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_MSVC_STATIC_RT ON)
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIBRARIES 
    "${OPENSSL_ROOT_DIR}/lib/libssl.lib" 
    "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib"
)
#set(OPENSSL_USE_STATIC_LIBS TRUE)


project(spider_program)


set(SOURCE_FILES "main.cpp"
                 "database.cpp" "database.h"
                 "indexer.cpp" "indexer.h"
                 "ini_parser.cpp" "ini_parser.h"
                 "spider.cpp" "spider.h"
                 "safe_queue.cpp" "safe_queue.h"
                 "thread_pool.cpp" "thread_pool.h"
                 #"search_program.cpp" "search_program.h"
                 "search_server.cpp" "search_server.h"
                 #"test_files/fast_test_spider.cpp"
                  )


add_executable(spider_program ${SOURCE_FILES})

if(WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/spider.ini
        ${CMAKE_CURRENT_BINARY_DIR}/spider.ini
        COPYONLY
    )
endif()

add_subdirectory(libpqxx-7.10.1)


find_package( Boost REQUIRED COMPONENTS locale)
find_package(OpenSSL REQUIRED)

if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL version: ${OPENSSL_VERSION}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found!")
endif()

include_directories(${OPENSSL_INCLUDE_DIR})


if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()


target_compile_features(spider_program PRIVATE cxx_std_17)

target_link_libraries(spider_program PRIVATE pqxx
                                             Boost::locale
                                             ${Boost_LIBRARIES}
                                             OpenSSL::SSL
                                             OpenSSL::Crypto)#${Boost_LIBRARIES})# Boost::locale)

