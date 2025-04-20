#include "connection_pool.h"

#include <json/json.h>
#include <fstream>

ConnectionPool* ConnectionPool::get_connection_pool() {
    static ConnectionPool connection_pool;

    return &connection_pool;
}

bool ConnectionPool::parse_json_file() {
    std::ifstream infile("dbconf.json");
    Json::Reader reader;
    Json::Value root;

    reader.parse(infile, root);
    if (root.isObject()) {
        ip_ = root["ip"].asString();
        port_ = root["port"].asInt();
        user_ = root["username"].asString();
        password_ = root["password"].asString();
        db_name_ = root["db_name"].asString();
        min_size_= root["min_size"].asInt();
        max_size_ = root["max_size"].asInt();
        max_idle_time_ = root["max_idle_time"].asInt();
        timeout_ = root["timeout"].asInt();
        return true;
    }

    return false;
}
