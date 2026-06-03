#include "DataBase.h"

void DataBase::ConnectDataBase(sql::Driver*& driver) {
	try {
		driver = get_driver_instance();
		con = driver->connect(SERVER, USERNAME, PASSWORD);
		con->setSchema(DATABASE);
		std::cout << "Connected to database successfully!" << std::endl;
	}
	catch (sql::SQLException e) {
		std::cerr << "Error connecting to database: " << e.what() << std::endl;
	}
}

void DataBase::DisconnectDataBase() {
	con->close();

	if (con->isClosed()) {
		std::cout << "Connection Close" << std::endl;
		con = nullptr;
		driver = nullptr;
		delete con;
	}
}

authResult DataBase::CreateUser(std::string user, std::string password) {
	try {
		sql::Statement* stmt = con->createStatement();
		std::string queryGetUsernames = "SELECT * FROM Users WHERE username = '" + user + "'";

		sql::ResultSet* res = stmt->executeQuery(queryGetUsernames);

		if (res->next()) {
			std::cout << "User already exists" << std::endl;
			delete res;
			delete stmt;
			return authResult::USER_ALREADY_EXISTS;
		}
		else {

			// La parte de enviar los datos mediante un statement, arreglado con IA debido a que el hash contiene
			// caracteres especiales y daba errores al intentar insertar el usuario con un statement normal
			std::string hash = bcrypt::generateHash(password);

			const unsigned short INITIAL_SCORE = 100;

			std::unique_ptr<sql::PreparedStatement> stmt(
				con->prepareStatement("INSERT INTO Users (username, pass, score) VALUES (?, ?, ?)")
			);

			stmt->setString(1, user);
			stmt->setString(2, hash);
			stmt->setInt(3, INITIAL_SCORE);

			int affected_rows = stmt->executeUpdate();

			if (affected_rows > 0) {
				std::cout << "User created successfully" << std::endl;
				return authResult::REGISTER_OK;
			}
		}
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "Error while creating user: " << e.what() << std::endl;
	}
}

authResult DataBase::Login(std::string user, std::string password) {
	try
	{
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("SELECT * FROM Users WHERE username = ?")
		);

		stmt->setString(1, user);
		sql::ResultSet* res = stmt->executeQuery();

		if (res->next()) {
			std::cout << "User finded" << std::endl;
			std::string storedHash = res->getString("pass");

			bool valid = bcrypt::validatePassword(password, storedHash);

			delete res;

			if (valid)
			{
				std::cout << "Password is correct" << std::endl;
				return authResult::LOGIN_OK;
			}
			else
			{
				std::cout << "Wrong password" << std::endl;
				return authResult::WRONG_PASSWORD;
			}
		}
		else
		{
			std::cout << "User not found" << std::endl;
			delete res;
			return authResult::USER_NOT_FOUND;
		}
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "Error don't find user: " << e.what() << std::endl;
	}
}

std::vector<PlayerScore> DataBase::GetRanking(std::string clientUsername) {
	std::vector<PlayerScore> ranking = std::vector<PlayerScore>();

	try {
		sql::Statement* stmt = con->createStatement();
		unsigned const short RANKING_LIMIT = 10;
		sql::ResultSet* res = stmt->executeQuery("SELECT username, score FROM Users ORDER BY score DESC LIMIT " + std::to_string(RANKING_LIMIT));

		while (res->next()) {
			PlayerScore playerScore;
			playerScore.name = res->getString("username");
			playerScore.score = res->getInt("score");
			playerScore.position = static_cast<unsigned short>(ranking.size() + 1);
			ranking.push_back(playerScore);
		}

		delete stmt;
		delete res;


		bool isClientInRanking = false;
		for (unsigned short i = 0; i < static_cast<short>(ranking.size()) || isClientInRanking; i++) {
			if (ranking[i].name == clientUsername) {
				isClientInRanking = true;
				break;
			}
		}

		if (!isClientInRanking) {
			sql::PreparedStatement* pstmt = con->prepareStatement(
				"SELECT u.username, u.score, "
				"1 + (SELECT COUNT(*) FROM Users WHERE score > u.score) AS ranking "
				"FROM Users u "
				"WHERE u.username = ?"
			);

			pstmt->setString(1, clientUsername);
			sql::ResultSet* res2 = pstmt->executeQuery();


			if (res2->next()) {
				PlayerScore clientScore;
				clientScore.name = res2->getString("username");
				clientScore.score = res2->getInt("score");
				clientScore.position = static_cast<unsigned short>(res2->getInt("ranking"));
				ranking.push_back(clientScore);
			}			

			delete pstmt;
			delete res2;
		}


	}
	catch (sql::SQLException& e) {
		std::cerr << "Error while fetching users: " << e.what() << std::endl;
	}
	return ranking;
}

int DataBase::GetScore(std::string username) {
	try {
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("SELECT score FROM Users WHERE username = '" + username + "'"));
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
		if (res->next()) {
			int score = res->getInt("score");
			return score;
		}
		else {
			return 0;
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Error while fetching score: " << e.what() << std::endl;
		return 0;
	}
}

void DataBase::UpdateScore(std::string username, short points) {
	try {
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("UPDATE Users SET score = score + ? WHERE username = ?")
		);
		stmt->setInt(1, points);
		stmt->setString(2, username);
		stmt->executeUpdate();
		std::cout << "Score updated for " << username << ": +" << points << std::endl;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Error updating score: " << e.what() << std::endl;
	}
}