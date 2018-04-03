//
//  Vector3.cpp
//  NYUCodebase
//
//  Created by Bixing Xie on 4/2/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include <stdio.h>
#include "Vector3.h"
#include <cmath>

Vector3::Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
float Vector3::length(){ return sqrt(x*x+y*y+z*z);}
    
void Vector3::normalize(){
    float len = sqrt(x*x+y*y+z*z);
        
    if(len!=0.0f){
        x = x/len;
        y = y/len;
        z = z/len;
    }
};

    

