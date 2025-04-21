#include "connection_pool.h"

#include <json/json.h>

#include <fstream>
#include <thread>

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
        min_size_ = root["min_size"].asInt();
        max_size_ = root["max_size"].asInt();
        max_idle_time_ = root["max_idle_time"].asInt();
        timeout_ = root["timeout"].asInt();
        return true;
    }

    return false;
}

ConnectionPool::ConnectionPool() {
    if (!parse_json_file()) {
        return;
    }

    for (int i = 0; i < min_size_; ++i) {
        MysqlConnection* connection = new MysqlConnection;
        connection->connect(user_, password_, db_name_, ip_, port_);
        connection_queue_.push(connection);
    }

    std::thread producer(&ConnectionPool::produce_connection, this);
    std::thread recycler(&ConnectionPool::recycle_connection, this);

    producer.detach();
    recycler.detach();
}

void ConnectionPool::produce_connection() {}

void ConnectionPool::recycle_connection() {}
