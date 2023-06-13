#ifndef MT_MYSQL_POOL_H
#define MT_MYSQL_POOL_H

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include<mutex>
#include<list>
#include<condition_variable>
#include<memory>
#include<fmt/format.h>
#include<stdexcept>
#include<thread>
#include<functional>


using namespace std;
class MysqlPool {
public:
    class MysqlPoolConfig {
    public:
        std::string server_addr;
        short server_port;
        std::string schema;
        std::string  username;
        std::string  password;
        int max_conn;
        int max_idle_conn;
        int min_conn;
        int init_conn;
        MysqlPoolConfig(std::string saddr, short sport, std::string schema, std::string uname, std::string upasswd, int mcn, int mic, int mc, int ic) :
            server_addr(saddr),
            server_port(sport),
            schema(schema),
            username(uname),
            password(upasswd),
            max_conn(mcn),
            max_idle_conn(mic),
            min_conn(mc),
            init_conn(ic) {}
    };

    class MysqlConnection
    {
    private:
        sql::Connection* sql_conn{ nullptr };
        int debug_id{ 0 };
    public:
        MysqlConnection() = delete;
        MysqlConnection(sql::Connection* conn, int did) :sql_conn(conn) {}
        ~MysqlConnection() {
            delete sql_conn;
        }
        sql::Connection* Get() {
            return sql_conn;
        }

        template<class ... Args>
        bool execute(const char* sqlm, Args&& ... args) {
            sql::SQLString str = fmt::format(sqlm, std::forward<Args>(args)...);
            sql::Statement* stmt = sql_conn->createStatement();
            bool ret = stmt->execute(str);
            delete stmt;
            return ret;
        }
        template<class ... Args>
        void  executeQuery(std::function<void(sql::ResultSet* rs)>cb, const char* sqlm, Args&& ... args) {
            sql::SQLString str = fmt::format(sqlm, std::forward<Args>(args)...);
            sql::Statement* stmt = sql_conn->createStatement();
            sql::ResultSet* rs = stmt->executeQuery(str);
            cb(rs);
            delete stmt;
            delete rs;
            return;
        }
    };

private:
    MysqlPoolConfig m_config;
    std::condition_variable cond;
    std::list<std::shared_ptr<MysqlPool::MysqlConnection>>connections;
    std::atomic_int  cur_conn;
    bool initialized{ false };
    sql::Driver* driver;
    std::atomic_bool destroyed{ false };
    std::mutex data_mtx;
    std::mutex op_mtx;
    int conn_id{ 0 };
public:
    MysqlPool(MysqlPoolConfig config);
    ~MysqlPool();
    std::shared_ptr<MysqlPool::MysqlConnection>GetConn();
    void ReleaseConn(std::shared_ptr<MysqlPool::MysqlConnection>conn);
    void Destroy();
    int Size();
private:
    std::shared_ptr<MysqlPool::MysqlConnection> create_conn();
    void init_pool();
    std::shared_ptr<MysqlPool::MysqlConnection>get_conn_mutex();
    std::shared_ptr<MysqlPool::MysqlConnection>get_conn_no_mutex();
};

#endif