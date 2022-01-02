#include <iostream>
#include <wiringPi.h>

using namespace std;

int main(){
    wiringPiSetup();
    pinMode(11, INPUT);

    while(true){
        if(digitalRead(11) == 1){
            cout << "bumper!" << endl;
        }
    }
    return 0;
}
