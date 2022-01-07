#include <iostream>
#include <wiringPi.h>
#include <stdlib.h>
using namespace std;

int main(){
    wiringPiSetup();
    pinMode(0, INPUT);

    while(true){
        if(digitalRead(0) == 1){
		cout << "no bumper!" << endl;
//		printf("bumper!");
        }
	else{
		exit(0);
	    }
	}
    return 0;
}
