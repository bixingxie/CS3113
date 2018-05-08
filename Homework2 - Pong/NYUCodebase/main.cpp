#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

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


class LeftPaddle{
public:
    
    void Draw(ShaderProgram &program){
        Matrix modelMatrix;
        program.SetModelMatrix(modelMatrix);
        
        modelMatrix.Translate(x, y, 0.0f);
        modelMatrix.Scale(width, height, 1.0f);
        program.SetModelMatrix(modelMatrix);

        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
    
    }
    
    float x = -3.2f;
    float y = 0.0f;

    float width = 0.2f;
    float height = 0.8f;
    float velocity_y = 4.0f;
    
};

class RightPaddle{
public:
    void Draw(ShaderProgram &program){
        Matrix modelMatrix;
    
        modelMatrix.Translate(x, y, 0.0f);
        modelMatrix.Scale(width, height, 1.0f);
        program.SetModelMatrix(modelMatrix);
        
        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        
    }
    
    float x = 3.2f;
    float y = 0.0f;
    float width = 0.2f;
    float height = 0.8f;
    float velocity_y = 4.0f;
    
};

class Ball{
public:
    void Draw(ShaderProgram &program){
        
        Matrix projectionMatrix;
        projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
        Matrix modelMatrix;
        Matrix viewMatrix;
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        modelMatrix.Translate(x, y, 0.0f);
        modelMatrix.Scale(width, height, 1.0f);
        program.SetModelMatrix(modelMatrix);
        
        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        
    }
    
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.2f;
    float height = 0.2f;
    float velocity_x = 5.5f;
    float velocity_y = 5.5f;
    
};



void SetUp(ShaderProgram& program){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    glViewport(0, 0, 640, 360);
    
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glUseProgram(program.programID);
    
    Matrix projectionMatrix;
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    
    Matrix viewMatrix;
    program.SetViewMatrix(viewMatrix);
}

void ProcessEvents(SDL_Event& event, bool& done){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
            
        }
    }
}




void Update(ShaderProgram& program, float& lastFrameTicks, LeftPaddle& left, RightPaddle& right, Ball& ball){
    glClear(GL_COLOR_BUFFER_BIT);
    
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_UP]){
        right.y += right.velocity_y * elapsed;
        if(right.y >= 1.60f){
            right.y = 1.60f;
        }
    }else if (keys[SDL_SCANCODE_DOWN]){
        right.y -= right.velocity_y * elapsed;
        if(right.y <= -1.60f){
            right.y = -1.60f;
        }
    }
    
    
    
    if (keys[SDL_SCANCODE_W]){
        left.y += left.velocity_y * elapsed;
        if(left.y >= 1.60f){
            left.y = 1.60f;
        }
    }else if (keys[SDL_SCANCODE_S]){
        left.y -= left.velocity_y * elapsed;
        if(left.y <= -1.60f){
            left.y = -1.60f;
        }
    }
    
    ball.x += cos(45 *3.1415926/180)* elapsed * ball.velocity_x;
    ball.y += sin(45 *3.1415926/180)* elapsed * ball.velocity_y;
    
    if(ball.y > 1.90f){
        if(ball.velocity_y > 0.0f){
            ball.velocity_y *= -1.0f;
        }
    }
    
    if(ball.y < (-1.90f)){
        if(ball.velocity_y < 0.0f){
            ball.velocity_y *= -1.0f;
        }
    }
    
    
    if(ball.x+0.1f < right.x-0.1f || ball.x-0.1f > right.x+0.1f || ball.y+0.1f < right.y-0.4f || ball.y-0.1f > right.y+0.4f){
        //not colliding
    }else {
        if(ball.velocity_x >= 0.0f){
            ball.velocity_x *= -1.0f;
        }
    }
    
//    if(ball.x+0.1f >= right.x-0.1f && ball.x-0.1f <= right.x+0.1f && ball.y+0.1f >= right.y-0.4f && ball.y-0.1f <= right.y+0.4f){
//        ball.velocity_x *= -1.0f;
//    }
//
//    if(ball.x+0.1f >= left.x-0.1f && ball.x-0.1f <= left.x+0.1f && ball.y+0.1f >= left.y-0.4f && ball.y-0.1f <= left.y+0.4f){
//        ball.velocity_x *= -1.0f;
//    }
    
    
    if(ball.x+0.1f < left.x-0.1f || ball.x-0.1f > left.x+0.1f || ball.y+0.1f < left.y-0.4f || ball.y-0.1f > left.y+0.4f){
        //not colliding
    }else {
        if(ball.velocity_x <= 0.0f){
            ball.velocity_x *= -1.0f;
        }
    }
    
    
    if(ball.x >= 3.45f){
        std::cout << "LEFT PLAYER WINS!!! YAY!!!" << std::endl;
    }
    else if(ball.x <= -3.45f){
        std::cout << "RIGHT PLAYER WINDS!!! YAY!!!" << std::endl;
    }
    
    left.Draw(program);
    right.Draw(program);
    
    if(ball.x > -3.50f && ball.x < 3.50f){
        ball.Draw(program);
    }
    
    SDL_GL_SwapWindow(displayWindow);
}

void Cleanup(){
    SDL_Quit();
}

void Render(){ }



int main(int argc, char *argv[])
{
    
    ShaderProgram program;
    SetUp(program);

    
#ifdef _WINDOWS
    glewInit();
#endif
    
    float lastFrameTicks = 0.0f;
    LeftPaddle left;
    RightPaddle right;
    Ball ball;
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        ProcessEvents(event, done);
        Update(program, lastFrameTicks, left, right, ball);

    }
    
    Cleanup();
    return 0;
}

