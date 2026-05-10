#pragma once
#include <string>
#include <SFML/Network.hpp>

class Client {
private:
	unsigned short id;
	std::string username;
	sf::TcpSocket* socket;
	unsigned short port = 0;

public:
	Client() : id(0), username("") {
		socket = new sf::TcpSocket();
	}
	Client(unsigned short id, std::string username) : id(id), username(username) {
		socket = new sf::TcpSocket();
	}
	const unsigned short GetId() const { return id; }
	const std::string GetUsername() const { return username; }
	sf::TcpSocket& GetSocket() { return *socket; }
	const std::string GetAddress() const { return socket->getRemoteAddress().value().toString(); }
	void SetUsername(std::string newUsername) { username = newUsername; }
	void SetId(unsigned short newId) { id = newId; }
	unsigned short GetPort() const { return port; }
	void SetPort(unsigned short p) { port = p; }
};