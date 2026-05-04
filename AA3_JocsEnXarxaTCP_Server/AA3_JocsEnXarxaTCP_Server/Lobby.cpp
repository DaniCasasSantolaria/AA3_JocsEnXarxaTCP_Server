#include "Lobby.h"
#include <iostream>

void Lobby::AddClient(Client* client)
{
	try
	{
		if (IsFull()) return;
		clients.push_back(client);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void Lobby::RemoveClient(Client* client) {
	for (int i = 0; i < (int)clients.size(); i++)
	{
		if (clients[i]->GetId() == client->GetId())
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
}
