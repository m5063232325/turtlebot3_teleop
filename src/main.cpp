// kbhit
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <wiringPi.h>

#define AUTO_MODE 0
#define MANUAL_MODE 1
#define BURGER_MAX_LIN_VEL 0.22
#define BURGER_MAX_ANG_VEL 2.84
#define LIN_VEL_STEP 0.01
#define ANG_VEL_STEP 0.1
using namespace std;

const char *welcome_messenge = 
    "Control Your Turtlebot3 vacuum robot!\n"
    "-------------------------------------\n"
    "Moving around: \n"
    "        w\n"
    "   a    s    d\n"
    "        x\n"
    "Press v to change auto/manual mode\n";

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

float makeSimpleProfile(float output, float input, float slop){
    if(input > output){
        output = min(input, output + slop);
    }
    else if (input < output){
        output = max(input, output - slop);
    }
    else{
        output = input;
    }
    return output;
}

float constrain(float input, float low, float high)
{
    if (input < low){
        input = low;
    }
    else if (input > high){
        input = high;
    }
    else
    {
        input = input;
    }
    
    return input;
}
float checkLinearLimitVelocity(float vel){
    return constrain(vel, -BURGER_MAX_LIN_VEL, BURGER_MAX_LIN_VEL);
}

float checkAngularLimitVelocity(float vel){
    return constrain(vel, -BURGER_MAX_ANG_VEL, BURGER_MAX_ANG_VEL);
}

void printVelocity(float lin_vel, float ang_vel){
    cout << "currently:\tlinear velocity " << lin_vel << "\t angular velocity " << ang_vel << endl;
}

int main(int argc, char** argv) {
    int mode = MANUAL_MODE;
    system("clear");
    cout << welcome_messenge << endl;
    wiringPiSetup();
    pinMode(0, INPUT);
    float target_linear_vel = 0.0;
    float target_angular_vel = 0.0;
    float control_linear_vel = 0.0;
    float control_angular_vel = 0.0;
    int counter = 0;
    ros::init(argc, argv, "vacuum_cleaner_control");
    ros::NodeHandle nh;

    ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1000);


    char key;
    while (true){
        if(kbhit()){
            key = getchar();
            if(mode == MANUAL_MODE){
                counter++;
                switch (key){
                    case 'w':
                        target_linear_vel = checkLinearLimitVelocity(target_linear_vel + LIN_VEL_STEP);
                        printVelocity(target_linear_vel, target_angular_vel);
                        break;
                    case 'x':
                        target_linear_vel = checkLinearLimitVelocity(target_linear_vel - LIN_VEL_STEP);
                        printVelocity(target_linear_vel, target_angular_vel);
                        break;
                    case 'a':
                        target_angular_vel = checkAngularLimitVelocity(target_angular_vel + ANG_VEL_STEP);
                        printVelocity(target_linear_vel, target_angular_vel);
                        break;
                    case 'd':
                        target_angular_vel = checkAngularLimitVelocity(target_angular_vel - ANG_VEL_STEP);
                        printVelocity(target_linear_vel, target_angular_vel);
                        break;
                    case 's':
                        target_linear_vel = 0.0;
                        target_angular_vel = 0.0;
                        control_linear_vel = 0.0;
                        control_linear_vel = 0.0;
                        printVelocity(target_linear_vel, target_angular_vel);
                        break;
                    case 'v':
                        mode = AUTO_MODE;
                        cout << "Switch to auto mode" << endl;
                        cout << "Press v to change to manual mode" << endl;
                        break;
                    case '\x03':
                        cout << "Bye!" << endl;
                        exit(0);
                    default:
                        break;
                }
            }
            else if (mode == AUTO_MODE){
                if(key == 'v'){
                    mode = MANUAL_MODE;
                    cout << "Switch to manual mode" << endl;
                    cout << "Press v to change to auto mode" << endl;
                    cout << welcome_messenge << endl;
                }
            }

            if(counter == 20){
                system("clear");
                cout << welcome_messenge << endl;
                printVelocity(target_linear_vel, target_angular_vel);
                counter = 0;
            }
        }
        else{
            if(mode == AUTO_MODE){
                /* turtlebot's code */
		// while(true){

		if(digitalRead(0) == 1){
		    // no bumper
		    target_linear_vel = 0.2;
		}
		else{
		    //bumper on
		    cout << "Bumper on!" << endl;
		}
            }
        }
        geometry_msgs::Twist msg;
        control_linear_vel = makeSimpleProfile(control_linear_vel, target_linear_vel, (LIN_VEL_STEP/2));
        msg.linear.x = control_linear_vel; msg.linear.y = 0.0; msg.linear.z = 0.0;
        control_angular_vel = makeSimpleProfile(control_angular_vel, target_angular_vel, (LIN_VEL_STEP/2));
        msg.angular.x = 0.0; msg.angular.y = 0.0; msg.angular.z = control_angular_vel;
        pub.publish(msg);

	// ros::spinOnce();
    }

    return 0;
}
