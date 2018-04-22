//
//  Vector3.h
//  NYUCodebase
//
//  Created by Bixing Xie on 4/2/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Vector3_h
#define Vector3_h

class Vector3{
public:
    Vector3(float x, float y, float z);
    
    float length();
    
    void normalize();
    
    float x;
    float y;
    float z;
};

#endif /* Vector3_h */
