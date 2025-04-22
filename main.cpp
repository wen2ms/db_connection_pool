#include <iostream>

#include "mysql_connection.h"

bool query() {
    MysqlConnection connection;
    connection.connect("root", "<password>", "test", "localhost");

    std::string sql = "INSERT INTO user(id, name, age, status, gender) VALUES (7, 'test_tom', 99, '0', '男');";
    if (!connection.update(sql)) {
        std::cout << "Update failed..." << std::endl;
        return false;
    }

    sql = "SELECT id, name, age, status, gender FROM user;";
    connection.query(sql);
    while (connection.next()) {
        std::cout << connection.value(0) << ", " << connection.value(1) << ", " << connection.value(2) << ", " 
                  << connection.value(3) << ", " << connection.value(4) << std::endl;
    }

    return true;
}

int main() {
    query();

    return 0;
}