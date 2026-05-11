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

sf::Packet& operator <<(sf::Packet& packet, lobbyResult result) {
	return packet << static_cast<short>(result);
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

sf::Packet& operator >>(sf::Packet& packet, lobbyResult& result) {
	short temp;
	packet >> temp;
	result = static_cast<lobbyResult>(temp);
	return packet;
}

// Procesa los paquetes recibidos del cliente
void PacketManager::HandlePacket(Client& client, sf::Packet& packet, DataBase& db, LobbyManager& lobbyManager, std::unordered_map<std::string, std::vector<std::vector<std::string>>>& gameResults) {
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

		response << LOGIN << userName << password << result << score;

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

		if (mode == COMPETITIVE) {
			competitiveQueue.push(player);

			std::cout << "Player added to COMPETITIVE queue: " << client.GetUsername() << std::endl;

			if (competitiveQueue.size() >= 2) {
				MatchmakingPlayer p1 = competitiveQueue.front();
				competitiveQueue.pop();

				MatchmakingPlayer p2 = competitiveQueue.front();
				competitiveQueue.pop();

				std::cout << "Starting COMPETITIVE match between "
					<< p1.client->GetUsername()
					<< " and "
					<< p2.client->GetUsername()
					<< std::endl;
			}
		}
		else if (mode == NON_COMPETITIVE) {
			nonCompetitiveQueue.push(player);

			std::cout << "Player added to NON_COMPETITIVE queue: " << client.GetUsername() << std::endl;

			if (nonCompetitiveQueue.size() >= 2) {
				MatchmakingPlayer p1 = nonCompetitiveQueue.front();
				nonCompetitiveQueue.pop();

				MatchmakingPlayer p2 = nonCompetitiveQueue.front();
				nonCompetitiveQueue.pop();

				std::cout << "Starting NON_COMPETITIVE match between "
					<< p1.client->GetUsername()
					<< " and "
					<< p2.client->GetUsername()
					<< std::endl;
			}
		}
		else {
			std::cout << "Invalid matchmaking mode received from client" << std::endl;
		}

		response << MATCHMAKE;
		break;
	}
	case GAME_RESULT:
	{
		std::string lobbyId;
		unsigned short numPlayers = 0;

		packet >> lobbyId >> numPlayers;

		std::vector<std::string> ranking;

		for (unsigned short i = 0; i < numPlayers; i++) {
			std::string username;
			packet >> username;
			ranking.push_back(username);
		}

		std::cout << "GAME_RESULT received: lobby=" << lobbyId
			<< " numPlayers=" << numPlayers << std::endl;

		for (unsigned short i = 0; i < static_cast<unsigned short>(ranking.size()); i++) {
			std::cout << "  [" << i << "] " << ranking[i] << std::endl;
		}

		gameResults[lobbyId].push_back(ranking);

		std::cout << "  submissions so far: "
			<< gameResults[lobbyId].size()
			<< "/"
			<< numPlayers
			<< std::endl;

		if (static_cast<unsigned short>(gameResults[lobbyId].size()) == numPlayers) {
			bool valid = true;

			for (unsigned short i = 1; i < numPlayers; i++) {
				if (gameResults[lobbyId][i] != gameResults[lobbyId][0]) {
					valid = false;
					std::cout << "  MISMATCH at submission " << i << std::endl;
					break;
				}
			}

			if (valid) {
				std::cout << "Game result validated for lobby " << lobbyId << std::endl;
				const short WINNER_POINTS = 3;
				for (unsigned short i = 0; i < numPlayers; i++) {
					short points = static_cast<short>(WINNER_POINTS - static_cast<short>(i));

					std::cout << "  Awarding " << points
						<< " pts to "
						<< gameResults[lobbyId][0][i]
						<< std::endl;

					db.UpdateScore(gameResults[lobbyId][0][i], points);
				}
			}
			else {
				std::cout << "Game result mismatch for lobby "
					<< lobbyId
					<< ", no points awarded"
					<< std::endl;
			}

			gameResults.erase(lobbyId);
		}

		sendResponse = false;
		break;
	}
	case MAP_REQUEST:
	{
		response << MAP_DATA;

		std::vector<std::pair<std::string, std::string>> maps;

		//HECHO CON IA PARA LEER LOS MAPAS DE LA CARPETA resources/Maps, por si hay mas de un mapa y para no tener que hardcodearlos
		for (const auto& entry : std::filesystem::recursive_directory_iterator("resources/Maps")) {
			if (!entry.is_regular_file()) {
				continue;
			}

			if (entry.path().extension() != ".txt") {
				continue;
			}

			std::ifstream file(entry.path());
			std::stringstream buffer;

			if (!file.is_open()) {
				std::cerr << "No se pudo abrir el mapa: " << entry.path() << std::endl;
				continue;
			}

			buffer << file.rdbuf();

			std::string relativePath = std::filesystem::relative(
				entry.path(),
				"resources/Maps"
			).generic_string();

			std::string mapContent = buffer.str();

			maps.push_back({ relativePath, mapContent });
		}

		response << static_cast<unsigned short>(maps.size());

		for (const auto& map : maps) {
			response << map.first << map.second;
		}

		sendResponse = true;
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

void PacketManager::DisconnectClient(Client* client, LobbyManager& lobbyManager, sf::SocketSelector& selector) {
	lobbyManager.RemoveClientFromAllLobbies(client);
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