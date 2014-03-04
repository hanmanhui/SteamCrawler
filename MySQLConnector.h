#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include <iostream>
#include <string>

#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

class MySQLConnector {
    public : 
        MySQLConnector() {
            this->ip = "127.0.0.1";
            this->port = 3306;
            
            this->id = "root";
            this->pwd = "root";
            
            this->db = "steamcrawler";
        }
        
        MySQLConnector(string ip, int port, string id, string pwd, string db) {
            this->ip = ip;
            this->port = port;
            
            this->id = id;
            this->pwd = pwd;
            
            this->db = db;
        }
        
        ~MySQLConnector() {
            delete driver;
            delete con;
        }
        
        bool connect() {
            try {
                string iport = ip + ":" + port;
                /* Create a connection */
                driver = get_driver_instance();
                con = driver->connect(iport.c_str(), id.c_str(), pwd.c_str());
                
                con->setSchema(db.c_str());
            } catch (sql::SQLException &e) {
                cout << "# ERR: SQLException in " << __FILE__;
                cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
                cout << " # ERR : " << e.what();
                cout << " (MySQL Error code: " << e.getErrorCode();
                cout << ", SQLState: " << e.getSQLState() << " )" << endl;
                
                return false;
            }
            
            return true;
        };
    
        sql::Connection *con;
    private :   
        sql::Driver *driver;
        
        string ip;
        int port;
        
        string id;
        string pwd;
        
        string db;
};

#endif /* MYSQLCONNECTOR_H */