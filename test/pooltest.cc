#include"MysqlPool.h"
#include<iostream>
#include<thread>

using namespace std;

struct st {
    int a;
};


int main(int argc, char const* argv[]) {

    MysqlPool pool(MysqlPool::MysqlPoolConfig("192.168.1.31", 3306, "debug", "root", "6ec8lNiDoO078ZeZ", 10, 5, 5, 5));
    try {
        for (int i = 0;i < 10;i++) {
            thread th([&pool]() {
                for (int i = 0;i < 10000;i++) {
                    try {
                        std::shared_ptr<MysqlPool::MysqlConnection>cnn = pool.GetConn();
                        cnn->executeQuery([](sql::ResultSet* res) {
                            while (res->next()) {
                                cout << "\t... MySQL replies: ";
                                /* Access column data by alias or column name */
                                cout << res->getInt64("uid") << endl;
                                cout << "\t... MySQL says it again: ";
                                /* Access column data by numeric offset, 1 is the first column */
                                cout << res->getString("password") << endl;
                            }
                            }, "select * from accounts where id={}", 1);
                        pool.ReleaseConn(cnn);
                    }
                    catch (const std::exception& e) {
                        std::cerr << e.what() << '\n';
                    }
                }

                });
            th.detach();
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    std::this_thread::sleep_for(std::chrono::seconds(100));

    return 0;
}
