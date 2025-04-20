#include "connection_pool.h"

ConnectionPool* ConnectionPool::get_connection_pool() {
    static ConnectionPool connection_pool;

    return &connection_pool;
}