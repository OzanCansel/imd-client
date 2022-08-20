#include "../connection.hpp"
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>

namespace imd
{

namespace
{
    template<class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

}

connection::pointer connection::create(
    std::string ip ,
    int port ,
    io_context& context
)
{
    return pointer {
        new connection
        {
            move( ip ) ,
            port ,
            context
        }
    };
}

connection::connection( std::string ip , int port , io_context& context )
    :   m_req_id     { 1 }
    ,   m_ip         { move( ip ) }
    ,   m_port       { std::uint16_t( port ) }
    ,   m_socket     { context }
    ,   m_sck_strand { context }
    ,   m_connected  { false }
    ,   m_timer      { context , boost::posix_time::milliseconds( 1000 ) }
{}

void connection::start()
{
    m_socket.async_connect(
        boost::asio::ip::tcp::endpoint
        {
            boost::asio::ip::address::from_string( m_ip ) ,
            m_port
        } ,
        [ self = shared_from_this() ]( const boost::system::error_code& err ){
            self->handle_connect( err );
        }
    );
}

connection::socket& connection::sck()
{
    return m_socket;
}

std::future<std::string> connection::get( std::string key )
{
    auto promise = std::make_shared<std::promise<std::string>>();
    std::future<std::string>  future { promise->get_future() };

    send(
        (
            boost::format( "get %1% %2%\n" ) % m_req_id % key
        ).str() ,
        m_req_id ,
        move( promise )
    );

    ++m_req_id;

    return future;
}

std::future<void> connection::put( std::string key , std::string value )
{
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void> future { promise->get_future() };

    send(
        (
            boost::format( "put %1% %2% %3%\n" ) % m_req_id % key % value
        ).str() ,
        m_req_id ,
        move( promise )
    );

    ++m_req_id;

    return future;
}

std::future<void> connection::remove( std::string key )
{
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void>  future { promise->get_future() };

    send(
        (
            boost::format( "remove %1% %2%\n" ) % m_req_id % key
        ).str() ,
        m_req_id ,
        move( promise )
    );

    ++m_req_id;

    return future;
}

void connection::handle_connect( const boost::system::error_code& err )
{
    if ( err )
    {
        start();

        return;
    }

    m_connected = true;

    read_line();
}

void connection::read_line()
{
    boost::asio::async_read_until(
        m_socket ,
        m_buffer ,
        '\n' ,
        [ self = shared_from_this() ]( boost::system::error_code err , int ){           
            self->handle_read_line( err );
        }
    );
}

void connection::handle_read_line( const boost::system::error_code& err )
{
    if ( err )
        return;

    std::istream is { &m_buffer };

    std::string line;
    std::getline( is , line );

    m_sck_strand.post(
        [ self = shared_from_this() , line = move( line ) ]() mutable {
            self->parse_and_handle_promise( move( line ) );
        }
    );

    read_line();
}

void connection::parse_and_handle_promise( std::string line )
{
    std::stringstream ss { line };

    std::string req_id_str , status_str;

    ss >> req_id_str >> status_str;

    std::uint64_t req_id  { std::stoull( req_id_str ) };
    bool          success { status_str == "success" };

    auto promise_v_it = m_promises.find( req_id );

    std::visit(
        overloaded {
            [ &ss , success ]( shared_promise<std::string> get_result )
            {
                if ( success )
                {
                    std::string value;

                    ss >> value;

                    get_result->set_value( move( value ) );
                }
                else
                {
                    std::string err_msg;

                    ss >> err_msg;

                    get_result->set_exception(
                        std::make_exception_ptr(
                            exception { err_msg }
                        )
                    );
                }
            } ,
            [ &ss , success ]( shared_promise<void> remove_put_result )
            {
                if ( success )
                {
                    remove_put_result->set_value();
                }
                else
                {
                    std::string err_msg;

                    ss >> err_msg;

                    remove_put_result->set_exception(
                        std::make_exception_ptr(
                            exception { err_msg }
                        )
                    );
                }
            }
        } ,
        promise_v_it->second
    );

    m_promises.erase( promise_v_it );
}

void connection::send( std::string text , int req_id , promise_variant promise )
{
    m_sck_strand.post(
        [
            self    = shared_from_this() ,
            text    = move( text ) ,
            req_id  = req_id ,
            promise = std::move( promise )
        ]() mutable {
            self->queue_packet(
                move( text ) ,
                req_id ,
                move( promise )
            );
        }
    );
}

void connection::queue_packet( std::string text , int req_id , promise_variant promise )
{
    bool write_in_progress { !m_send_packet_queue.empty() };

    m_send_packet_queue.push_back( move( text ) );

    m_promises.insert( { req_id , move( promise ) } );

    if ( !write_in_progress )
        start_packet_send();
}

void connection::start_packet_send()
{
    boost::asio::async_write(
        m_socket ,
        boost::asio::buffer( m_send_packet_queue.front() ) ,
        m_sck_strand.wrap(
            [
                self = shared_from_this()
            ]( const boost::system::error_code& err , std::size_t ) mutable {
                self->packet_send_done( err );
            }
        )
    );
}

void connection::packet_send_done( const boost::system::error_code& err )
{
    if ( err )
    {
        if ( !m_connected )
        {
            bool expected { false };

            if ( m_timer_running.compare_exchange_strong( expected , true ) )
                retry_send();
        }

        return;
    }

    m_send_packet_queue.pop_front();

    if ( !m_send_packet_queue.empty() )
        start_packet_send();
}

void connection::retry_send()
{
    m_connected = true;
    m_timer.expires_from_now( boost::posix_time::milliseconds( 1000 ) );

    m_timer.async_wait(
        [ self = shared_from_this() ]( const boost::system::error_code& err ){
            self->handle_retry( err );
        }
    );
}

void connection::handle_retry( const boost::system::error_code& err )
{
    if ( err )
    {
        m_timer_running = false;

        return;
    }
    
    m_sck_strand.post(
        [ self = shared_from_this() ](){
            self->start_packet_send();
        }
    );

    m_timer_running = false;
}

}