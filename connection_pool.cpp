#include "connection_pool.h"

#include <json/json.h>

#include <fstream>
#include <thread>

ConnectionPool::~ConnectionPool() {
    while (!connection_queue_.empty()) {
        MysqlConnection* connection = connection_queue_.front();
        connection_queue_.pop();
        delete connection;
    }
}

ConnectionPool* ConnectionPool::get_connection_pool() {
    static ConnectionPool connection_pool;

    return &connection_pool;
}

std::shared_ptr<MysqlConnection> ConnectionPool::get_connection() {
    std::unique_lock<std::mutex> locker(mutex_);
    while (connection_queue_.empty()) {
        if (std::cv_status::timeout == condition_.wait_for(locker, std::chrono::milliseconds(timeout_))) {
            if (connection_queue_.empty()) {
                continue;
            }
        }
    }

    std::shared_ptr<MysqlConnection> connection_ptr(connection_queue_.front(), [this](MysqlConnection* connection) {
        std::lock_guard<std::mutex> locker(mutex_);
        connection->refresh_alive_time();
        connection_queue_.push(connection);
    });
    connection_queue_.pop();
    condition_.notify_all();

    return connection_ptr;
}

bool ConnectionPool::parse_json_file() {
    std::ifstream infile("<absolute_file_path>");
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
        condition_.notify_all();
    }
}

void ConnectionPool::recycle_connection() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::lock_guard<std::mutex> locker(mutex_);
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
