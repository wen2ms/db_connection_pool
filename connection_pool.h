#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

#include "mysql_connection.h"

class ConnectionPool {
  public:
    ConnectionPool(const ConnectionPool& other) = delete;
    ConnectionPool& operator=(const ConnectionPool& other) = delete;

    static ConnectionPool* get_connection_pool();

  private:
    ConnectionPool() = default;

    bool parse_json_file();

    std::string ip_;
    std::string user_;
    std::string password_;
    std::string db_name_;
    unsigned short port_;
    int min_size_;
    int max_size_;
    int timeout_;
    int max_idle_time_;
    std::queue<MysqlConnection*> connection_queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

#endif  // CONNECTION_POOL_H