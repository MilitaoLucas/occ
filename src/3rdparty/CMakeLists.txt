add_library(_subprocess
    STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/CowData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/environ.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/pipe.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/PipeVar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ProcessBuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ProcessBuilder_posix.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ProcessBuilder_windows.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/shell_utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utf8_to_utf16.cpp"
)

target_link_libraries(_subprocess
    PUBLIC
    Threads::Threads
)
if(UNIX)
    target_sources(_subprocess PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/pipe.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ProcessBuilder_posix.cpp"
    )
elseif(WIN32)
    target_sources(_subprocess PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ProcessBuilder_windows.cpp"
    )
endif()
target_compile_features(_subprocess PUBLIC cxx_std_20)

target_include_directories(_subprocess
    PUBLIC
    $<BUILD_INTERFACE:${OCC_INCLUDE_DIR}/occ/3rdparty/subprocess/>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/occ/3rdparty/subprocess/>
)

if("${MSVC}")
    target_compile_options(_subprocess PUBLIC -Zc:__cplusplus)
endif()

add_subdirectory(gau2grid)
add_subdirectory(libecpint)
