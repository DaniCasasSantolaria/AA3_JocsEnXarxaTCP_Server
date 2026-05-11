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

enum packetType { HANDSHAKE, LOGIN, REGISTER, RANKING, MATCHMAKE, WIN_NOTIFICATION, GAME_RESULT, MAP_REQUEST, MAP_DATA };

enum matchMode { NON_COMPETITIVE, COMPETITIVE };

struct MatchmakingPlayer {
    Client* client;
    matchMode mode;
};

class PacketManager {
private:
    const sf::IpAddress SERVER_IP = sf::IpAddress(192, 168, 1, 39);

    sf::TcpSocket socket;

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