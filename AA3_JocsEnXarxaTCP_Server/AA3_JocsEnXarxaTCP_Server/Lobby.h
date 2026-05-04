#pragma once
#include <string>
#include <vector>
#include "Client.h"

class Lobby
{
private:
	std::string idLobby;
	std::vector<Client*> clients;
	int maxClients = 4;

public:
	Lobby() = default;
	Lobby(std::string idLobby) : idLobby(idLobby) {}

	void AddClient(Client* client);
	void RemoveClient(Client* client);
	std::vector<Client*> GetClients() const { return clients; }
	std::string GetIdLobby() const { return idLobby; }
	bool IsFull() const { return (int)clients.size() >= maxClients; }
	Client* GetHost() const { return clients.empty() ? nullptr : clients[0]; }
	bool AllPortsReported() const {
		if (clients.empty()) return false;
		for (Client* c : clients) {
			if (c->GetPort() == 0) return false;
		}
		return true;
	}
};