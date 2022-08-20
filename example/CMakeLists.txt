add_executable( example example.cpp )
target_link_libraries( example PRIVATE imd::imd )
target_include_directories( example PRIVATE imd::imd )