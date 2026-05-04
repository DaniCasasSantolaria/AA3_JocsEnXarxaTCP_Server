#include "LobbyManager.h"

const lobbyResult LobbyManager::CreateLobby(const std::string idLobby, Client* client) {
	if (lobbies.find(idLobby) != lobbies.end()) {
		return LOBBY_ALREADY_EXISTS;
	}

	Lobby* newLobby = new Lobby(idLobby);
	newLobby->AddClient(client);
	lobbies[idLobby] = newLobby;
	return LOBBY_CREATED_OK;
}

Lobby* LobbyManager::GetLobby(const std::string idLobby) {
	std::unordered_map<std::string, Lobby*>::iterator it = lobbies.find(idLobby);
	if (it != lobbies.end()) {
		return it->second;
	}
	return nullptr;
}

void LobbyManager::AddClientToLobby(const std::string idLobby, Client* client) {
	std::unordered_map<std::string, Lobby*>::iterator it = lobbies.find(idLobby);
	if (it != lobbies.end()) {
		it->second->AddClient(client);
	}
}

void LobbyManager::RemoveClientFromLobby(const std::string idLobby, Client* client) {
	std::unordered_map<std::string, Lobby*>::iterator it = lobbies.find(idLobby);
	if (it != lobbies.end()) {
		it->second->RemoveClient(client);
	}
}

void LobbyManager::RemoveClientFromAllLobbies(Client* client) {
	std::vector<std::string> emptyLobbies;

	for (std::unordered_map<std::string, Lobby*>::iterator it = lobbies.begin(); it != lobbies.end(); it++) {
		Lobby* lobby = it->second;
		lobby->RemoveClient(client);

		if (lobby->GetClients().empty()) {
			emptyLobbies.push_back(it->first);
		}
	}

	for (std::string& id : emptyLobbies) {
		CloseLobby(id);
	}
}

void LobbyManager::CloseLobby(const std::string idLobby) {
	std::unordered_map<std::string, Lobby*>::iterator it = lobbies.find(idLobby);
	if (it != lobbies.end()) {
		delete it->second;
		lobbies.erase(it);
	}
}