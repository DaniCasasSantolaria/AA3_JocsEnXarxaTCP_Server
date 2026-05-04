#pragma once
#include <string>
#include <SFML/Network.hpp>
class Client {
private:
	int id;
	std::string username;
	sf::TcpSocket* socket;
	int port = 0;

public:
	Client() : id(0), username("") {
		socket = new sf::TcpSocket();
	}
	Client(int id, std::string username) : id(id), username(username) {
		socket = new sf::TcpSocket();
	}
	const int GetId() const { return id; }
	const std::string GetUsername() const { return username; }
	sf::TcpSocket& GetSocket() { return *socket; }
	const std::string GetAddress() const { return socket->getRemoteAddress().value().toString(); }
	void SetUsername(std::string newUsername) { username = newUsername; }
	void SetId(int newId) { id = newId; }
	int GetPort() const { return port; }
	void SetPort(int p) { port = p; }
};