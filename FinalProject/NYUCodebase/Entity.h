//
//  Entity.h
//  NYUCodebase
//
//  Created by Bixing Xie on 5/7/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Entity_h
#define Entity_h

#include "SheetSprite.h"
#include "Vector3.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>



enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL_1, STATE_GAME_LEVEL_2, STATE_GAME_LEVEL_3, STATE_GAME_OVER, STATE_WIN, STATE_LOSE};
enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY_SNAIL, ENTITY_ENEMY_FLY, ENTITY_COIN, ENTITY_STATIC};

class Entity{
public:
    
    Entity();
    
    Entity(const SheetSprite& sprite, float positionX, float positionY, float sizeX, float sizeY, float velocityX, float velocityY, float accelerationX, float accelerationY, EntityType entityType);
    
    void UpdateX(float elapsed, GameMode& mode, Entity& player);
    
    void UpdateY(float elapsed);
    
    void Render(ShaderProgram* program, Entity* player, float elapsed);
    
    void drawUniformSpriteSheetSprite(ShaderProgram* program, int index, int spriteCountX, int spriteCountY, GLuint textureID);
    
    void drawSingleSprite(ShaderProgram* program, GLuint textureID);
    
    void animate(ShaderProgram* program, float elapsed);
    
    bool CollidesWithX(Entity* entity, GameMode& mode, Entity* player);
    
    bool CollidesWithY(Entity* entity, GameMode& mode, Entity* player);
                       
    
    SheetSprite sprite;
    
    
    Vector3 position;
    Vector3 size;
    Vector3 velocity;
    Vector3 acceleration;
    
    Matrix modelMatrix;
    
    //    bool isStatic;
    
    float animationElapsed = 0.0f;
    int currentIndex = 0;
    
    EntityType entityType;
    GLuint frontTextureID;
    GLuint jumpTextureID;
    GLuint uniformSpriteSheetTextureID;
    GLuint blockTextureID;
    GLuint blockMadTextureID;
    GLuint walk03;
    GLuint walk04;
    GLuint walk05;
    GLuint walk06;
    GLuint walk07;
    
    GLuint fly01;
    GLuint fly02;
    
    GLuint snailWalk01;
    GLuint snailWalk02;
    GLuint snailShell;
    
    bool collideTop=false;
    bool collideBottom=false;
    bool collideLeft=false;
    bool collideRight=false;
    
    bool invertX = false;
    GLuint snailNum;
    
    Mix_Chunk* screamSound;
    
};


#endif /* Entity_h */
