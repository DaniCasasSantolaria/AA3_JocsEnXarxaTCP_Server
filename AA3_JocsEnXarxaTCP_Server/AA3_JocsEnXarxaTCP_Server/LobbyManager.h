#pragma once
#include <unordered_map>
#include <string>
#include "Lobby.h"

enum lobbyResult { LOBBY_CREATED_OK, LOBBY_ALREADY_EXISTS, LOBBY_CREATE_ERROR, LOBBY_JOINED_OK, LOBBY_NOT_FOUND, LOBBY_FULL };

class LobbyManager {
private:
    std::unordered_map<std::string, Lobby*> lobbies;

public:
    LobbyManager() = default;
    ~LobbyManager() = default;

    const lobbyResult CreateLobby(const std::string idLobby, Client* client);

    Lobby* GetLobby(const std::string idLobby);

    void AddClientToLobby(const std::string idLobby, Client* client);

    void RemoveClientFromLobby(const std::string idLobby, Client* client);

    void RemoveClientFromAllLobbies(Client* client);

    void CloseLobby(const std::string idLobby);
};