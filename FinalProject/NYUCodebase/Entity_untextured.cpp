//
//  Entity_untextured.cpp
//  NYUCodebase
//
//  Created by Bixing Xie on 4/29/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity_untextured.h"

float easeIn(float from, float to, float time) {
    float tVal = time*time*time*time*time;
    return (1.0f-tVal)*from + tVal*to;
}


float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin)/(srcMax-srcMin) * (dstMax-dstMin));
    if(retVal < dstMin) {
        retVal = dstMin;
    }
    if(retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}

Entity_untextured::Entity_untextured(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};

void Entity_untextured::Draw(ShaderProgram* program){
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    
    modelMatrix.Translate(position.x, position.y, position.z);
    modelMatrix.Scale(size.x, size.y, size.z);
    Matrix viewMatrix;
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    program->SetColor(0, 0, 0, 0.3f);
    glUseProgram(program->programID);
    
    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);

    modelMatrix.Identity();
}

float animationTime = 0.0f;

void Entity_untextured::update(float elapsed){
//    animationTime = animationTime + elapsed;
//    float animationValue = mapValue(elapsed, 1.5f,
//                                    3.0f, 0.0, 1.0);
//
//    std::cout << easeIn(0.0, 1.0, animationValue) << std::endl;
//    modelMatrix.Translate(easeIn(0.0, 1.0, animationValue), 0.0, 0.0);
}

    

