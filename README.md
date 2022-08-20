# imd-client
Header-only client library for imd-srv.

- It is a header only library.

- Depends on Boost.Asio and Boost.Thread.

- Provides required files for cmake to be used with `find_package( imd )`

- Requires a C++17 compliant compiler

Only tested on Ubuntu 20.04 with GCC 11.2.0 and Boost v1.74

## Example

``` C++
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
```

```console
calynr@calynr-dev:~/repo/imd-client/build/example$ ./example 
green_1.png
green_2.png
green_3.png
green_4.png

It is an intentional error.
Here is the error message for the operation :
NO_SUCH_A_KEY
```

## How to use ?
### Way 1. Install to the system
- Install `imd-client` as system-wide.
```bash
cd $(mktemp -d)
git clone https://github.com/OzanCansel/imd-client.git
cd imd-client
mkdir build && cd build
cmake ..
sudo cmake --build . --target install -- -j$(nproc)
```

- Include `imd-client` to your cmake project with `find_package( imd-client )`
``` cmake
cmake_minimum_required( VERSION 3.10 )
project( my_project )

# Allows you to use imd-client
find_package( Boost COMPONENTS thread REQUIRED )
find_package( imd-client REQUIRED )

add_executable( my_binary main.cpp )

target_link_libraries( my_binary PRIVATE imd::imd )
```

### Way 2. Add as a subdirectory
- Add as an subdirectory to your existing cmake project.

```bash
cd already_existing_project
git clone https://github.com/OzanCansel/imd-client.git
```
``` cmake
cmake_minimum_required( VERSION 3.10 )
project( already_existing_project )

# Allows you to use imd-client
add_subdirectory( imd-client EXCLUDE_FROM_ALL )

add_executable( my_binary main.cpp )

target_link_libraries( my_binary PRIVATE imd::imd )
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[MIT](https://raw.githubusercontent.com/OzanCansel/imd-client/master/LICENSE)
