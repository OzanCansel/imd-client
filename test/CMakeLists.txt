add_executable( imd-test imd-test.cpp catch.hpp )
target_link_libraries( imd-test imd::imd )
add_test( NAME imd-test COMMAND imd-test -s )