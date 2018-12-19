#include <iostream>
#include <asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
std::atomic<int> global_counts = 0;
class Client
{
private:
	asio::io_service& m_io;
	asio::ip::tcp::endpoint endpoint;
	std::shared_ptr<asio::ip::tcp::socket> socket_ptr;
	std::vector<char> buff;
public:
	Client(asio::io_service& io, const std::string& ip, short port) :m_io(io), endpoint(asio::ip::address::from_string(ip), port)
	{
		buff.resize(1024);
		socket_ptr = std::make_shared<asio::ip::tcp::socket>(m_io);
		init();
	}

	void init()
	{
		socket_ptr->async_connect(endpoint, [this](std::error_code const& ec) {
			std::cout << "opened=="<< global_counts << std::endl;
			this->read_some(global_counts);
			global_counts++;
		});
	}

	void read_some(int id)
	{
		socket_ptr->async_read_some(asio::buffer(buff), [this,id](std::error_code const& ec,std::size_t size) {
			if (ec) {
				std::cout << "error" << std::endl;
				socket_ptr->close();
				return;
			}
			if (size != 0) {
				std::cout << id << ":    " << std::string(buff.data(), size) << std::endl;
				//global_counts++;
			}
			this->read_some(id);
		});
	}
};
int main()
{
	asio::io_service io;
	std::vector<std::shared_ptr<Client>> vec;
	for (int i = 0; i < 800; i++) {
		auto client = std::make_shared<Client>(io, std::string("192.168.0.13"), 9090);
		vec.push_back(client);
	}
	io.run();
	std::cin.get();
	//vec.clear();
	return 0;
}
