#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <imd/client.hpp>

TEST_CASE( "put-get consistency" )
{
    imd::client client { "127.0.0.1" , 9424 };

    client.put( "lib_name" , "imd" ).get();

    REQUIRE( client.get( "lib_name" ).get() == "imd" );
}

TEST_CASE( "put empty value" )
{
    imd::client client { "127.0.0.1" , 9424 };

    client.put( "key_for_empty_value" , "" ).get();

    REQUIRE( client.get( "key_for_empty_value" ).get() == "" );
}

TEST_CASE( "put key which contains white space character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.put( "test _" , "any" ) , std::invalid_argument );
}

TEST_CASE( "put empty key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.put( "" , "any" ) , std::invalid_argument );
}

TEST_CASE( "put key which contains carriage return character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.put( "test \r" , "any" ) , std::invalid_argument );
}

TEST_CASE( "put key which contains new line character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.put( "test _\n" , "any" ) , std::invalid_argument );
}

TEST_CASE( "get unexisted key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.get( "unexisted_key" ).get() , imd::exception );
}

TEST_CASE( "get empty key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.get( "" ) , std::invalid_argument );
}

TEST_CASE( "get key which contains carriage return character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.get( "test \r" ) , std::invalid_argument );
}

TEST_CASE( "get key which contains new line character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.get( "test _\n" ) , std::invalid_argument );
}

TEST_CASE( "put-remove key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    client.put( "removal_key" , "value" );
    client.remove( "removal_key" ).get();

    REQUIRE_THROWS_AS( client.get( "removal_key" ).get() , imd::exception );
}

TEST_CASE( "remove unexisted key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    client.remove( "unexisted_key" ).get();

    REQUIRE( true );
}

TEST_CASE( "remove empty key" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.remove( "" ) , std::invalid_argument );
}

TEST_CASE( "remove key which contains carriage return character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.remove( "test \r" ) , std::invalid_argument );
}

TEST_CASE( "remove key which contains new line character" )
{
    imd::client client { "127.0.0.1" , 9424 };

    REQUIRE_THROWS_AS( client.remove( "test _\n" ) , std::invalid_argument );
}