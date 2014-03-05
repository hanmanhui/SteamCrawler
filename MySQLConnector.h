#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include <iostream>
#include <string>
#include <sstream>

#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class MySQLConnector {
    public : 
        MySQLConnector();
        MySQLConnector(string ip, int port, string id, string pwd, string db);

        bool connect();
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
