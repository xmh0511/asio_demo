#include <iostream>
#include <thread>
#include <asio.hpp>
#include <vector>
#include <memory>
#include <sstream>
class connection
{
public:
	asio::ip::tcp::socket socket;
	std::vector<char> buff;
	connection(asio::io_service& io):socket(io)
	{
		buff.resize(1024);
	}
	void read_something()
	{
		socket.async_read_some(asio::buffer(buff), [this](std::error_code const& ec, std::size_t size) {
			std::cout << "thread id=" << std::this_thread::get_id() << std::endl;
			this->read_something();
		});
	}
};
class Server
{
private:
	std::vector<asio::io_service> tcp_io_pool;
	std::vector<std::thread> thread_pool;
	std::vector<std::shared_ptr<asio::ip::tcp::socket>> sockets;
	std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> acceptors;
	asio::ip::tcp::endpoint endpoint;
	std::vector<std::shared_ptr<asio::io_service::work>> works;
	int get_io_service_index = 0;
public:
	Server(std::size_t size,short port):tcp_io_pool(size), works(size), endpoint(asio::ip::tcp::v4(), port)
	{
		init();
	}

	asio::io_service& get_io_service()
	{
		if (get_io_service_index == tcp_io_pool.size()) {
			get_io_service_index = 0;
		}
		//std::cout << get_io_service_index << std::endl;
		return tcp_io_pool[get_io_service_index++];
	}

	void init()
	{
		for (auto&iter : tcp_io_pool) {
			works.push_back(std::make_shared<asio::io_service::work>(iter));
		}
		auto& io = get_io_service();
		auto acceptor = std::make_shared<asio::ip::tcp::acceptor>(io, endpoint);
		start_acceptor(acceptor);
	}

	void read_something(std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		auto buff = std::make_shared<std::vector<char>>(1024);
		socket->async_read_some(asio::buffer(*buff), [this, buff, socket](std::error_code const& ec, std::size_t size) {
			//std::cout << "thread id=" << std::this_thread::get_id() << std::endl;
			//this->read_something(socket);
		});
	}

	void start_acceptor(std::shared_ptr<asio::ip::tcp::acceptor> acceptor)
	{
		auto socket = std::make_shared<asio::ip::tcp::socket>(get_io_service());
		sockets.push_back(socket);
		acceptor->async_accept((*socket), [this,acceptor, socket](std::error_code const& ec) {
			this->write_something(socket);
			start_acceptor(acceptor);
		});
	}

	void write_something(std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		socket->async_write_some(asio::buffer("hello"), [this, socket](std::error_code const& ec,std::size_t size) {
			std::cout << "connect" << std::endl;
			//std::cout << "thread id=" << std::this_thread::get_id() << std::endl;
			//this->read_something(socket);
			this->read_something(socket);
		});
	}

	void run()
	{
		for (auto&iter : tcp_io_pool) {
			thread_pool.emplace_back(std::thread([](asio::io_service* io_ptr) {
				std::cout << "run" << std::endl;
				io_ptr->run();
			},&iter));
		}
		for (auto&iter : thread_pool) {
			iter.join();
			std::cout << "join" << std::endl;
		}
	}
};
int main()
{
	Server seve(8, 8080);
	seve.run();
	std::cin.get();
	return 0;
}
