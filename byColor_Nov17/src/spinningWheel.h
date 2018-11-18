//
//  spinningWheel.hpp
//  taiwanColorFramework
//
//  Created by KJ Wu on 11/6/18.
//
//

#ifndef _spinningWheel
#define _spinningWheel

#include "ofMain.h"
class SpinningWheel {
    
public: // place public functions or variables declarations here
    
    // methods, equivalent to specific functions of your class objects
    void setup(float _or=150, float _ir=100, float _sd =22.5);	// setup method, use this to setup your object's initial state
    void update();  // update method, used to refresh your objects properties
    void draw(float _x, float _y);    // draw method, this where you'll do the object's drawing
    void resetTimes();
    
    // variables
    ofColor colorWheel[16];
    int sections;
    float outterR;
    float innerR;
    float rotateSpeed;
    float angle;
    ofPoint center;
    int times;
    
    int mode;
    int index;
    
    SpinningWheel();  // constructor - used to initialize an object, if no properties are passed the program sets them to the default value
private: // place private functions or variables declarations here
}; // don't forget the semicolon!



#endif /* spinningWheel.h */
