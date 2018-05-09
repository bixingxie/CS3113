//
//  Entity.cpp
//  NYUCodebase
//
//  Created by Bixing Xie on 5/7/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity.h"


float lerp(float v0, float v1, float t);


Entity::Entity(){};

Entity::Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType):sprite(sprite), position(positionX, positionY, 0.0f), size(sizeX*sprite.size*sprite.width/sprite.height, sizeY*sprite.size, 0.0f), velocity(velocityX, velocityY, 0.0f), acceleration(accelerationX, accelerationY, 0.0f), entityType(entityType){
    
    screamSound = Mix_LoadWAV("scream.wav");
    lolSound = Mix_LoadWAV("lol.wav");
    
};

void Entity::updateX(float elapsed, GameMode& mode, Entity& player){
    collideRight = false;
    collideLeft = false;
    
    if(entityType == ENTITY_PLAYER){
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        if(keys[SDL_SCANCODE_RIGHT]){
            acceleration.x = 2.5f;
        }else if(keys[SDL_SCANCODE_LEFT]){
            acceleration.x = -2.5f;
        }else{
            acceleration.x = 0.0f;
        }
        
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 1.5f);
        velocity.x += acceleration.x * elapsed;
        position.x += velocity.x * elapsed;
        
    }if(entityType == ENTITY_ENEMY_SNAIL){
        if(collideTop || collideBottom || collideLeft || collideRight){
            acceleration.x = 20.0f;
            Mix_PlayChannel(-1, screamSound, 0);
//            player = Entity(playerSheet, -3.35f, -1.0f, 1.0f/1.3, 1.5f/1.3, 0.0f, 0.0f, 0.0f, -2.0f, ENTITY_PLAYER);
            
            mode = STATE_LOSE;
            player.position.x = -3.35f;
            player.position.y = -1.0f;
            player.velocity.x = 0.0f;
            player.velocity.y = 0.0f;
            
            velocity.x = 0.0f;
            acceleration.x = -3.0f;
            
        }else{
            bool speed = false;
            if(pow(pow((player.position.x-position.x),2)+pow((player.position.y-position.y),2), 0.5) < 0.8f){
                speed = true;
            }
            
            if(snailNum == 1){
                if(position.x >= -1.5+2.5*0.5*0.2){
                    invertX = false;
                    acceleration.x = -1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = -2.0f;
                    }
                }else if(position.x <= -1.5-2.5*0.5*0.2){
                    invertX = true;
                    acceleration.x = 1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = 2.0f;
                    }
                }
            }else if(snailNum == 2){
                if(position.x >= -1.5+0.8+2.5*0.5*0.2){
                    invertX = false;
                    acceleration.x = -1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = -2.0f;
                    }
                }else if(position.x <= -1.5+0.8-2.5*0.5*0.2){
                    invertX = true;
                    acceleration.x = 1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = 2.0f;
                    }
                }
            }else if(snailNum == 3){
                if(position.x >= -1.5+2.4+2.5*0.5*0.2){
                    invertX = false;
                    acceleration.x = -1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = -2.0f;
                    }
                }else if(position.x <= -1.5+2.4-2.5*0.5*0.2){
                    invertX = true;
                    acceleration.x = 1.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = 2.0f;
                    }
                }
            }else if(snailNum == 4){
                if(position.x >= -1.0+2.5*0.5*0.2){
                    invertX = false;
                    acceleration.x = -3.0f;

                }else if(position.x <= -1.0-2.5*0.5*0.2){
                    invertX = true;
                    acceleration.x = 3.0f;
                }
            }else if(snailNum == 5){
                if(position.x >= 3.55-1.0*0.5*0.2){
                    invertX = false;
                    acceleration.x = -4.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = -5.0f;
                    }
                }else if(position.x <= -3.55+1.0*0.5*0.2){
                    invertX = true;
                    acceleration.x = 4.0f;
                    if(mode == STATE_GAME_LEVEL_3){
                        acceleration.x = 5.0f;
                    }
                }
                
            }
            
            if(speed){
                acceleration.x*=1.025f;
            }
            speed = false;
            
        }
        
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 10.0f);
        velocity.x += acceleration.x * elapsed;
        position.x += velocity.x * elapsed;
        
        
    }else if(entityType == ENTITY_ENEMY_FLY){
        if(collideTop || collideBottom || collideLeft || collideRight){
            acceleration.x = 20.0f;
            Mix_PlayChannel(-1, screamSound, 0);
            mode = STATE_LOSE;
            player.position.x = -3.35f;
            player.position.y = -1.0f;
            player.velocity.x = 0.0f;
            player.velocity.y = 0.0f;
        }else{
            
            if(position.x <= -3.55+1.0*0.5*0.2){
                invertX = true;
                acceleration.x = 6.0f;
            }else if(position.x >= -2.2f){
                invertX = false;
                acceleration.x = -6.0f;
            }
            velocity.x = lerp(velocity.x, 0.0f, elapsed * 10.0f);
            velocity.x += acceleration.x * elapsed;
            position.x += velocity.x * elapsed;
        }
    }
    
//    collideBottom = false;
//    collideTop = false;

    
}

void Entity::updateY(float elapsed){
    collideBottom = false;
    collideTop = false;

    
    if(entityType == ENTITY_PLAYER){
        if(position.x-size.x*0.5 == -2.0f){
            collideBottom = true;
        }
        acceleration.y = -2.0f;
        
        velocity.y += acceleration.y * elapsed;
        position.y += velocity.y * elapsed;
        
        if(position.y <= -2.0f+size.y*0.5){
            position.y = -2.0f+size.y*0.5;
            collideBottom = true;
//            velocity.y *= -0.15;
            velocity.y = 0.0f;
        }
        
    }else if(entityType == ENTITY_ENEMY_SNAIL){
        if(collideTop){
            acceleration.y = -2.0f;
            
            velocity.y += acceleration.y * elapsed;
            position.y += velocity.y * elapsed;
            
            if(position.y <= -2.0f+size.y*0.5){
                position.y = -2.0f+size.y*0.5;
                collideBottom = true;
            }
        }
    }
    
}


void Entity::Render(ShaderProgram* program, Entity* player, float elapsed){
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    modelMatrix.Identity();
    modelMatrix.Translate(position.x, position.y, position.z);
    modelMatrix.Scale(size.x, size.y, size.z);
    
    Matrix viewMatrix;
    
    glUseProgram(program->programID);
    
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelMatrix(modelMatrix);
    
    
    
    program->SetViewMatrix(viewMatrix);
    
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(entityType == ENTITY_PLAYER){
        if(collideBottom == true){
            if(keys[SDL_SCANCODE_RIGHT]){
                //                    drawUniformSpriteSheetSprite(program, 2, 7, 5, uniformSpriteSheetTextureID);
                animate(program, elapsed);
            }else if(keys[SDL_SCANCODE_LEFT]){
                modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
                program->SetModelMatrix(modelMatrix);
                //                    drawUniformSpriteSheetSprite(program, 2, 7, 5, uniformSpriteSheetTextureID);
                animate(program, elapsed);
            }else{
                drawSingleSprite(program, frontTextureID);
            }
        }else{
            drawSingleSprite(program, jumpTextureID);
        }
    }else if(entityType == ENTITY_STATIC){
        //            if(player->collideBottom == true && player->position.y > -2.0f+player->size.y*0.5){
        if(collideTop == true){
            //                light lights[4] = {*new light(player->position.x, player->position.y), *new light(position.x, position.y), *new light(1000.0f, 0.3f), *new light(1000.0f,-0.8f)};
            //                program->SetLightPos(lights);
            drawSingleSprite(program, blockMadTextureID);
            collideTop = false;
        }else{
            drawSingleSprite(program, blockTextureID);
        }
    }else if (entityType == ENTITY_COIN){
        sprite.Draw(program);
    }else if (entityType == ENTITY_ENEMY_SNAIL){
        if(collideTop || collideBottom  || collideLeft || collideRight){
            drawSingleSprite(program, snailShell);
        }else{
            if(invertX){
                modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
                program->SetModelMatrix(modelMatrix);
            }
            animate(program, elapsed);
        }
    }else if (entityType == ENTITY_ENEMY_FLY){
        if(invertX){
            modelMatrix.Scale(-1.0f, 1.0f, 1.0f);
            program->SetModelMatrix(modelMatrix);
        }
        animate(program, elapsed);
    }
}

void Entity::drawUniformSpriteSheetSprite(ShaderProgram* program, int index, int spriteCountX, int spriteCountY, GLuint textureID){
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    
    float texCoods[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    
    float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f,
        -0.5f, 0.5f, -0.5f};
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoods);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

void Entity::drawSingleSprite(ShaderProgram* program, GLuint textureID){
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    float vertices01[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices01);
    glEnableVertexAttribArray(program->positionAttribute);
    
    float texCoords01[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords01);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

void Entity::animate(ShaderProgram* program, float elapsed){
    if(entityType == ENTITY_PLAYER){
        const int runAnimation[] = {3,4,5,6,7};
        const int numFrames = 5;
        float framesPerSecond = 10.0f;
        
        animationElapsed += elapsed;
        
        if(animationElapsed >= 1.0/framesPerSecond){
            currentIndex++;
            animationElapsed = 0.0;
            
            if(currentIndex>numFrames-1){
                currentIndex = 0;
            }
        }
        
        if(runAnimation[currentIndex]==3){
            drawSingleSprite(program, walk03);
        }else if(runAnimation[currentIndex]==4){
            drawSingleSprite(program, walk04);
        }else if(runAnimation[currentIndex]==5){
            drawSingleSprite(program, walk05);
        }else if(runAnimation[currentIndex]==6){
            drawSingleSprite(program, walk06);
        }else{
            drawSingleSprite(program, walk07);
        }
    }else if(entityType == ENTITY_ENEMY_SNAIL){
        const int runAnimation[] = {1, 2};
        const int numFrames = 2;
        float framesPerSecond = 5.0f;
        
        animationElapsed += elapsed;
        
        if(animationElapsed >= 1.0/framesPerSecond){
            currentIndex++;
            animationElapsed = 0.0;
            
            if(currentIndex>numFrames-1){
                currentIndex = 0;
            }
        }
        
        if(runAnimation[currentIndex]==1){
            drawSingleSprite(program, snailWalk01);
        }else if(runAnimation[currentIndex]==2){
            drawSingleSprite(program, snailWalk02);
        }
        
    }else if(entityType == ENTITY_ENEMY_FLY){
        const int runAnimation[] = {1, 2};
        const int numFrames = 2;
        float framesPerSecond = 10.0f;
        
        animationElapsed += elapsed;
        
        if(animationElapsed >= 1.0/framesPerSecond){
            currentIndex++;
            animationElapsed = 0.0;
            
            if(currentIndex>numFrames-1){
                currentIndex = 0;
            }
        }
        
        if(runAnimation[currentIndex]==1){
            drawSingleSprite(program, fly01);
        }else if(runAnimation[currentIndex]==2){
            drawSingleSprite(program, fly02);
        }
    }
}

bool Entity::collidesWithX(Entity* entity, GameMode& mode, Entity* player){
    
    if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
        return false;
    }else{
        if(entity->entityType == ENTITY_COIN){
            entity->position.x += -2000.0f;
            
            Mix_PlayChannel(-1, lolSound, 0);
            
            mode = STATE_WIN;
            player->position.x = -3.35f;
            player->position.y = -1.0f;
            player->velocity.x = 0.0f;
            player->velocity.y = 0.0f;
            
            entity->velocity.x = 0.0f;
            entity->velocity.y = 0.0f;
            
            entity->position.x -= -2000.0f;
            
        }else if(entity->entityType == ENTITY_STATIC){
            double Xpenetration = 0.0f;
            
            Xpenetration = fabs(fabs(position.x-entity->position.x) - size.x*0.5 - entity->size.x*0.5);
            
            if(position.x>entity->position.x){
                position.x = position.x + Xpenetration + 0.00001f;
                collideLeft = true;
            }else{
                position.x = position.x - Xpenetration - 0.000001f;
                collideRight = true;
            }
            
            velocity.x = 0.0f;
        }else if(entity->entityType == ENTITY_ENEMY_SNAIL || entity->entityType == ENTITY_ENEMY_FLY){
            entity->collideLeft = true;
            entity->collideRight = true;
        }
        return true;
    }
}

bool Entity::collidesWithY(Entity* entity, GameMode& mode, Entity* player){
    
    if(position.x+size.x*0.5 < entity->position.x-entity->size.x*0.5 || position.x-size.x*0.5 > entity->position.x+entity->size.x*0.5|| position.y+size.y*0.5 < entity->position.y-entity->size.y*0.5 || position.y-size.y*0.5 > entity->position.y+entity->size.y*0.5){
        return false;
    }else{
        if(entity->entityType == ENTITY_COIN){
            entity->position.x += 2000.0f;
            
            Mix_PlayChannel(-1, lolSound, 0);
            
            mode = STATE_WIN;
            player->position.x = -3.35f;
            player->position.y = -1.0f;
            player->velocity.x = 0.0f;
            player->velocity.y = 0.0f;
            
            
            entity->position.x -= 2000.0f;
        }else if(entity->entityType == ENTITY_STATIC){
            double Ypenetration = 0.0f;
            
            
            Ypenetration = fabs(fabs(position.y-entity->position.y) - size.y*0.5 - entity->size.y*0.5);
            
            if(position.y>entity->position.y){
                position.y = position.y + Ypenetration + 0.00001f;
                collideBottom = true;
                entity->collideTop = true;
            }else{
                position.y = position.y - Ypenetration - 0.00001f;
                collideTop = true;
            }
            
            velocity.y = 0.0f;
            //Restituution val set to 0.2
//            velocity.y *= -0.15;
            
        }else if(entity->entityType == ENTITY_ENEMY_SNAIL || entity->entityType == ENTITY_ENEMY_FLY){
            if(position.y>entity->position.y){
                collideBottom = true;
                entity->collideTop = true;
            }else{
                collideTop = true;
                entity->collideBottom = true;
            }
        }
        return true;
    }
}
