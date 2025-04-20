#ifndef MYSQL_CONNECTION_H
#define MYSQL_CONNECTION_H

#include <iostream>
#include <mysql.h>

class MysqlConnection {
  public:
    MysqlConnection();
    ~MysqlConnection();

    bool connect(std::string user, std::string password, std::string db_name, std::string ip, unsigned short port = 3306);
    bool update(std::string sql);
    bool query(std::string sql);

    bool next();
    std::string value(int index);

    bool transaction();
    bool commit();
    bool rollback();

  private:
    void free_result();

    MYSQL* connection_;
    MYSQL_RES* result_;
    MYSQL_ROW row_;
};

#endif  // MYSQL_CONNECTION_H