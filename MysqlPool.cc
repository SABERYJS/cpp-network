#include"MysqlPool.h"

MysqlPool::MysqlPool(MysqlPoolConfig config) :m_config(config) {
    //Make sure that you free con, the MysqlPool::MysqlConnection object, as soon as you do not need it any more.
    // But do not explicitly free driver, the connector object. Connector/C++ takes care of freeing that.
    driver = get_driver_instance();
    init_pool();
}

MysqlPool::~MysqlPool() {
    std::cout << "MysqlPool called" << std::endl;
}

void MysqlPool::init_pool() {
    std::unique_lock<std::mutex>lock{op_mtx};
    if (initialized) {
        return;
    }
    for (int i = 0;i < m_config.init_conn;i++) {
        std::shared_ptr<MysqlPool::MysqlConnection> con = create_conn();
        if (!con) {
            //connect failed
            throw std::runtime_error("connect database failed");
        }
        else {
            connections.push_back(con);
        }
        conn_id++;
    }
    initialized = true;
}

std::shared_ptr<MysqlPool::MysqlConnection>MysqlPool::GetConn() {
    data_mtx.lock();
    if (connections.size() > 0) {
        std::shared_ptr<MysqlPool::MysqlConnection>conn = get_conn_no_mutex();
        data_mtx.unlock();
        return conn;
    }
    else {
        data_mtx.unlock();
        std::unique_lock<std::mutex>oplock(op_mtx);
        std::shared_ptr<MysqlPool::MysqlConnection>conn = nullptr;
        cond.wait(oplock, [this, &conn]() {
            std::unique_lock<std::mutex>dlock{data_mtx};
            if (this->destroyed) {
                return true;
            }
            if (this->connections.size() > 0) {
                conn = get_conn_no_mutex();
                return true;
            }
            return false;
            });
        if (destroyed) {
            throw std::runtime_error("pool has been destroyed");
        }
        return conn;
    }
}

void MysqlPool::ReleaseConn(std::shared_ptr<MysqlPool::MysqlConnection>conn) {
    std::unique_lock<std::mutex>dlock{data_mtx};
    connections.push_back(conn);
    cond.notify_one();
}

void MysqlPool::Destroy() {
    destroyed = true;
    cond.notify_all();
    //do not release MysqlPool::MysqlConnection manual
    //smarter pointer will release it
    connections.clear();
}

int MysqlPool::Size() {
    return cur_conn;
}

std::shared_ptr<MysqlPool::MysqlConnection> MysqlPool::create_conn() {
    std::shared_ptr<MysqlPool::MysqlConnection> con(new MysqlPool::MysqlConnection(driver->connect(fmt::format("tcp://{}:{}", m_config.server_addr, m_config.server_port), m_config.username, m_config.password), conn_id));
    if (con->Get()->isValid()) {
        /* Connect to the MySQL test database */
        con->Get()->setSchema(m_config.schema);
        return con;
    }
    else {
        return nullptr;
    }
}

std::shared_ptr<MysqlPool::MysqlConnection>MysqlPool::get_conn_mutex() {
    data_mtx.lock();
    std::shared_ptr<MysqlPool::MysqlConnection>conn = get_conn_no_mutex();
    data_mtx.unlock();
    return conn;
}

std::shared_ptr<MysqlPool::MysqlConnection>MysqlPool::get_conn_no_mutex() {
    std::shared_ptr<MysqlPool::MysqlConnection>con = connections.front();
    connections.pop_front();
    if (!con->Get()->isValid() || con->Get()->isClosed()) {
        if (!con->Get()->reconnect()) {
            throw std::runtime_error("reconnect mysql failed");
        }
    }
    return con;
}