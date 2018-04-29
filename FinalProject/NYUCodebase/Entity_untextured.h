//
//  Entity_untextured.h
//  NYUCodebase
//
//  Created by Bixing Xie on 4/29/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Entity_untextured_h
#define Entity_untextured_h
#include "ShaderProgram.h"
#include "Vector3.h"

class Entity_untextured{
public:
    
    Entity_untextured(float positionX, float positionY, float sizeX, float sizeY);
    
    void Draw(ShaderProgram* program);
    
    Vector3 position;
    Vector3 size;
};


#endif /* Entity_untextured_h */
