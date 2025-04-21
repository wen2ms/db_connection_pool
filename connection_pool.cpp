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
        add_connection();
    }

    std::thread producer(&ConnectionPool::produce_connection, this);
    std::thread recycler(&ConnectionPool::recycle_connection, this);

    producer.detach();
    recycler.detach();
}

void ConnectionPool::produce_connection() {
    while (true) {
        std::unique_lock<std::mutex> locker(mutex_);
        while (connection_queue_.size() >= min_size_) {
            condition_.wait(locker);
        }

        add_connection();
    }
}

void ConnectionPool::recycle_connection() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while (connection_queue_.size() > min_size_) {
            MysqlConnection* connection = connection_queue_.front();
            if (connection->get_alive_time() >= max_idle_time_) {
                connection_queue_.pop();
                delete connection;
            } else {
                break;
            }
        }
    }
}

void ConnectionPool::add_connection() {
    MysqlConnection* connection = new MysqlConnection;
    connection->connect(user_, password_, db_name_, ip_, port_);
    connection->refresh_alive_time();
    connection_queue_.push(connection);
}
