#include <iostream>
#include <memory>
#include <thread>

#include "connection_pool.h"
#include "mysql_connection.h"

void op1(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        MysqlConnection connection;
        connection.connect("root", "<password>", "test", "localhost");
        char sql[1024] = {0};
        snprintf(sql, sizeof(sql), "INSERT INTO person(id, age, gender, name) VALUES (%d, 99, 'M', 'Tom');", i);
        connection.update(sql);
    }
}

void op2(ConnectionPool* connection_pool, int begin, int end) {
    for (int i = begin; i < end; ++i) {
        std::shared_ptr<MysqlConnection> connection_ptr = connection_pool->get_connection();
        char sql[1024] = {0};
        snprintf(sql, sizeof(sql), "INSERT INTO person(id, age, gender, name) VALUES (%d, 99, 'M', 'Tom');", i);
        connection_ptr->update(sql);
    }
}

void non_pool_single_thread() {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op1(0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = end - begin;
    std::cout << "Non-connection pool, single thread, consuming: " << duration.count() << " nanoseconds, "
              << duration.count() / 1000000 << " milliseconds" << std::endl;
}

void pool_single_thread() {
    ConnectionPool* pool = ConnectionPool::get_connection_pool();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    op2(pool, 0, 5000);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = end - begin;
    std::cout << "Connection pool, single thread, consuming: " << duration.count() << " nanoseconds, "
              << duration.count() / 1000000 << " milliseconds" << std::endl;
}

void non_pool_multithread() {
    MysqlConnection connection;
    connection.connect("root", "<password>", "test", "localhost");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::thread t1(op1, 0, 1000);
    std::thread t2(op1, 1000, 2000);
    std::thread t3(op1, 2000, 3000);
    std::thread t4(op1, 3000, 4000);
    std::thread t5(op1, 4000, 5000);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = end - begin;
    std::cout << "Non-connection pool, multithread, consuming: " << duration.count() << " nanoseconds, "
              << duration.count() / 1000000 << " milliseconds" << std::endl;
}

void pool_multithread() {
    ConnectionPool* pool = ConnectionPool::get_connection_pool();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    std::thread t1(op2, pool, 0, 1000);
    std::thread t2(op2, pool, 1000, 2000);
    std::thread t3(op2, pool, 2000, 3000);
    std::thread t4(op2, pool, 3000, 4000);
    std::thread t5(op2, pool, 4000, 5000);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto duration = end - begin;
    std::cout << "Connection pool, multithread, consuming: " << duration.count() << " nanoseconds, "
              << duration.count() / 1000000 << " milliseconds" << std::endl;
}

bool query() {
    MysqlConnection connection;
    connection.connect("root", "<password>", "test", "localhost");

    std::string sql = "INSERT INTO person(id, age, gender, name) VALUES (1, 99, 'M', 'Tom');";
    if (!connection.update(sql)) {
        std::cout << "Update failed..." << std::endl;
        return false;
    }

    sql = "SELECT id, age, gender, name FROM person;";
    connection.query(sql);
    while (connection.next()) {
        std::cout << connection.value(0) << ", " << connection.value(1) << ", " << connection.value(2) << ", "
                  << connection.value(3) << std::endl;
    }

    return true;
}

int main() {
    // Non-connection pool, single thread, consuming: 3634614397 nanoseconds, 3634 milliseconds
    // non_pool_single_thread();

    // Connection pool, single thread, consuming: 2098097560 nanoseconds, 2098 milliseconds
    // pool_single_thread();

    // Non-connection pool, multithread, consuming: 1612285354 nanoseconds, 1612 milliseconds
    non_pool_multithread();

    // Connection pool, multithread, consuming: 863227036 nanoseconds, 863 milliseconds
    // pool_multithread();

    return 0;
}