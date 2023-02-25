#include <iostream>
using namespace std;

int main() {

    int a = 23;
    int* pa = &a;
    char* pb = reinterpret_cast<char*> (pa);
    cout << "*pb: " << (*pb) << "\n"; // prints shit like 'ㅓ'
    int* pc = reinterpret_cast<int*> (pb);
    //cout << (*pc); // prints 23
    char aa = a;
    cout<<"aa: "<<aa;
}
