#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include "Client.h"
#include "DataBase.h"
#include "LobbyManager.h"

#define PM PacketManager::Instance()

#define LISTENER_PORT 55007

enum packetType { HANDSHAKE, LOGIN, REGISTER, RANKING, CREATE_LOBBY, JOIN_LOBBY, GAME_RESULT };

class PacketManager {
private:
    const sf::IpAddress SERVER_IP = sf::IpAddress(192, 168, 1, 39);

    sf::TcpSocket socket;

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