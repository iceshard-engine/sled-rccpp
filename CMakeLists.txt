cmake_minimum_required(VERSION 3.15)
project(sled-rccpp CXX)

find_package(rccpp REQUIRED)

add_library(sled
    "private/runtime_types.cxx"
    "private/runtime_object.cxx"
    "private/runtime_object_actual.cxx"
    "private/runtime_object_constructor.cxx"
    "private/runtime_ptr.cxx"
    "public/sled/runtime_uniqueid.hxx"
    "public/sled/runtime_types.hxx"
    "public/sled/runtime_object_interface.hxx"
    "public/sled/runtime_object.hxx"
    "public/sled/runtime_object_actual.hxx"
    "public/sled/runtime_object_constructor.hxx"
    "public/sled/runtime_object_constructor_concrete.hxx"
    "public/sled/runtime_object_serializer.hxx"
    "public/sled/runtime_object_owner.hxx"
    "public/sled/runtime_swappable_ptr.hxx"
    "public/sled/runtime_ptr.hxx"
)

target_include_directories(sled PUBLIC public ${rccpp_INCLUDE_DIR})
target_link_libraries(sled rccpp::rccpp)

if (MSVC)
    target_compile_options(sled PRIVATE /utf-8)
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/public/"
    DESTINATION "include"
    FILES_MATCHING
    PATTERN "*.hxx"
)

install(TARGETS sled)
