#include "SteamGameCrawler.h"

long SteamGameCrawler::calTime() {
	long seconds = this->end.tv_sec - this->start.tv_sec;
	long useconds = this->end.tv_usec - this->start.tv_usec;
	
	return ((seconds)*1000 + useconds/1000.0) + 0.5;
}

bool SteamGameCrawler::run() {
	printf("Started Running Game Crawler\n");
	
	gettimeofday(&start, NULL);
	// Connect To DB
	dbConn->connect();
	gettimeofday(&end, NULL);
	printf("DB Connected (time consumed : %ldms)\n", this->calTime());

	sql::Statement *stmt;
	sql::ResultSet *res;
	sql::PreparedStatement *pstmt;
	pstmt = dbConn->con->prepareStatement("INSERT INTO game(appid, title) VALUES (?, ?) ON DUPLICATE KEY UPDATE title=VALUES(title);");

	string page;
	int appid = -1;

    gettimeofday(&start, NULL);
	stmt = dbConn->con->createStatement();
	res = stmt->executeQuery("SELECT appid FROM game WHERE (release_date IS NULL OR metacritic_score IS NULL OR platforms IS NULL) ORDER BY RAND() LIMIT 1;");
	gettimeofday(&end, NULL);
	if(res->next()) {
		appid = res->getInt(1);
		printf("Getting Random appid from DB Done (time consumed : %ldms)\n", this->calTime());
	}
	delete stmt;
	delete res;

	while(appid != -1) {
		stringstream appDetailsUrl;
		appDetailsUrl << "http://store.steampowered.com/api/appdetails/?appids=";
		appDetailsUrl << appid;

		for(int cc_index = 0; cc_index < cc_size; cc_index++) {
			string urlWithCC = appDetailsUrl.str() + "&cc=" + cc[cc_index];
			gettimeofday(&start, NULL);
			page = curl->getPage(urlWithCC);
			gettimeofday(&end, NULL);
			printf("Getting Game Information Done (time consumed : %ldms)\n", this->calTime());

			gettimeofday(&start, NULL);
			if(page.find("\"success\":true") != string::npos) {
//				printf("Current URL [%s]\n", appDetailsUrl.str().c_str());
//				printf("[%s]\n", page.c_str());
				break;
			} else {
				printf("Not Found : %d\n", appid);
			}
			/*
			stringstream ss(page);
			string line;
			int gamesCount = -1;

			while(std::getline(ss, line, '\n')) {
				size_t begin = line.find("var rgGames = [");
				if(begin != string::npos) {
					size_t first = line.find("{");
					size_t last = line.find("];");

					if(first == string::npos) {
						gamesCount = 0;
						break;
					}

					string arrayJson = line.substr(first, last-first);
					arrayJson.erase(std::remove(arrayJson.begin(),
					arrayJson.end(), '\\'), arrayJson.end());
					
					size_t pos = arrayJson.find("},{");
					vector<string> jsonStrings;

					while(pos != string::npos) {
						jsonStrings.push_back(arrayJson.substr(0, pos+1));
						arrayJson = arrayJson.substr(pos+2);
						pos = arrayJson.find("},{");
					}
					jsonStrings.push_back(arrayJson);

					gamesCount = jsonStrings.size();

					for(int i = 0; i < gamesCount; i++) {
						rapidjson::Document doc;
						doc.Parse<0>(jsonStrings[i].c_str());

						int appID = doc.HasMember("appid") ? doc["appid"].GetInt()  : -1;
						string title = doc.HasMember("name") ?
						doc["name"].GetString() : "";
						float played = -1;
						if(doc.HasMember("hours_forever")) {
							string t = doc["hours_forever"].GetString();
							t.erase(std::remove(t.begin(), t.end(), ','),
							t.end());
							stringstream tSS;
							tSS << t;
							tSS >> played;
						}

						if(appID != -1 && title != "") {
							pstmt->setInt(1, appID);
							pstmt->setString(2, title);
							pstmt->execute();

							if(played != -1) {
								stmt = dbConn->con->createStatement();
								stringstream q;
								q <<  "INSERT IGNORE INTO user_game(user_id, game_id, played) VALUES (";
								q << "(SELECT id ";
								q << "FROM user ";
								q << "WHERE url='" + url + "'), ";
								q << appID;
								q << ", ";
								q << played;
								q << ");";
								stmt->execute(q.str().c_str());

								delete stmt;
							} else {
								stmt = dbConn->con->createStatement();
								stringstream q;
								q << "INSERT IGNORE INTO user_game(user_id, game_id) VALUES (";
								q << "(SELECT id ";
								q << "FROM user ";
								q << "WHERE url='" + url + "'), ";
								q << appID;
								q << ");";
								stmt->execute(q.str().c_str());

								delete stmt;
							}
						}
					}

					if(gamesCount != -1) {
						stmt = dbConn->con->createStatement();
						stringstream tSS;
						tSS << "INSERT INTO user(url, games) VALUES('";
						tSS << url;
						tSS << "', ";
						tSS << gamesCount;
						tSS << ") ON DUPLICATE KEY UPDATE ";
						tSS << "games=VALUES(games);";
						stmt->execute(tSS.str().c_str());
						delete stmt;
					}
					gettimeofday(&end, NULL);
					printf("Saving User's Games Information Done (time consumed : %ldms)\n", this->calTime());
					break;
				}
			}
			*/
		}

		// Getting Next appid 
		appid = -1;
		gettimeofday(&start, NULL);
		stmt = dbConn->con->createStatement();
		res = stmt->executeQuery("SELECT appid FROM game WHERE (release_date IS NULL OR metacritic_score IS NULL OR platforms IS NULL) ORDER BY RAND() LIMIT 1;");
		gettimeofday(&end, NULL);
		if(res->next()) {
			appid = res->getInt(1);
			printf("Getting Random appid from DB Done (time consumed : %ldms)\n", this->calTime());
		}
		delete stmt;
		delete res;
	}
	delete pstmt;

	return false;
}