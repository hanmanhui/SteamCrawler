#include "MySQLConnector.h"

MySQLConnector::MySQLConnector() {
	this->ip = "127.0.0.1";
	this->port = 3306;
            
	this->id = "root";
	this->pwd = "root";
            
	this->db = "steamcrawler";
}

MySQLConnector::MySQLConnector(string ip, int port, string id, string pwd, string db) {
	this->ip = ip;
	this->port = port;

	this->id = id;
	this->pwd = pwd;
       
	this->db = db;
}
      
bool MySQLConnector::connect() {
	try {
		stringstream ss;
		ss << ip;
		ss << ":";
		ss << port;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(ss.str().c_str(), id.c_str(), pwd.c_str());

		con->setSchema(db.c_str());
	} catch (sql::SQLException &e) {
	    stringstream ss;
		ss << "# ERR: SQLException in " << __FILE__;
		ss << "(" << __FUNCTION__ << ") on line " << __LINE__ << '\n';
		ss << " # ERR : " << e.what();
		ss << " (MySQL Error code: " << e.getErrorCode();
		ss << ", SQLState: " << e.getSQLState() << " )\n";

        printf("%s", ss.str().c_str());
        return false;
	}
            
	return true;
}
