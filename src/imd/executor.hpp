#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/thread.hpp>

namespace imd
{

class executor
{

public:

    using io_context = boost::asio::io_context;

    inline explicit executor( int n_threads );
    inline ~executor();
    inline io_context& context();

private:

    using thread_group = boost::thread_group;
    using work         = boost::asio::io_context::work;

    io_context   m_context;
    thread_group m_pool;
    work         m_keep_alive;
};

}

#include "impl/executor_impl.hpp"