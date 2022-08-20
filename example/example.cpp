#include <imd/client.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <iostream>

int main()
{
    imd::client client {
        "127.0.0.1" ,
        9424
    };

    std::vector<std::future<void>>        puts;
    std::vector<std::future<std::string>> gets;

    puts.push_back( client.put( "picture_1" , "green_1.png" ) );
    puts.push_back( client.put( "picture_2" , "green_2.png" ) );
    puts.push_back( client.put( "picture_3" , "green_3.png" ) );
    puts.push_back( client.put( "picture_4" , "green_4.png" ) );
    gets.push_back( client.get( "picture_1" ) );
    gets.push_back( client.get( "picture_2" ) );
    gets.push_back( client.get( "picture_3" ) );
    gets.push_back( client.get( "picture_4" ) );

    for ( auto& f : puts )
        f.get();

    for ( auto& f : gets )
        std::cout << f.get() << std::endl;

    client.remove( "picture_2" ).get();

    try
    {
        client.get( "picture_2" ).get();
    }
    catch ( const imd::exception& e )
    {
        std::cout << "\nIt is an intentional error.\n"
                  << "Here is the error message for the operation :\n"
                  << e.what()
                  << std::endl;
    }
}