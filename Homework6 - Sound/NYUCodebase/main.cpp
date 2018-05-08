#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <vector>
#include "Matrix.h"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif



SDL_Window* displayWindow;
#define MAX_BULLETS 300
int bulletIndex = 0;
float count = 0.0f;
bool win = false;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER, STATE_WIN};



GLuint LoadTexture(const char* filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is corret\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}


class SheetSprite{
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
                size):textureID(textureID), u(u), v(v), width(width), height(height), size(size){ }
    
    void Draw(ShaderProgram *program) const {
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        
        glUseProgram(program->programID);
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    
    
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};

void DrawText(ShaderProgram *program, GLuint fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, text.size()*6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

class Vector3{
public:
    Vector3(float x, float y, float z):x(x), y(y), z(z){};
    
    float x;
    float y;
    float z;
};

class Entity_textured{
public:
    
    Entity_textured(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};
    
    void Draw_text(ShaderProgram* program, GLuint fontTexture, const std::string& text){
        Matrix projectionMatrix;
        Matrix modelMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        modelMatrix.Translate(position.x, position.y, position.z);
        Matrix viewMatrix;
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        DrawText(program, fontTexture, text, size.x, -size.x/5);
    }
    
    
    void Draw(ShaderProgram* program, const SheetSprite& sprite) const {
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        modelMatrix.Translate(position.x, position.y, position.z);
        modelMatrix.Scale(size.x, size.y, size.z);

        Matrix viewMatrix;
        
        glUseProgram(program->programID);
        
        program->SetProjectionMatrix(projectionMatrix);
        program->SetModelMatrix(modelMatrix);
        program->SetViewMatrix(viewMatrix);
        
        sprite.Draw(program);
    }
    
    void update(float elasped){
        position.x += elasped * 0.3f;
        
        //0.5 is the initial size and 0.2 is the sprite size
        if(position.x >= 3.55+size.x*0.5*0.2){
            position.x = -3.55f-size.x*0.5*0.2;
        }
    }
    
    void shootBullets(std::vector<Entity_textured>& bullets){
        bullets[bulletIndex].position.x = position.x;
        bullets[bulletIndex].position.y = position.y - size.y*0.5*0.2;
        bullets[bulletIndex].velocityY = -2.0f;
        bulletIndex++;
        if(bulletIndex > MAX_BULLETS-1) {
            bulletIndex = 0;
        }
    }
    
    Vector3 position;
    Vector3 size;
    float velocityY;
};

class Entity_untextured{
public:
    
    Entity_untextured(float positionX, float positionY, float sizeX, float sizeY):position(positionX, positionY, 0.0f), size(sizeX, sizeY,0.0f){};
    
    void Draw(ShaderProgram* program){
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
        program->SetColor(1.0f, 1.0f, 1.0f, 0.8f);
        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program->positionAttribute);
    }
    
    
    Vector3 position;
    Vector3 size;
};

void setup(ShaderProgram* program, ShaderProgram* program_untextured){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 1280, 720);
    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    
    program->Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program_untextured->Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    Mix_Music *music;
    music = Mix_LoadMUS("cheetos.wav");
    
    Mix_VolumeMusic(30);
    Mix_PlayMusic(music, -1);
}

void ProcessGameInput(SDL_Event* event, bool& done, std::vector<Entity_textured>& bullets, const Entity_textured& player, Mix_Chunk *shootingSound){
    while (SDL_PollEvent(event)){
        if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }else if (event->type == SDL_KEYDOWN){
            if(event->key.keysym.scancode == SDL_SCANCODE_DOWN){
                
                bullets[bulletIndex].position.x = player.position.x;
                bullets[bulletIndex].position.y = player.position.y + player.size.y*0.5*0.2;
                bullets[bulletIndex].velocityY = 5.0f;
                bulletIndex++;
                Mix_PlayChannel(1, shootingSound, 0);
                if(bulletIndex > MAX_BULLETS-1) {
                    bulletIndex = 0;
                }
               
            }
        }
    }
}


void ProcessMainMenuInput(SDL_Event* event, bool& done, Entity_untextured* board, GameMode& gameMode){
    std::cout << "PLEASE LEFT CLICK TO START THE GAME" << std::endl;
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }else if(event->type == SDL_MOUSEBUTTONDOWN){
            if(event->button.button == 1){
                float unitX = (((float)event->button.x / 1280.0f) * 3.554f ) - 1.777f;
                float unitY = (((float)(720-event->button.y) / 720.0f) * 2.0f ) - 1.0f;
                
                if(unitX>=board->position.x-board->size.x/4 && unitX<=board->position.x+board->size.x/4 && unitY>=board->position.y-board->size.y/4 && unitY<=board->position.y+board->size.y/4){
                    gameMode = STATE_GAME_LEVEL;
                }


            }
        }
    }
}

void ProcessGameOverInput(SDL_Event* event, bool& done){
    while(SDL_PollEvent(event)){
        if(event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE){
            done = true;
        }
    }
}

void RenderGame(ShaderProgram* program, const std::vector<Entity_textured>& enemies, const SheetSprite& EnemySprite, const Entity_textured& player, const SheetSprite& PlayerSprite, const std::vector<Entity_textured>& bullets, const SheetSprite& bulletSprite ){
    
    for(GLsizei i=0; i<enemies.size(); i++){
        enemies[i].Draw(program, EnemySprite) ;
    }
    
    for(GLsizei i=0; i<bullets.size(); i++){
        bullets[i].Draw(program, bulletSprite);
    }
    
    player.Draw(program, PlayerSprite);

}

void UpdateGame(std::vector<Entity_textured>& enemies, Entity_textured& player, std::vector<Entity_textured>& bullets, float elapsed, GameMode& gameMode, Mix_Chunk *exploSound){

    count += elapsed;
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_LEFT]){
        player.position.x -= elapsed * 1.6f;
        if(player.position.x <= -3.55+player.size.x*0.5*0.2){
            player.position.x = -3.55+player.size.x*0.5*0.2;
        }
        
    } else if(keys[SDL_SCANCODE_RIGHT]){
        player.position.x += elapsed * 1.6f;
        if(player.position.x >= 3.55-player.size.x*0.5*0.2){
            player.position.x = 3.55-player.size.x*0.5*0.2;
        }
    }
    
    for (GLsizei i=0; i<bullets.size(); i++){
        bullets[i].position.y += bullets[i].velocityY*elapsed;
    }
    
    for(GLsizei i=0; i<enemies.size(); i++){
        enemies[i].update(elapsed);
    }
    
    if(count >= 2.0f){
        for(GLsizei i=0; i<enemies.size(); i++){
            if(i%5 == 0){
                enemies[i].shootBullets(bullets);
            }
        }
        count-=2.0f;
    }
    

    for(GLsizei i=0; i<bullets.size(); i++){

        Entity_textured bullet = bullets[i];
        for(GLsizei j=0; j<enemies.size(); j++){

            Entity_textured enemy = enemies[j];
            if(bullet.position.x+bullet.size.x*0.2*0.5 < enemy.position.x-enemy.size.x*0.2*0.5 || bullet.position.x-bullet.size.x*0.2*0.5 > enemy.position.x+enemy.size.x*0.2*0.5 || bullet.position.y+bullet.size.y*0.2*0.5 < enemy.position.y-enemy.size.y*0.2*0.5 || bullet.position.y-bullet.size.y*0.2*0.5 > enemy.position.y+enemy.size.y*0.2*0.5){
            }else{
                if(bullets[i].velocityY >= 0){
                    Mix_PlayChannel(-1, exploSound, 0);
                    bullets[i].position.x = -200.0f;
                    enemies[j].position.x = -200.0f;
                }
            }
            
            if(bullet.position.x+bullet.size.x*0.2*0.5 < player.position.x-player.size.x*0.2*0.5 || bullet.position.x-bullet.size.x*0.2*0.5 > player.position.x+player.size.x*0.2*0.5 || bullet.position.y+bullet.size.y*0.2*0.5 < player.position.y-player.size.y*0.2*0.5 || bullet.position.y-bullet.size.y*0.2*0.5 > player.position.y+player.size.y*0.2*0.5){
            }else{
                if(bullets[i].velocityY <= 0){
                    gameMode = STATE_GAME_OVER;
                }
        }
        
        }
    
    }
    
    win = true;
    for(GLsizei i=0; i<enemies.size(); i++){
        if(enemies[i].position.x > -3.55f && enemies[i].position.x < 3.55f){
            win = false;
        }
    }

    if (win){
        gameMode = STATE_WIN;
    }


}

void RenderMainMenu(ShaderProgram* program, ShaderProgram* program_untextured, Entity_textured& font, Entity_untextured& board, GLuint fontTexture){
    font.Draw_text(program, fontTexture, "START GAME");
    board.Draw(program_untextured);
}

void RenderGameOver(ShaderProgram* progam, Entity_textured& gameoverFont, GLuint fontTeture){
    gameoverFont.Draw_text(progam, fontTeture, "GAME OVER");
}

void RenderWin(ShaderProgram* progam, Entity_textured& gameoverFont, GLuint fontTeture){
    gameoverFont.Draw_text(progam, fontTeture, "YOU'VE WON!!");
}

void UpdateMainMenu(){}

void cleanup(){

    SDL_Quit();
}


int main(int argc, char *argv[])
{
    ShaderProgram program;
    ShaderProgram program_untextured;
    setup(&program, &program_untextured);
    
    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    Mix_Chunk *shootingSound;
    shootingSound = Mix_LoadWAV("gun.wav");
    Mix_VolumeChunk(shootingSound, 10);
    
    Mix_Chunk *exploSound;
    exploSound = Mix_LoadWAV("explo.wav");
    Mix_VolumeChunk(exploSound, 5);
    

    
    SheetSprite EnemySprite = SheetSprite(spriteSheetTexture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
    SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 247.0f/1024.0f, 84.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    SheetSprite BulletSprite = SheetSprite(spriteSheetTexture, 858.0f/1024.0f, 230.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 0.2);
    
    float lastFrameTicks = 0.0f;

    
    SDL_Event event;
    bool done = false;
    
    GameMode mode = STATE_MAIN_MENU;
    Entity_textured font(-1.08f, 0.0f, 0.3f, 0.3f);
    Entity_textured gameoverFont(-1.08f, 0.0f, 0.3f, 0.3f);
    Entity_untextured board(0.0f, 0.0f, 2.4f, 0.9f);
    Entity_textured player(0.0f, -1.3f, 1.5f, 1.5f);
    

    
    std::vector<Entity_textured> enemies;
    
    float initialX = -3.55f-0.5*0.2*1.5f;
    float initialY = 1.8f;
    
    for(GLsizei j=0; j<5; j++){
        for(GLsizei i=0; i<11; i++){
            Entity_textured newEnemy(initialX, initialY, 1.5f, 1.5f);
            initialX+=0.5*1.3f;
            enemies.push_back(newEnemy);
        }
        initialX = -3.55f-0.5*0.2*1.5f;
        initialY-=0.4f;
    }
    

    std::vector<Entity_textured> bullets;
    for(GLsizei i=0; i<MAX_BULLETS; i++){
        Entity_textured newBullet(-2000.0f, 0.0f, 1.5f, 1.5f);
        bullets.push_back(newBullet);
    }
        
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        if(mode == STATE_MAIN_MENU){
            ProcessMainMenuInput(&event, done, &board, mode);
        }else if(mode == STATE_GAME_LEVEL){
            ProcessGameInput(&event, done, bullets, player, shootingSound);
        }else if(mode == STATE_GAME_OVER){
            ProcessGameOverInput(&event, done);
        }else if(mode == STATE_WIN){
            ProcessGameOverInput(&event, done);
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        if(mode == STATE_MAIN_MENU){
            RenderMainMenu(&program, &program_untextured, font, board, fontTexture);
            UpdateMainMenu();
        }else if(mode == STATE_GAME_LEVEL){
            UpdateGame(enemies, player, bullets, elapsed, mode, exploSound);
            RenderGame(&program, enemies, EnemySprite, player, playerSprite, bullets, BulletSprite);
        }else if(mode == STATE_GAME_OVER){
            RenderGameOver(&program, gameoverFont, fontTexture);
        }else if(mode == STATE_WIN){
            RenderWin(&program, gameoverFont, fontTexture);
        }
        

        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    cleanup();
    return 0;
}

