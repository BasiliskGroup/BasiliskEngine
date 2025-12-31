#include "print.h"
#include <iostream>
using namespace std;

void print(float3 vec){
    for (int i = 0; i < 3; i++) {
        cout << vec[i] << " ";
    }
    cout << endl;
}

void print(float3x3 mat){
    for (int i = 0; i < 3; i++) {
        print(mat[i]);
    }
}

void print(std::string s) {
    cout << s << endl;
}

void print(char* s) {
    cout << s << endl;
}

void print(float f) {
    cout << f << endl;
}