#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include "Client.h"
#include "DataBase.h"
#include "LobbyManager.h"
#include <queue>

#define PM PacketManager::Instance()

#define LISTENER_PORT 55007

enum packetType { HANDSHAKE, LOGIN, REGISTER, RANKING, MATCHMAKE, WIN_NOTIFICATION, GAME_RESULT, MAP_REQUEST };

enum matchMode { NON_COMPETITIVE, COMPETITIVE };

enum matchmakeStatus { QUEUE_WAITING, MATCH_FOUND};

enum mapRequestType { MAP_VERSION_CHECK, MAP_UP_TO_DATE, MAP_UPDATE };

struct MatchmakingPlayer {
    Client* client;
    matchMode mode;
};

class PacketManager {
private:
    const unsigned short SERVER_MAP_VERSION = 1;							// ----------------------------------- MAP VERSION -----------------------------------

    //Colas de matchmaking para cada modo
    std::queue<MatchmakingPlayer> nonCompetitiveQueue;
    std::queue<MatchmakingPlayer> competitiveQueue;

    PacketManager() = default;
    PacketManager(PacketManager&) = delete;
    PacketManager& operator =(const PacketManager&) = delete;
    ~PacketManager() = default;

public:
    inline static PacketManager* Instance() {
        static PacketManager instance;
        return &instance;
    }

    void PairToPair() {};

    void HandlePacket(Client& client, sf::Packet& packet, DataBase& db, LobbyManager& lobbyManager, std::unordered_map<std::string, std::vector<std::vector<std::string>>>& gameResults);

    void DisconnectClient(Client* client, LobbyManager& lobbyManager, sf::SocketSelector& selector);

    void SendData(sf::TcpSocket& client, sf::Packet& packet);
};