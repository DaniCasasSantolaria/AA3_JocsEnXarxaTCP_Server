#include "Lobby.h"
#include <iostream>

void Lobby::AddClient(Client* client) {
	try {
		if (IsFull()) return;

		clients.push_back(client);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void Lobby::RemoveClient(Client* client) {
	for (unsigned short i = 0; i < static_cast<unsigned short>(clients.size()); i++) {
		if (clients[i]->GetId() == client->GetId()) {
			clients.erase(clients.begin() + i);
			break;
		}
	}
}