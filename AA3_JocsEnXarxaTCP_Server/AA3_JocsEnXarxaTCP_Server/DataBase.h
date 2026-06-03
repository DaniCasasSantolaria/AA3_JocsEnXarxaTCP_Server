#pragma once
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "Bcrypt/bcrypt.h"
#include <vector>

#define SERVER   "212.227.144.246:3306"
#define USERNAME "manager"
#define PASSWORD "Hola12345!"
#define DATABASE "aa1_bbdd"

enum authResult { LOGIN_OK, USER_NOT_FOUND, WRONG_PASSWORD, REGISTER_OK, USER_ALREADY_EXISTS };

struct PlayerScore {
	std::string name = "";
	int score = 0;
	unsigned short position = 0;
};

class DataBase {
private:
	sql::Driver* driver;
	sql::Connection* con;
public:
	DataBase() {
		ConnectDataBase(driver);
	}
	~DataBase() {
		DisconnectDataBase();
	}

	void ConnectDataBase(sql::Driver*& driver);

	void DisconnectDataBase();

	authResult CreateUser(std::string user, std::string password);

	authResult Login(std::string user, std::string password);

	std::vector<PlayerScore> GetRanking(std::string clientUsername);

	int GetScore(std::string username);

	void UpdateScore(std::string username, short points);
};