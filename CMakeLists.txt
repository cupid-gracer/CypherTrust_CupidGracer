cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
project(HitBTC)
set(requiredlibs)

find_package(cpprestsdk REQUIRED)
 include_directories($(cpprestsdk_DIR))
 set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)

find_package(Threads REQUIRED)
set(requiredlibs ${requiredlibs} ${CMAKE_THREAD_LIBS_INIT})

find_package(rapidjson REQUIRED)
if(RAPIDJSON_FOUND)
  include_directories(${RAPIDJSON_INCLUDE_DIRS})
else(RAPIDJSON_FOUND)
  message(FATAL_ERROR "Could not find RapidJson Library.")
endif(RAPIDJSON_FOUND)

find_package(Crypto++ REQUIRED)
if(CRYPTO++_FOUND)
  include_directories(${CRYPTO++_INCLUDE_DIR})
  set(requiredlibs ${requiredlibs} ${CRYPTO++_LIBRARIES})
else(CRYPTO++_FOUND)
  message(FATAL_ERROR "Could not find Crypto++ library")
endif(CRYPTO++_FOUND)

find_package(CURL)
if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})
  set(requiredlibs ${requiredlibs} ${CURL_LIBRARIES})
else(CURL_FOUND)
  message(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

if(${CMAKE_CXX_COMPILER} MATCHES "icpc.*$")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -xHOST -unroll-aggressive -inline-level=2")
endif()

find_package(Casablanca REQUIRED)
  include_directories(${CASABLANCA_INCLUDE_DIR})
  set(requiredlibs ${requiredlibs} ${CASABLANCA_LIBRARIES})


include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(cp ${SOURCES})

target_link_libraries(cp ${requiredlibs} cpprestsdk::cpprest)

find_path(HIREDIS_INC_DIR hiredis)
target_include_directories(cp PUBLIC ${HIREDIS_INC_DIR})

find_library(HIREDIS_LIB_DIR hiredis)
target_link_libraries(cp ${HIREDIS_LIB_DIR})

find_path(REDIS_PLUS_PLUS_INC_DIR sw)
target_include_directories(cp PUBLIC ${REDIS_PLUS_PLUS_INC_DIR})

find_library(REDIS_PLUS_PLUS_LIB_DIR redis++)
target_link_libraries(cp ${REDIS_PLUS_PLUS_LIB_DIR})