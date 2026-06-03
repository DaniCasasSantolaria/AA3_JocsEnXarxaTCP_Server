#include "PackageManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

sf::Packet& operator <<(sf::Packet& packet, packetType type) {
	return packet << static_cast<short>(type);
}

sf::Packet& operator <<(sf::Packet& packet, authResult result) {
	return packet << static_cast<short>(result);
}

sf::Packet& operator <<(sf::Packet& packet, matchMode mode) {
	return packet << static_cast<short>(mode);
}

sf::Packet& operator <<(sf::Packet& packet, matchmakeStatus status) {
	return packet << static_cast<short>(status);
}

sf::Packet& operator >>(sf::Packet& packet, packetType& type) {
	short temp;
	packet >> temp;
	type = static_cast<packetType>(temp);
	return packet;
}

sf::Packet& operator >>(sf::Packet& packet, authResult& result) {
	short temp;
	packet >> temp;
	result = static_cast<authResult>(temp);
	return packet;
}

sf::Packet& operator >>(sf::Packet& packet, matchMode& mode) {
	short temp;
	packet >> temp;
	mode = static_cast<matchMode>(temp);
	return packet;
}

sf::Packet& operator >>(sf::Packet& packet, matchmakeStatus& status) {
	short  temp;
	packet >> temp;
	status = static_cast<matchmakeStatus>(temp);
	return packet;
}

// Procesa los paquetes recibidos del cliente
void PacketManager::HandlePacket(Client& client, sf::Packet& packet, DataBase& db, std::unordered_map<std::string, std::vector<std::vector<std::string>>>& gameResults) {
	packetType type;
	packet >> type;

	bool sendResponse = true;
	sf::Packet response;

	switch (type) {
	case HANDSHAKE:
	{
		std::string message = "Handshake done with client";
		response << HANDSHAKE << message;
		break;
	}
	case SERVER_HANDSHAKE:
	{
		udpServerClient = &client;

		std::cout << "UDP Gameplay Server connected to TCP Server. Connection IP: " << client.GetAddress() << std::endl;

		sendResponse = false;
		break;
	}
	case LOGIN:
	{
		std::string userName;
		std::string password;

		packet >> userName >> password;

		authResult result = db.Login(userName, password);

		if (result == LOGIN_OK) {
			client.SetUsername(userName);
		}

		int score = db.GetScore(userName);

		response << LOGIN << userName << password << result << score << client.GetId();

		std::cout << "Score: " << score << std::endl;
		break;
	}
	case REGISTER:
	{
		std::string userName;
		std::string password;

		packet >> userName >> password;

		authResult result = db.CreateUser(userName, password);

		response << REGISTER << userName << password << result;
		break;
	}
	case RANKING:
	{
		std::string clientUsername;

		packet >> clientUsername;

		std::vector<PlayerScore> ranking = db.GetRanking(clientUsername);

		response << RANKING;

		for (unsigned short i = 0; i < static_cast<unsigned short>(ranking.size()); i++) {
			response << ranking[i].name << ranking[i].score << ranking[i].position;
		}

		break;
	}
	case MATCHMAKE:
	{
		short modeInt;
		packet >> modeInt;

		matchMode mode = static_cast<matchMode>(modeInt);

		MatchmakingPlayer player;
		player.client = &client;
		player.mode = mode;
		player.score = db.GetScore(client.GetUsername());

		matchmakeStatus status = QUEUE_WAITING;

		MatchmakingPlayer p1;
		MatchmakingPlayer p2;

		if (mode == COMPETITIVE) {
			competitiveQueue.push_back(player);

			std::cout << "Player added to COMPETITIVE queue: "
				<< client.GetUsername()
				<< " score: " << player.score
				<< std::endl;

			for (unsigned short i = 0; i < competitiveQueue.size(); i++) {
				for (unsigned short j = i + 1; j < competitiveQueue.size(); j++) {
					short difference = competitiveQueue[i].score - competitiveQueue[j].score;

					if (std::abs(difference) <= MAX_SCORE_DIFFERENCE) {
						status = MATCH_FOUND;

						p1 = competitiveQueue[i];
						p2 = competitiveQueue[j];

						competitiveQueue.erase(competitiveQueue.begin() + j);
						competitiveQueue.erase(competitiveQueue.begin() + i);

						break;
					}
				}

				if (status == MATCH_FOUND) {
					break;
				}
			}
		}
		else if (mode == NON_COMPETITIVE) {
			nonCompetitiveQueue.push(player);

			std::cout << "Player added to NON_COMPETITIVE queue: " << client.GetUsername() << std::endl;

			if (nonCompetitiveQueue.size() >= 2) {
				status = MATCH_FOUND;

				p1 = nonCompetitiveQueue.front();
				nonCompetitiveQueue.pop();

				p2 = nonCompetitiveQueue.front();
				nonCompetitiveQueue.pop();
			}
		}
		else {
			std::cout << "Invalid matchmaking mode received from client" << std::endl;
			break;
		}

		response << MATCHMAKE << mode << status;

		if (status == MATCH_FOUND) {
			unsigned short matchId = nextMatchId++;

			std::cout << "Starting " << (mode == COMPETITIVE ? "COMPETITIVE" : "NON_COMPETITIVE")
				<< " match between " << p1.client->GetUsername() << " and "	<< p2.client->GetUsername()	<< std::endl;

			if (udpServerClient != nullptr) {
				sf::Packet udpServerPacket;

				udpServerPacket << MATCH_CREATED << matchId << mode
					<< p1.client->GetId() << p1.client->GetUsername()
					<< p2.client->GetId()<< p2.client->GetUsername();

				SendData(udpServerClient->GetSocket(), udpServerPacket);

				std::cout << "TCP Server notified UDP Server. MatchId: "
					<< matchId << " | P1: " << p1.client->GetUsername()	<< p1.client->GetAddress() << ":" << p1.client->GetPort()
					<< " | P2: " << p2.client->GetUsername() << p2.client->GetAddress() << ":" << p2.client->GetPort() << std::endl;
			}
			else {
				std::cout << "UDP Server is not connected. Match created but not notified." << std::endl;
			}

			sendResponse = false;

			sf::Packet response2 = response;

			SendData(p1.client->GetSocket(), response);
			SendData(p2.client->GetSocket(), response2);
		}
		break;
	}
	case GAME_RESULT:
	{
		unsigned short winnerId = 0;
		std::string winnerUsername = "";

		packet >> winnerId >> winnerUsername;

		const short WIN_SCORE_REWARD = 100;

		int oldScore = db.GetScore(winnerUsername);

		db.UpdateScore(winnerUsername, WIN_SCORE_REWARD);

		int newScore = db.GetScore(winnerUsername);

		std::cout << "Score updated for: " << winnerUsername << std::endl << " | Old score: " << oldScore << " | New score: " << newScore << std::endl;

		sendResponse = false;
		break;
	}
	case MAP_REQUEST:
	{
		short requestTypeValue;
		packet >> requestTypeValue;

		mapRequestType requestType = static_cast<mapRequestType>(requestTypeValue);

		if (requestType == MAP_VERSION_CHECK) {
			unsigned short clientMapVersion;
			packet >> clientMapVersion;

			if (clientMapVersion == SERVER_MAP_VERSION) {
				response << MAP_REQUEST << static_cast<short>(MAP_UP_TO_DATE) << SERVER_MAP_VERSION;
			}
			else {
				//La parte de leer ficheros hecha con IA y adaptada a lo que necesitavamos
				std::ifstream file("resources/Maps/Map.txt");
				std::stringstream buffer;

				if (file.is_open()) {
					buffer << file.rdbuf();
				}
				else {
					std::cerr << "No se pudo abrir el mapa del servidor" << std::endl;
				}

				std::string mapContent = buffer.str();

				response << MAP_REQUEST	<< static_cast<short>(MAP_UPDATE) << SERVER_MAP_VERSION	<< mapContent;
			}

			sendResponse = true;
		}
		break;
	}
	default:
		std::cout << "Packet type does not exist" << std::endl;
		break;
	}

	if (sendResponse) {
		SendData(client.GetSocket(), response);
	}
}

void PacketManager::DisconnectClient(Client* client, sf::SocketSelector& selector) {
	selector.remove(client->GetSocket());
	client->GetSocket().disconnect();
}

void PacketManager::SendData(sf::TcpSocket& client, sf::Packet& packet) {
	if (client.send(packet) == sf::Socket::Status::Done) {
		std::cout << "Mensaje enviado" << std::endl;
		packet.clear();
	}
	else {
		std::cerr << "Error al enviar el mensaje al cliente" << std::endl;
	}
}