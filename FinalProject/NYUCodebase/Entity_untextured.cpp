//
//  Entity_untextured.cpp
//  NYUCodebase
//
//  Created by Bixing Xie on 4/29/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity_untextured.h"



Entity_untextured::Entity_untextured(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};

void Entity_untextured::Draw(ShaderProgram* program){
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix modelMatrix;
    
    modelMatrix.Translate(position.x, position.y, position.z);
    modelMatrix.Scale(size.x, size.y, size.z);
    Matrix viewMatrix;
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    program->SetViewMatrix(viewMatrix);
    glUseProgram(program->programID);
    program->SetColor(1.0f, 1.0f, 1.0f, 0.4f);
    
    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
}

    

