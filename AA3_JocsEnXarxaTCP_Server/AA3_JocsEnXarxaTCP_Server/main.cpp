#include <iostream>
#include <unordered_map>
#include <stdlib.h>
#include "PackageManager.h"

int main() {
	srand(time(NULL));

	DataBase db;

	bool closeServer = false;

	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::unordered_map<std::string, Client*> clients;

	LobbyManager lobbyManager;

	std::unordered_map<std::string, std::vector<std::vector<std::string>>> gameResults;

	if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done) {
		std::cerr << "FAILED listening to port " << LISTENER_PORT << std::endl;
		closeServer = true;
	}

	selector.add(listener);

	int nextClientId = 1;

	while (!closeServer) {
		std::cout << "Waiting for connection..." << std::endl;
		if (selector.wait()) {
			if (selector.isReady(listener)) {
				std::unique_ptr<Client> newClient = std::make_unique<Client>();		//Buscat amb IA perque no es repeteixin els clients

				if (listener.accept(newClient->GetSocket()) == sf::Socket::Status::Done) {
					newClient->SetId(nextClientId);
					nextClientId++;
					newClient->GetSocket().setBlocking(false);

					selector.add(newClient->GetSocket());

					std::string mapKey = std::to_string(newClient->GetId());

					clients.emplace(mapKey, newClient.release()); //Buscat amb IA perque no es repeteixin els clients
				}
				else {
					std::cout << "Failed accepting client connection" << std::endl;
				}
			}

			for (std::unordered_map<std::string, Client*>::iterator it = clients.begin(); it != clients.end();) {
				Client* client = it->second;

				if (selector.isReady(client->GetSocket())) {
					sf::Packet packet;
					sf::Socket::Status status = client->GetSocket().receive(packet);

					if (status == sf::Socket::Status::Done) {
						PM->HandlePacket(*client, packet, db, lobbyManager, gameResults);
						it++;
					}
					else if (status == sf::Socket::Status::Disconnected) {
						PM->DisconnectClient(client, lobbyManager, selector);
						it = clients.erase(it);
					}
					else {
						it++;
					}
				}
				else {
					it++;
				}
			}
		}
	}
}