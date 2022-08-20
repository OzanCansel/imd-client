#pragma once

#include <string>
#include <future>
#include <vector>
#include <boost/thread.hpp>
#include "connection.hpp"
#include "executor.hpp"

namespace imd
{

class client
{

public:

    inline client(
        std::string ip_addr ,
        int port ,
        int n_threads     = boost::thread::hardware_concurrency() ,
        int n_connections = boost::thread::hardware_concurrency()
    );

    inline std::future<std::string> get( const std::string& key );
    inline std::future<void>        put( const std::string& key , const std::string& value );
    inline std::future<void>        remove( const std::string& key );

private:

    connection::pointer& next();
    inline void validate_key( const std::string& ) const;

    using connections = std::vector<connection::pointer>;

    std::string m_ip_addr;
    int         m_port;
    connections m_connections;
    int         m_conn_idx;
    executor    m_executor;
};

}

#include "impl/client_impl.hpp"