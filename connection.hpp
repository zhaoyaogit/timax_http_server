#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

namespace timax
{
	class server_t;
	class response_t;
	class connection : public std::enable_shared_from_this<connection>
	{
	public:

		connection(server_t* server, boost::asio::io_service& ios) : server_(server), socket_(ios), read_buf_(MAX_LEN)
		{
		}

		~connection()
		{
			close();
		}

		void start()
		{
			read_head();
		}

		boost::asio::ip::tcp::socket& socket()
		{
			return socket_;
		}

		void write(const std::shared_ptr<response_t>& response, const std::vector<boost::asio::const_buffer>& buffers, bool need_close = true);

	private:
		void read_head();

		void read_body(const std::shared_ptr<connection>& self, bool need_close, request_t request, size_t body_len);

		bool need_close_conneciton(const request_t& request)
		{
			if (request.minor_version() == 0 && !request.has_keepalive_attr()) //short conneciton
			{
				return true;
			}

			if (request.minor_version() == 1 && request.has_close_attr())
			{
				return true;
			}

			return false;
		}

		void shutdown_send(boost::asio::ip::tcp::socket& s)
		{
			boost::system::error_code ignored_ec;
			s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ignored_ec);
		}

		void close()
		{
			if (!socket_.is_open())
				return;

			boost::system::error_code ignored_ec;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket_.close(ignored_ec);
		}

		void response(size_t status_code, bool need_close, const std::shared_ptr<connection>& self, request_t& request);

	private:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf read_buf_;
		const int MAX_LEN = 8192;
		server_t* server_;
	};
}
