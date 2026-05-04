#pragma once
#include <string>
#include <vector>
#include "Client.h"

class Lobby
{
private:
	std::string idLobby;
	std::vector<Client*> clients;
	int maxClients = 2;

public:
	Lobby() = default;
	Lobby(std::string idLobby) : idLobby(idLobby) {}

	void AddClient(Client* client);
	void RemoveClient(Client* client);
	std::vector<Client*> GetClients() const { return clients; }
	std::string GetIdLobby() const { return idLobby; }
	bool IsFull() const { return (int)clients.size() >= maxClients; }
};