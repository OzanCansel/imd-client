#include "../executor.hpp"

namespace imd
{

executor::executor( int n_threads )
    :   m_keep_alive { m_context }
{
    for ( auto i = 0; i < n_threads; ++i )
        m_pool.create_thread(
            [ this ](){
                m_context.run();
            }
        );
}

executor::~executor()
{
    m_context.stop();

    m_pool.join_all();
}

executor::io_context& executor::context()
{
    return m_context;
}

}