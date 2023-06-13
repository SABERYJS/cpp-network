
#include<memory>
#include<functional>
#include<iostream>

using namespace std;
int main(int argc, char const* argv[]) {
    int a = 1;
    int b = 3;
    int c = a ^ b;
    cout << c << endl;
    cout << (c ^ b) << endl;
    cout << (c ^ a) << endl;
    return 0;
}
