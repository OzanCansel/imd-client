#include "../client.hpp"
#include <future>
#include <stdexcept>
#include <regex>

namespace imd
{

client::client(
    std::string ip_addr ,
    int port ,
    int n_threads ,
    int max_connections
)
    :   m_ip_addr { move( ip_addr ) }
    ,   m_port { port }
    ,   m_conn_idx {}
    ,   m_executor { n_threads }
{
    for ( auto i = 0; i < max_connections; ++i )
    {
        auto c = connection::create(
            m_ip_addr ,
            m_port ,
            m_executor.context()
        );

        c->start();
        m_connections.push_back(
            move( c )
        );
    }
}

std::future<std::string> client::get( const std::string& key )
{
    validate_key( key );

    return next()->get( key );
}

std::future<void> client::put( const std::string& key , const std::string& value )
{
    validate_key( key );

    return next()->put( key , value );
}

std::future<void> client::remove( const std::string& key )
{
    validate_key( key );

    return next()->remove( key );
}

connection::pointer& client::next()
{
    return m_connections[ m_conn_idx++ % std::size( m_connections ) ];
}

void client::validate_key( const std::string& key ) const
{
    if ( key.empty() )
        throw std::invalid_argument {
            "'key' is empty."
        };

    if ( regex_search( key , std::regex { R"(\s)" } ) )
        throw std::invalid_argument {
            "'key' contains whitespace"
        };
}

}