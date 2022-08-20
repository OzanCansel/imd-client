#pragma once

#include <string>
#include <cstdint>
#include <future>
#include <deque>
#include <variant>
#include <atomic>
#include <unordered_map>
#include <stdexcept>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/strand.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace imd
{

struct exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class connection : public boost::enable_shared_from_this<connection>
{

public:

    using pointer    = boost::shared_ptr<connection>;
    using io_context = boost::asio::io_context;
    using socket     = boost::asio::ip::tcp::socket;
    using streambuf  = boost::asio::streambuf;
    using strand     = boost::asio::io_context::strand;
    using timer      = boost::asio::deadline_timer;

    static inline pointer create(
        std::string ip ,
        int port ,
        io_context&
    );

    inline void start();
    inline socket& sck();
    inline std::future<std::string> get( std::string key );
    inline std::future<void>        put( std::string key , std::string value );
    inline std::future<void>        remove( std::string key );

private:

    template<typename T>
    using shared_promise = std::shared_ptr<std::promise<T>>;
    using promise_variant  = std::variant<
                                 shared_promise<std::string> ,
                                 shared_promise<void>
                             >;

    inline connection( std::string ip , int port , io_context& );
    inline void handle_connect( const boost::system::error_code& );
    inline void read_line();
    inline void handle_read_line( const boost::system::error_code& );
    inline void parse_and_handle_promise( std::string line );
    inline void send( std::string , int req_id , promise_variant );
    inline void queue_packet( std::string , int req_id , promise_variant );
    inline void start_packet_send();
    inline void packet_send_done( const boost::system::error_code& );
    inline void retry_send();
    inline void handle_retry( const boost::system::error_code& );

    timer             m_timer;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_timer_running;
    std::uint64_t     m_req_id;
    std::string       m_ip;
    std::uint16_t     m_port;
    socket            m_socket;
    strand            m_sck_strand;
    streambuf         m_buffer;
    std::unordered_map<std::uint64_t , promise_variant> m_promises;
    std::deque<std::string> m_send_packet_queue;
};

}

#include "impl/connection_impl.hpp"