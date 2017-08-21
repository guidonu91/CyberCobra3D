/*
Snake 3D
TP INFO 3 2014
ING. INFORMATICA - UCA 
JIMENEZ - NUÑEZ
 */

#include <GL/glut.h>
// #include <GL/glext.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "bitmap.h"
//Constantes
typedef enum{NORTH, WEST, SOUTH, EAST} directions;
typedef enum{MENU, MAIN_GAME, GAME_OVER, RANKING, NEW_HIGHSCORE} screenTypes;
typedef enum{WHITE, BLUE, RED, YELLOW, GREEN} colors;
typedef enum{NORMAL, SNAKE, UPPER} views;
typedef enum{NEW_GAME, TOP, EXIT, RETURN_MENU} menuOptions;
#define RADIUS 0.5
#define SMALL_RADIUS 0.25
#define BIG_RADIUS 1
#define NORMAL_VIEW 20 
#define SNAKE_VIEW 2
#define UPPER_VIEW 40
#define SLOW_FACTOR 1.25
#define SPEED_FACTOR 1.1
#define BASE_SPEED 75
#define SNAKE_DX 5.0
#define SNAKE_DZ 5.0
#define NORMAL_DX 20
#define NORMAL_DZ 20
//TEXTURAS
#define VERTICAL 0 
#define HORIZONTAL 1
//Prototipos
static void display(void);
//Variables Globales
struct coordinates{
    float x_pos;
    float z_pos;
};
struct coordinates body[500];
struct coordinates fruit;
static int slices = 20;
static int stacks = 20;
int cameraHeight = NORMAL_VIEW;
views view = NORMAL;
float x =   -25.0;
float z =   10.0;
float dx =  0.0;
float dz =  -NORMAL_DZ;
int bodyParts = 5;
screenTypes screen = MENU;
directions direction = NORTH;
colors fruitColor;
float fruitRadius = RADIUS;
int fromCenterLvl2 = 1;
int fromCenterLvl3 = 1;
int level = 1;
int score = 0;
int speed = BASE_SPEED;
int speedAux;
double elapsedTime;
double invincibleTime;
double slowingTime;
int slowingPower = 0;
int invincible = 0;
int slower = 0;
int paused = 0;
//PARA TEXTURAS
BITMAPINFO *TexInfo; /* Texture bitmap information */
GLubyte *TexBits; /* Texture bitmap pixel bits */
//PARA RANKING
struct ranking{
    char score[10];
    char name[11];
    int points;
};
struct ranking top3[3];
char newName[11];
//
double pausedTime = 0.0;
double startPauseTime = 0.0;
double gameTime = 0.0;
double accelerateTime = 0.0;
//
int changeSpeed = 1;
//
menuOptions menuOption = NEW_GAME;
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 0.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

static void resize(int width, int height){
    const float ar = (float) width / (float) height;
    glViewport(0, 0, width, height);                // indica la relacion dibujo ventana (ini, ini, fin, fin)
    glMatrixMode(GL_PROJECTION);                    // GL_PROJECTION es una matriz vectorial, da un efecto de proyeccion
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);      // izq, der, base, altura, posicion_ini, posicion_fin 
    glMatrixMode(GL_MODELVIEW);
}
//Procedimiento que setea el color de la siguiente fruta a colocar en el terreno de juego.
void fruitNextColor(){
    int chance = rand()%100;
    if(chance < 50)
        fruitColor = WHITE;
    else if(chance >= 50 && chance < 65)
        fruitColor = BLUE;
    else if(chance >= 65 && chance < 80)
        fruitColor = RED;
    else if(chance >= 80 && chance < 90)
        fruitColor = YELLOW;
    else 
        fruitColor = GREEN;
    if((invincible || level == 1) && fruitColor == RED)//Controlar que no setee otra fruta roja mientras sea invencible
        fruitNextColor();
    if((slower || level == 1) && fruitColor == BLUE)//Controlar que no setee otra fruta azul mientras sea mas lento
        fruitNextColor();
}
//Procedimiento que genera la posicion de la fruta a colocar en el terreno de juego.
void generateFruitPosition() {
    int i;
    float k,j;
    fruit.x_pos = -((rand() % 49)+1);
    fruit.z_pos = (rand() % 29) + 1;
    //CONTROLAR QUE NO ELIJA LAS MISMAS COORDENADAS QUE ALGUN PEDAZO DEL CUERPO DE LA VIBORA
    for(i = 0; i < bodyParts; i++)
        if (body[i].x_pos == fruit.x_pos && body[i].z_pos == fruit.z_pos)
            generateFruitPosition();
    //CONTROLAR QUE NO ELIJA LAS MISMAS COORDENADAS DE ALGUN OBSTACULO DE ALGUN NIVEL
    if(level == 2){
        for(k = 9.0; k < 21.0; k+=1.0)
            for(j = -11.0; j >= -13.0; j-=1.0)
                if(fruit.x_pos == j && fruit.z_pos == k)
                    generateFruitPosition();
        for(k = 9.0; k < 21.0; k+=1.0)
            for(j = -36.0; j >= -38.0; j-=1.0)
                if(fruit.x_pos == j && fruit.z_pos == k)
                    generateFruitPosition();
    }
    if(level == 3){
        for(k = 4.0; k < 18.0; k+=1.0)
            for(j = -11.0; j >= -13.0; j-=1.0)
               if(fruit.x_pos == j && fruit.z_pos == k)
                generateFruitPosition();
        for(k = 12.0; k < 26.0; k+=1.0)
            for(j = -36.0; j >= -38.0; j-=1.0)
               if(fruit.x_pos == j && fruit.z_pos == k)
                generateFruitPosition();
        for(k = -4.0; k > -21.0; k-=1.0)
            for(j = 23.0; j <= 25.0; j+=1.0)
               if(fruit.x_pos == k && fruit.z_pos == j)
                generateFruitPosition();
        for(k = -29.0; k > -46.0; k-=1.0)
            for(j = 5.0; j <= 7.0; j+=1.0)
               if(fruit.x_pos == k && fruit.z_pos == j)
                generateFruitPosition();
    }
    fruitNextColor();
}
//Procedimiento que dibuja la fruta en el terreno, atendiendo los tamanhos y los radios respectivos.
static void drawFruit(){
    if(fruitColor == WHITE){
        glColor3d(1,1,1);
        fruitRadius = RADIUS;
    }
    if(fruitColor == BLUE){
        glColor3d(0,0,1);
        fruitRadius = SMALL_RADIUS;
    }
    if(fruitColor == RED){
        glColor3d(1,0,0);
        fruitRadius = RADIUS;
    }
    if(fruitColor == YELLOW){
        glColor3d(1,1,0);
        fruitRadius = SMALL_RADIUS;
    }
    if(fruitColor == GREEN){
        glColor3d(0,0.5,0);
        fruitRadius = BIG_RADIUS;
    }
    glPushMatrix();
        glTranslated(fruit.x_pos,fruitRadius,fruit.z_pos);
        glutSolidSphere(fruitRadius,slices,stacks);
    glPopMatrix();
}
//Procedimiento que inicializa a la serpiente, la coloca en el centro del terreno y genera su cuerpo inicial.
void firstSnake(){
    int index;
    body[0].x_pos = x;
    body[0].z_pos = z;
    for (index = 1; index < bodyParts; index++)
    {
        body[index].z_pos = body[index-1].z_pos-1;
        body[index].x_pos = body[index-1].x_pos; 
    } 
    generateFruitPosition();  
}
//Procedimiento que dibuja las partes del cuerpo de la serpiente
static void generateBodyParts(float x_pos, float z_pos){
    glPushMatrix();
        glTranslated(x_pos,0.25,z_pos);
        glutSolidSphere(RADIUS,slices,stacks);
    glPopMatrix();
}
//Generar los ojos de la serpiente
static void generateSnakeEyes(float x_pos, float z_pos){
    
    if(direction == EAST || direction == WEST){
        glPushMatrix();
            glTranslated(x_pos,1,z_pos-0.2);
            glutSolidSphere(0.2,slices,stacks);
        glPopMatrix();
        glPushMatrix();
            glTranslated(x_pos,1,z_pos+0.2);
            glutSolidSphere(0.2,slices,stacks); 
        glPopMatrix();   
    } else if(direction == NORTH || direction == SOUTH){
        glPushMatrix();
            glTranslated(x_pos-0.2,1,z_pos);
            glutSolidSphere(0.2,slices,stacks);
        glPopMatrix();
        glPushMatrix();
            glTranslated(x_pos+0.2,1,z_pos);
            glutSolidSphere(0.2,slices,stacks);  
        glPopMatrix();
    } 
}
//Procedimiento que controla el dibujo de la serpiente.
static void drawSnake (){
    int i;
    //Cabeza de la Serpiente
    glColor3d(0.2,0.4,0.3);  
    generateBodyParts(body[0].x_pos,body[0].z_pos);
    glColor3d(0.0,0.0,0.0);
    generateSnakeEyes(body[0].x_pos,body[0].z_pos);
    //Cuerpo Serpiente
    glColor3d(0.5,0.75,0);
    for (i = 1; i < bodyParts; i++)
        generateBodyParts(body[i].x_pos,body[i].z_pos);
}
//Procedimiento principal que controla el movimiento de la serpiente, 
//Se analizan las colisiones, el paso de niveles, que fruta se comio.
void snakeMovement(){
    int index;
    float i,j;
    for (index = bodyParts; index > 0; index--)
    {
        body[index].x_pos = body[index-1].x_pos;
        body[index].z_pos = body[index-1].z_pos;   
    }   
    if ( direction == WEST && x<0 ) {
        x = x+1;
    }
    if ( direction == NORTH && z<30 ) {
        z = z+1;
    }
    if ( direction == EAST && x>-50 ) {
        x = x-1;
    }
    if ( direction == SOUTH && z>0 ){
        z = z-1;
    }
    body[0].x_pos = x;
    body[0].z_pos = z;
    //Controlar si comio una frutita
    if( (fruit.x_pos == body[0].x_pos && fruit.z_pos == body[0].z_pos) || 
        (fruitRadius == BIG_RADIUS && ( ( ( (fruit.z_pos + BIG_RADIUS) == body[0].z_pos || (fruit.z_pos - BIG_RADIUS) == body[0].z_pos) 
            && fruit.x_pos == body[0].x_pos) || ( ( (fruit.x_pos + BIG_RADIUS) == body[0].x_pos || (fruit.x_pos - BIG_RADIUS) == body[0].x_pos) 
            && fruit.z_pos == body[0].z_pos) ) ) ){
        if(fruitColor == WHITE)
            score += 25;
        else if(fruitColor == YELLOW)
            score += 10;
        else if(fruitColor == GREEN)
            score += 100;
        else if(fruitColor == RED){
            invincible = 1;
            invincibleTime = (glutGet(GLUT_ELAPSED_TIME)/ 1000.0) - pausedTime;
        }
        else if(fruitColor == BLUE){
            slowingPower = 1;
        }
        bodyParts++;
        if(score > 125 && score < 250){
            level = 2;
            bodyParts = 7;
            if (fromCenterLvl2){ 
                system("aplay newLevel.wav &");
                direction = NORTH; 
                dx =  0.0;
                if(view == NORMAL)
                    dz =  -NORMAL_DZ;
                else
                    dz = -SNAKE_DZ; 
                x = -25.0;
                z = 10.0;
                body[0].x_pos = x;
                body[0].z_pos = z;
                for (index = 1; index < bodyParts; index++)
                {
                    body[index].z_pos = body[index-1].z_pos-1;
                    body[index].x_pos = body[index-1].x_pos; 
                }
                fromCenterLvl2 = 0;
            }
        }
        else if (score > 250){
            level = 3;
            if (fromCenterLvl3){
                system("aplay newLevel.wav &");
                x = -25.0;
                z = 13.0;
                bodyParts = 10;
                direction = NORTH; 
                dx =  0.0;
                if(view == NORMAL)
                    dz =  -NORMAL_DZ;
                else
                    dz = -SNAKE_DZ; 
                body[0].x_pos = x;
                body[0].z_pos = z;
                for (index = 1; index < bodyParts; index++)
                {
                    body[index].z_pos = body[index-1].z_pos-1;
                    body[index].x_pos = body[index-1].x_pos; 
                }
                fromCenterLvl3 = 0;
            }   
        }
        generateFruitPosition();
    }

    //CONTROL DE COLISION CON LAS PAREDES 
    if(body[0].x_pos >= 0 || body[0].x_pos <= -50 || body[0].z_pos >= 30 || body[0].z_pos <= 0){
        system("aplay bang.wav&");
        // system("killall aplay");
        // system("aplay perdi.wav&");
        menuOption = RETURN_MENU;
        screen = GAME_OVER;
    }
    
    //CONTROL DE COLISION CON EL PROPIO CUERPO DE LA VIBORA
    for(index = 1; index < bodyParts; index++)
        if (body[0].x_pos == body[index].x_pos && body[0].z_pos == body[index].z_pos){
            system("aplay bang.wav&");
            // system("aplay perdi.wav&");
            menuOption = RETURN_MENU;
            screen = GAME_OVER;
        }

    //CONTROL DE COLISION CON LOS OBSTACULOS POR NIVELES
    if(level == 2 && !invincible){
        for(i = 9.0; i < 21.0; i+=1.0)
            for(j = -11.0; j >= -13.0; j-=1.0)
                if(body[0].x_pos == j && body[0].z_pos == i){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
                }
        for(i = 9.0; i < 21.0; i+=1.0)
            for(j = -36.0; j >= -38.0; j-=1.0)
                if(body[0].x_pos == j && body[0].z_pos == i){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
                }
    }
    if(level == 3 && !invincible){
        for(i = 4.0; i < 18.0; i+=1.0)
            for(j = -11.0; j >= -13.0; j-=1.0)
               if(body[0].x_pos == j && body[0].z_pos == i){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
               }
        for(i = 12.0; i < 26.0; i+=1.0)
            for(j = -36.0; j >= -38.0; j-=1.0)
               if(body[0].x_pos == j && body[0].z_pos == i){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
               }
        for(i = -4.0; i > -21.0; i-=1.0)
            for(j = 23.0; j <= 25.0; j+=1.0)
               if(body[0].x_pos == i && body[0].z_pos == j){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
               }
        for(i = -29.0; i > -46.0; i-=1.0)
            for(j = 5.0; j <= 7.0; j+=1.0)
               if(body[0].x_pos == i && body[0].z_pos == j){
                    system("aplay bang.wav&");
                    // system("aplay perdi.wav&");
                    menuOption = RETURN_MENU;
                    screen = GAME_OVER;
               }
    }
}
//Procedimiento que da la velocidad al juego.
void timer(int value) {
    if(!paused && screen == MAIN_GAME){
        display();
        snakeMovement();
        glutTimerFunc(speed, timer, 0);
    }
}
void processTexture(char *image_path){
    free(TexBits);
    free(TexInfo);
    TexBits = LoadDIBitmap(image_path, &TexInfo);
    
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TexInfo->bmiHeader.biWidth, TexInfo->bmiHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, TexBits);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
}
//Procedimientos que dibujan el terreno de juego.
static void drawField(){
    float i;
        
    processTexture("grass.bmp");    
    
    glPushMatrix();     
        
        glColor3d(1.0,1.0,1.0); 
        
        //PISO        
        glEnable(GL_TEXTURE_2D);
        
        glBegin(GL_QUADS);
            glVertex3f(-50,0,0);
            glTexCoord2f(8.0, 4.0);
                    
            glVertex3f(-50,0,30);
            glTexCoord2f(0.0, 4.0);
                
            glVertex3f(0,0,30);
            glTexCoord2f(0.0, 0.0);
                  
            glVertex3f(0,0,0);
            glTexCoord2f(8.0, 0.0);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        
        //PAREDES
        processTexture("lot_fence.bmp");
        
        //PARED OESTE
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        
        glBegin(GL_QUADS);
            glVertex3f(0,0,30);
            glTexCoord2f(8.0, 1.0);
                    
            glVertex3f(0,5,30);
            glTexCoord2f(0.0, 1.0);
                
            glVertex3f(0,5,0);
            glTexCoord2f(0.0, 0.0);
                  
            glVertex3f(0,0,0);
            glTexCoord2f(8.0, 0.0);                          
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
                
        //PARED ESTE
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);       
        glEnable(GL_TEXTURE_2D);
        
        glBegin(GL_QUADS); 
            glVertex3f(-50,0,30);
            glTexCoord2f(8.0, 1.0);
                    
            glVertex3f(-50,5,30);
            glTexCoord2f(0.0, 1.0);
                
            glVertex3f(-50,5,0);
            glTexCoord2f(0.0, 0.0);
                  
            glVertex3f(-50,0,0);
            glTexCoord2f(8.0, 0.0);                          
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        
        //PARED SUR
        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);       
        glEnable(GL_TEXTURE_2D);
        
        glBegin(GL_QUADS); 
            glVertex3f(-50,0,0);
            glTexCoord2f(8.0, 1.0);
                    
            glVertex3f(-50,5,0);
            glTexCoord2f(0.0, 1.0);
                
            glVertex3f(0,5,0);
            glTexCoord2f(0.0, 0.0);
                  
            glVertex3f(0,0,0);
            glTexCoord2f(8.0, 0.0);                             
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        
        //PARED NORTE
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        
        glBegin(GL_QUADS);        
            glVertex3f(-50,0,30);
            glTexCoord2f(8.0, 1.0);
                    
            glVertex3f(-50,5,30);
            glTexCoord2f(0.0, 1.0);
                
            glVertex3f(0,5,30);
            glTexCoord2f(0.0, 0.0);
                  
            glVertex3f(0,0,30);
            glTexCoord2f(8.0, 0.0);                          
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
               
    glPopMatrix();
}
void drawPrism(float x, float z, int orientation){
        
        int vx = -2;             
        int vz = 11;
        
        if (level == 3){ 
           if (orientation == VERTICAL){
                 vz = 13;
           }
           else {
                 vx = -16;
                 vz = 2;
           }
        }
        if(!invincible){
            //CARA OESTE DEL PRISMA RECTANGULAR        
            glBegin(GL_QUADS);        
                glVertex3f(x,0,z+vz);
                glTexCoord2f(8.0, 1.0);
                        
                glVertex3f(x,2,z+vz);
                glTexCoord2f(0.0, 1.0);
                    
                glVertex3f(x,2,z);
                glTexCoord2f(0.0, 0.0);
                      
                glVertex3f(x,0,z);
                glTexCoord2f(8.0, 0.0);                          
            glEnd();
            
            //CARA ESTE DEL PRISMA RECTANGULAR
            glBegin(GL_QUADS);        
                glVertex3f(x+vx,0,z+vz);
                glTexCoord2f(8.0, 1.0);
                        
                glVertex3f(x+vx,2,z+vz);
                glTexCoord2f(0.0, 1.0);
                    
                glVertex3f(x+vx,2,z);
                glTexCoord2f(0.0, 0.0);
                      
                glVertex3f(x+vx,0,z);
                glTexCoord2f(8.0, 0.0);                          
            glEnd();

            //CARA ARRIBA DEL PRISMA RECTANGULAR
            glBegin(GL_QUADS);        
                glVertex3f(x+vx,2,z);
                if (orientation == VERTICAL) glTexCoord2f(0.0, 1.0);
                else glTexCoord2f(8.0, 1.0);
                         
                glVertex3f(x+vx,2,z+vz);
                if (orientation == VERTICAL) glTexCoord2f(0.0, 0.0);
                else glTexCoord2f(0.0, 1.0);         
                                
                glVertex3f(x,2,z+vz);
                if (orientation == VERTICAL) glTexCoord2f(8.0, 0.0);
                else glTexCoord2f(0.0, 0.0);  

                glVertex3f(x,2,z);
                if (orientation == VERTICAL) glTexCoord2f(8.0, 1.0);
                else glTexCoord2f(8.0, 0.0);
                                                      
            glEnd(); 
            
            //CARA SUR DEL PRISMA RECTANGULAR    
            glBegin(GL_QUADS);        
                glVertex3f(x+vx,0,z);
                glTexCoord2f(8.0, 1.0);
                                
                glVertex3f(x+vx,2,z);
                glTexCoord2f(0.0, 1.0);

                glVertex3f(x,2,z);
                glTexCoord2f(0.0, 0.0);
                     
                glVertex3f(x,0,z);
                glTexCoord2f(8.0, 0.0);      
            glEnd();  
                   
            //CARA NORTE DEL PRISMA RECTANGULAR
            glBegin(GL_QUADS);        
                glVertex3f(x+vx,0,z+vz);
                glTexCoord2f(8.0, 1.0);
                        
                glVertex3f(x+vx,2,z+vz);
                glTexCoord2f(0.0, 1.0);
                    
                glVertex3f(x,2,z+vz);
                glTexCoord2f(0.0, 0.0);
                      
                glVertex3f(x,0,z+vz);
                glTexCoord2f(8.0, 0.0);                          
            glEnd();
        } else{//SI SOY INVENCIBLE
            glBegin(GL_QUADS);        
                glVertex3f(x+vx,0.25,z);
                if (orientation == VERTICAL) glTexCoord2f(0.0, 1.0);
                else glTexCoord2f(8.0, 1.0);
                         
                glVertex3f(x+vx,0.25,z+vz);
                if (orientation == VERTICAL) glTexCoord2f(0.0, 0.0);
                else glTexCoord2f(0.0, 1.0);         
                                
                glVertex3f(x,0.25,z+vz);
                if (orientation == VERTICAL) glTexCoord2f(8.0, 0.0);
                else glTexCoord2f(0.0, 0.0);  

                glVertex3f(x,0.25,z);
                if (orientation == VERTICAL) glTexCoord2f(8.0, 1.0);
                else glTexCoord2f(8.0, 0.0);                                   
            glEnd(); 
        }
}    
void drawLevel2(){
    float i;
    glPushMatrix();
        glColor3d(1.0, 1.0, 1.0);

        processTexture("wood.bmp");
        glEnable(GL_TEXTURE_2D);
        
        drawPrism(-11.0, 9.0, VERTICAL);
        drawPrism(-36.0, 9.0, VERTICAL);
        
        glDisable(GL_TEXTURE_2D);        
    glPopMatrix();
}
void drawLevel3(){
    float i;
    glPushMatrix();
        glColor3d(1.0, 1.0, 1.0);
        
        processTexture("wood.bmp");
        glEnable(GL_TEXTURE_2D);
        
        drawPrism(-11.0, 4.0, VERTICAL);
        drawPrism(-36.0, 12.0, VERTICAL);
        
        drawPrism(-4.0, 23.0, HORIZONTAL);
        drawPrism(-29.0, 5.0, HORIZONTAL);
        
        glDisable(GL_TEXTURE_2D); 
  
    glPopMatrix();
}
int checkForHighScore(){
    FILE* topRanking = fopen("ranking","r");
    fscanf(topRanking, "%s %d\n%s %d\n%s %d", top3[0].name,&top3[0].points,top3[1].name,&top3[1].points,top3[2].name,&top3[2].points);
    fclose(topRanking);
    if(score >= top3[0].points || score >= top3[1].points || score >= top3[2].points)
        return 1;
    return 0;
}
void saveNewScores(){
    FILE* topRanking = fopen("ranking","r");
    fscanf(topRanking, "%s %s\n%s %s\n%s %s", top3[0].name,top3[0].score,top3[1].name,top3[1].score,top3[2].name,top3[2].score);
    fclose(topRanking);
    newName[strlen(newName)] = 0;
    if(score >= top3[0].points){
        top3[2].points = top3[1].points;
        top3[1].points = top3[0].points;
        top3[0].points = score;
        strcpy(top3[2].score,top3[1].score);
        strcpy(top3[1].score,top3[0].score);
        snprintf(top3[0].score,10,"%d",score);
        strcpy(top3[2].name,top3[1].name);
        strcpy(top3[1].name,top3[0].name);
        strcpy(top3[0].name,newName);
    } else if (score >= top3[1].points && score < top3[0].points){
        top3[2].points = top3[1].points;
        top3[1].points = score;
        strcpy(top3[2].score,top3[1].score);
        snprintf(top3[1].score,10,"%d",score);
        strcpy(top3[2].name,top3[1].name);
        strcpy(top3[1].name,newName);
    } else if (score >= top3[2].points && score < top3[1].points){
        top3[2].points = score;
        snprintf(top3[2].score,10,"%d",score);
        strcpy(top3[2].name,newName);
    }
    topRanking = fopen("ranking","w");
    fprintf(topRanking, "%s %d\n%s %d\n%s %d", top3[0].name,top3[0].points,top3[1].name,top3[1].points,top3[2].name,top3[2].points);
    fclose(topRanking);
    score = 0;
}
void displayMenu(){
    char newGameOption[20],topOption[20],exitOption[20];
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 100, 0.0, 100);    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);   

    glColor3f(0.2,0.4,0.3);
    glRasterPos2i(45,90);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "CYBER COBRA 3D");     
    
    glColor3f(1.0, 1.0, 1.0); 
    glRasterPos2i(44, 78);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "*- Menu Principal -*");
    if(menuOption == NEW_GAME){
        memset(newGameOption,0,sizeof(newGameOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(newGameOption,"- Nuevo Juego -");
    }
    else{
        memset(newGameOption,0,sizeof(newGameOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(newGameOption,"Nuevo Juego");
    }
    glRasterPos2i(45, 70);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)newGameOption);
    if(menuOption == TOP){
        memset(topOption,0,sizeof(topOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(topOption,"- TOP 3 -");
    }
    else{
        memset(topOption,0,sizeof(topOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(topOption,"TOP 3");
    }
    glRasterPos2i(45, 65);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)topOption);
    if(menuOption == EXIT){
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(exitOption,"- Salir -");
    }   
    else{
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(exitOption,"Salir");
    }
    glRasterPos2i(45, 61);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)exitOption);

    glColor3f(0.7, 0.0, 0.0);    
    glRasterPos2i(43, 57);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Instrucciones de Juego");
    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2i(24, 53);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Debes guiar a la serpiente para atrapar las frutas que se colocan en el tablero evitando");
    glRasterPos2i(24, 49);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "colisionar con los obstaculos o las paredes.");
    glRasterPos2i(23, 45);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Hay tres visuales distintas que se pueden activar y cambiar con las teclas 1, 2 y 3:");
    glRasterPos2i(23, 41);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "1 -> Vista en 3era Persona: para moverse se utilizan las teclas derecha e izquierda.");
    glRasterPos2i(23, 37);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "2 -> Vista en 1era Persona: para moverse se utilizan las teclas derecha e izquierda.");
    glRasterPos2i(23, 33);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "3 -> Vista Aerea: para moverse se utilizan las teclas derecha, izquierda, arriba y abajo.");
    glRasterPos2i(20, 29);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Hay 5 tipos de 'frutas': Blanca(25p)-Amarilla(10p)-Verde(100p)-Roja(Invencibilidad)-Azul(Lentitud).");
    glRasterPos2i(20, 25);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Debes apretar la tecla '0' para activar el poder de lentitud cuando lo desees.");
    glRasterPos2i(20, 21);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "En total el juego tiene 3 niveles. 0p < lvl1 < 125p < lvl2 < 250p < lvl3.");
    glRasterPos2i(20, 17);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Atencion! Cada 15 segundos la velocidad de la serpiente aumenta en un factor de 1.1x.");

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
    glMatrixMode(GL_MODELVIEW); 
}
void displayData(){
    char scoreBuffer[10], invisibleTimeBuffer[3], slowerTimeBuffer[3], timeBuffer[10];
    char scoreString[20] = "Puntaje = ";
    char invisibleTimeString[20] = "Invencible = "; 
    char slowerTimeString[20] = "Lentitud = "; 
    char timeString[20] = "Tiempo = ";
    int invisibleTime, slowerTime;
    char speedBuffer[4];
            
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 100, 0.0, 100);    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);        
    glColor3f(1.0, 1.0, 1.0); 
    
    //Tiempo de juego
    glRasterPos2i(2, 95);
    snprintf(timeBuffer,20,"%d",(int)gameTime);
    strcat(timeString, timeBuffer);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)timeString);                                                         
    
    //NIVELES
    if(level == 1){
        glRasterPos2i(48, 95);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "NIVEL 1"); 
    } else if (level == 2){
        glRasterPos2i(48, 95);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "NIVEL 2"); 
    }else if (level == 3){
        glRasterPos2i(48, 95);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "NIVEL 3"); 
    }
    //Puntaje
    glRasterPos2i(89, 12);
    snprintf(scoreBuffer,20,"%d",score);
    strcat(scoreString, scoreBuffer);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)scoreString);                                                         
    
    //Tiempo de invisibilidad
    if(invincible){
        glRasterPos2i(89, 7);
        invisibleTime = 11 - (gameTime - invincibleTime);
        snprintf(invisibleTimeBuffer,20,"%d",invisibleTime);
        strcat(invisibleTimeString, invisibleTimeBuffer);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)invisibleTimeString);           
    }
    
    //Tiempo de disminicion de velocidad
    if(slowingPower){
        glRasterPos2i(89, 2);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Lentitud -> '0'");
    }
    if (slower){
        glRasterPos2i(89, 2);
        slowerTime = 6 - (gameTime - slowingTime);
        snprintf(slowerTimeBuffer,20,"%d",slowerTime);
        strcat(slowerTimeString, slowerTimeBuffer);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)slowerTimeString);        
    }

    //VELOCIDAD
    // glRasterPos2i(2, 86);
    // snprintf(speedBuffer,4,"%d",speed);
    // glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)speedBuffer);
    if (gameTime - accelerateTime <= 2 && gameTime > 2){
        glColor3f(0.5, 0.0, 0.0);
        glRasterPos2i(2, 90);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Speed 1.1x!!!"); 
    }

    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(2, 2);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Salir del Juego -> ESC");
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(2, 6);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Pausar el Juego -> p");

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
    glMatrixMode(GL_MODELVIEW);     
}
void displayRanking(){
    char returnOption[20],exitOption[20];
    if(checkForHighScore())
        saveNewScores();
    char places[30];
    FILE* topRanking = fopen("ranking","r");
    fscanf(topRanking, "%s %s\n%s %s\n%s %s", top3[0].name,top3[0].score,top3[1].name,top3[1].score,top3[2].name,top3[2].score);
    fclose(topRanking);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 100, 0.0, 100);    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);        
    glColor3f(1.0, 1.0, 1.0); 

    glRasterPos2i(44, 65);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "*- Los Mejores 3 -*");
    //PRIMER LUGAR
    strcpy(places, "1- ");
    strcat(places, top3[0].name);
    strcat(places, " -> ");
    strcat(places, top3[0].score);
    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2i(44, 60);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)places);
    //SEGUNDO LUGAR
    strcpy(places, "2- ");
    strcat(places, top3[1].name);
    strcat(places, " -> ");
    strcat(places, top3[1].score);
    glRasterPos2i(44, 50);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)places);
    //TERCER LUGAR
    strcpy(places, "3- ");
    strcat(places, top3[2].name);
    strcat(places, " -> ");
    strcat(places, top3[2].score);
    glRasterPos2i(44, 40);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)places);

    if(menuOption == RETURN_MENU){
        memset(returnOption,0,sizeof(returnOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(returnOption,"*- Regresar al Menu -*");
    }
    else{
        memset(returnOption,0,sizeof(returnOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(returnOption,"Regresar al Menu");
    }
    glRasterPos2i(20, 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)returnOption);
    
    if(menuOption == EXIT){
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(exitOption,"*- Salir del Juego -*");
        
    }
    else{
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(exitOption,"Salir del Juego");
    }
    glRasterPos2i(70, 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)exitOption);

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
    glMatrixMode(GL_MODELVIEW); 
}
void displayGameOver(){
    char returnOption[20],exitOption[20];
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 100, 0.0, 100);    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);        
    glColor3f(1.0, 1.0, 1.0); 

    glRasterPos2i(42, 50);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "X_X GAME OVER X_X");

    if(menuOption == RETURN_MENU){
        memset(returnOption,0,sizeof(returnOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(returnOption,"*- Regresar al Menu -*");
    }
    else{
        memset(returnOption,0,sizeof(returnOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(returnOption,"Regresar al Menu");
    }
    glRasterPos2i(20, 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)returnOption);
    
    if(menuOption == EXIT){
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.0, 0.0, 0.5);
        strcpy(exitOption,"*- Salir del Juego -*");
        
    }
    else{
        memset(exitOption,0,sizeof(exitOption));
        glColor3f(0.9, 0.9, 0.9);
        strcpy(exitOption,"Salir del Juego");
    }
    glRasterPos2i(70, 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)exitOption);

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
    glMatrixMode(GL_MODELVIEW); 
}
void displayInputName(){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 100, 0.0, 100);    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);        
    
    glColor3f(1.0, 1.0, 0.0); 
    glRasterPos2i(30, 55);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Enhorabuena!!! Has logrado ingresar al prestigioso TOP3!!!");
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(30, 50);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "Introduzca un nombre de maximo 10 caracteres, al terminar presione Enter.");

    glColor3f(0.0, 0.0, 0.3); 
    glRasterPos2i(45, 46);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)newName);

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
    glMatrixMode(GL_MODELVIEW); 
}
static void display(void){
    char tiempo[20];
    elapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;      //tiempo transcurrido desde la creacion de la ventana                     
    if(!paused)
        gameTime = elapsedTime - pausedTime;
    glLoadIdentity();
    if(view == NORMAL)
        gluLookAt(x+dx, cameraHeight, z+dz, x+(-dx/2), 0, z+(-dz/2), 0, 1, 0 );
    else if(view == SNAKE)
        gluLookAt(x, cameraHeight, z, x+(-dx/2), cameraHeight, z+(-dz/2), 0, 1, 0);
    else if(view == UPPER)
        gluLookAt(-25, cameraHeight, -5, -25, 0, 15, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    if (screen == MENU){
        cameraHeight = UPPER_VIEW;
        glLoadIdentity() ;
        gluLookAt(-25, cameraHeight, -5, -25, 0, 15, 0, 1, 0);
        drawField();
        displayMenu();
    }
    if (screen == MAIN_GAME){
        //SPEED CHANGE
        if((int)gameTime % 15 == 0 && (int)gameTime > 2){
            if(changeSpeed){
                system("aplay speed.wav&");
                accelerateTime = (glutGet(GLUT_ELAPSED_TIME) / 1000) - pausedTime;
                speed = speed / SPEED_FACTOR;
                changeSpeed = 0;
            }
        } else {
            changeSpeed = 1;
        }
        // Campo de Juego
        drawField();
        displayData();
        if(invincible)
            if(gameTime - invincibleTime >= 10.0){
                invincible = 0;
            }
        if(slower)
            if(gameTime - slowingTime >= 5.0){
                speed = speedAux;
                slower = 0;
            }
        if (level == 2)
            drawLevel2();
        if (level == 3)
            drawLevel3();
        //Generar el cuerpo de la Serpiente
        drawSnake();
        //Generar fruta
        drawFruit();
    } 
    if (screen == GAME_OVER){
        cameraHeight = UPPER_VIEW;
        glLoadIdentity() ;
        gluLookAt(-25, cameraHeight, -5, -25, 0, 15, 0, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawField();
        if (checkForHighScore())
            screen = NEW_HIGHSCORE; 
        displayGameOver();
    }   
    if (screen == RANKING){
        cameraHeight = UPPER_VIEW;
        glLoadIdentity() ;
        gluLookAt(-25, cameraHeight, -5, -25, 0, 15, 0, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawField();
        displayRanking();
    }
    if (screen == NEW_HIGHSCORE){
        cameraHeight = UPPER_VIEW;
        glLoadIdentity() ;
        gluLookAt(-25, cameraHeight, -5, -25, 0, 15, 0, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawField();
        // system("aplay cheering.wav");
        displayInputName();
    }
    glutSwapBuffers();
}
//Procedimiento para inicializar un nuevo juego, setea todas las variables necesarias para un nuevo juego.
void newGame(){
    speed = BASE_SPEED;
    fromCenterLvl2 = 1;
    fromCenterLvl3 = 1;
    level = 1;
    score = 0;
    bodyParts = 5;
    direction = NORTH; 
    dx =  0.0;
    view = UPPER;
    dz =  -NORMAL_DZ;
    x = -25.0;
    z = 10.0;
    firstSnake();
    invincible = 0;
    slower = 0;
    slowingPower = 0;
    paused = 0;
    memset(newName, 0, sizeof(newName));
    cameraHeight = UPPER_VIEW;
    glutTimerFunc(0, timer, 0);
    changeSpeed = 1;
    accelerateTime = 0.0;
    pausedTime = glutGet(GLUT_ELAPSED_TIME) / 1000;
    screen = MAIN_GAME;
}
static void pressKey(int key, int xx, int yy) {
    if(screen == MENU){
        switch(key){
            case GLUT_KEY_DOWN:
                if(menuOption == NEW_GAME)
                    menuOption = TOP;
                else if(menuOption == TOP)
                    menuOption = EXIT;
                else if(menuOption == EXIT)
                    menuOption = NEW_GAME;
                break;
            case GLUT_KEY_UP:
                if(menuOption == NEW_GAME)
                    menuOption = EXIT;
                else if(menuOption == TOP)
                    menuOption = NEW_GAME;
                else if(menuOption == EXIT)
                    menuOption = TOP;
                break;
        }
    }
    else if(screen == RANKING || screen == GAME_OVER){
        switch(key){
            case GLUT_KEY_RIGHT:
                if(menuOption == RETURN_MENU)
                    menuOption = EXIT;
                else if(menuOption == EXIT)
                    menuOption = RETURN_MENU;
                break;
            case GLUT_KEY_LEFT:
                if(menuOption == RETURN_MENU)
                    menuOption = EXIT;
                else if(menuOption == EXIT)
                    menuOption = RETURN_MENU;
                break;
        }
    }
    else if(screen == MAIN_GAME){
        if(!paused){
            if(view != UPPER){
                switch (key) {
                    case GLUT_KEY_LEFT : 
                        if (direction == NORTH){
                            if(view == NORMAL)
                                dx =  -NORMAL_DX;
                            else
                                dx = -SNAKE_DX; 
                            dz = 0.0;
                            direction = WEST;
                        }
                        else if (direction == WEST){
                            dx = 0.0;
                            if(view == NORMAL)
                                dz =  NORMAL_DZ;
                            else
                                dz = SNAKE_DZ;
                            direction = SOUTH;
                        }
                        else if (direction == SOUTH){
                            if(view == NORMAL)
                                dx =  NORMAL_DX;
                            else
                                dx = SNAKE_DX;
                            dz = 0.0;
                            direction = EAST;
                        }
                        else if (direction == EAST){
                            dx = 0.0;
                            if(view == NORMAL)
                                dz =  -NORMAL_DZ;
                            else
                                dz = -SNAKE_DZ;
                            direction = NORTH;
                        }
                        break;
                    case GLUT_KEY_RIGHT : 
                        if (direction == NORTH){
                            if(view == NORMAL)
                                dx =  NORMAL_DX;
                            else
                                dx = SNAKE_DX;
                            dz = 0.0;
                            direction = EAST;
                        }
                        else if (direction == WEST){
                            dx = 0.0;
                            if(view == NORMAL)
                                dz =  -NORMAL_DZ;
                            else
                                dz = -SNAKE_DZ;
                            direction = NORTH;
                        }
                        else if (direction == SOUTH){
                            if(view == NORMAL)
                                dx =  -NORMAL_DX;
                            else
                                dx = -SNAKE_DX;
                            dz = 0.0;
                            direction = WEST;
                        }
                        else if (direction == EAST){
                            dx = 0.0;
                            if(view == NORMAL)
                                dz =  NORMAL_DZ;
                            else
                                dz = SNAKE_DZ;
                            direction = SOUTH;
                        }   
                        break;
                }
            } else{
                switch (key) {
                    case GLUT_KEY_LEFT : 
                        if(direction != EAST && body[0].z_pos != body[1].z_pos)
                            direction = WEST;
                        break;
                    case GLUT_KEY_RIGHT : 
                        if(direction != WEST && body[0].z_pos != body[1].z_pos)
                            direction = EAST;
                        break;
                    case GLUT_KEY_UP:
                        if(direction != SOUTH && body[0].x_pos != body[1].x_pos)
                            direction = NORTH;
                        break;
                    case GLUT_KEY_DOWN:
                        if(direction != NORTH && body[0].x_pos != body[1].x_pos)
                            direction = SOUTH;
                        break;
                }
            }
        }    
    }
    
}
static void key(unsigned char key, int x, int y){
    static int position = 0;
    int i;
    if(screen != NEW_HIGHSCORE){
        switch (key) 
        {
            case 27:
            case 'q':
                system("killall aplay");
                exit(0);
                break;
            case 13:
                if(screen == MENU || screen == RANKING || screen == GAME_OVER){
                    if(menuOption == NEW_GAME)
                        newGame();
                    else if(menuOption == TOP){
                        menuOption = RETURN_MENU;
                        screen = RANKING;
                    }
                    else if(menuOption == EXIT){
                        system("killall aplay");
                        exit(0);  
                    }
                    else if (menuOption == RETURN_MENU){
                        menuOption = NEW_GAME;
                        screen = MENU;
                    }
                }
                break;
            case 'm':
                if(screen != MAIN_GAME)
                    screen = MENU;
                break;
            case 'n':
                if(screen != MAIN_GAME)
                    newGame();
                break; 
            case 'r':
                if(screen != MAIN_GAME)
                    screen = RANKING;
                break;
            case 'p':
                if(!paused){
                    startPauseTime = glutGet(GLUT_ELAPSED_TIME) / 1000;
                    paused = 1;
                }
                else{
                    pausedTime += glutGet(GLUT_ELAPSED_TIME) / 1000 - startPauseTime; 
                    paused = 0;
                    glutTimerFunc(0, timer, 0);
                }
                break;
            case '0':
                if(slowingPower == 1){
                    speedAux = speed;
                    speed = speed + (speed*SLOW_FACTOR);
                    slowingTime = (glutGet(GLUT_ELAPSED_TIME)/ 1000.0) - pausedTime;
                    slower = 1;
                    slowingPower = 0;
                }
                break; 
            case '1': // CAMBIA A LA VISTA NORMAL
                if(direction == SOUTH){
                    dx = 0.0;
                    dz =  NORMAL_DZ;
                }
                else if(direction == NORTH){
                    dx = 0.0;
                    dz = -NORMAL_DZ;
                }
                else if(direction == WEST){
                    dx = -NORMAL_DX;
                    dz = 0.0;
                }
                else if(direction == EAST){
                    dx = NORMAL_DX;
                    dz = 0.0;
                }
                view = NORMAL;
                cameraHeight = NORMAL_VIEW;
                break;
            case '2': //CAMBIA A LA VISTA PRIMERA PERSONA DESDE LA CABEZA DE LA SERPIENTE
                if(direction == SOUTH){
                    dx = 0.0;
                    dz =  SNAKE_DZ;
                }
                else if(direction == NORTH){
                    dx = 0.0;
                    dz = -SNAKE_DZ;
                }
                else if(direction == WEST){
                    dx = -SNAKE_DX;
                    dz = 0.0;
                }
                else if(direction == EAST){
                    dx = SNAKE_DX;
                    dz = 0.0;
                }
                view = SNAKE;
                cameraHeight = SNAKE_VIEW;
                break;
            case '3': // CAMBIA A LA VISTA AEREA DE 4 DIRECCIONES
                view = UPPER;
                cameraHeight = UPPER_VIEW;
                break;
        }
    }
    else if (screen == NEW_HIGHSCORE){
        if (position == 10){
            position = 0;
            screen = RANKING;
        }
        else if (key == 13){
            position = 0;
            screen = RANKING;
        }
        else if (key == 8 && position > 0){
            newName[--position] = 0;
        }
        else if ((key >= 48 && key <= 57) || (key >= 65 && key <= 90) || (key >= 97 && key <= 122))
            newName[position++] = (char)key;
    }
    glutPostRedisplay();
}
static void idle(void)
{
    glutPostRedisplay();
}

/* Program entry point */
void playMusic (int value) {
    system("aplay bomberman.wav &");
    glutTimerFunc(67000, playMusic, 0);
}
int main(int argc, char *argv[]){
    glutInit(&argc, argv);    //Abstrae todo lo necesario para crear la ventana
    glutInitWindowSize(1366,768);                                
    glutInitWindowPosition(10,10);                              
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);   

    glutCreateWindow("Snake 3D");
    glutFullScreen();
    srand(time(NULL));
    //Musica
    glutTimerFunc(0, playMusic, 0);
    glutReshapeFunc(resize);    //Posicionar la camara para empezar el dibujo
    glutDisplayFunc(display);   //Area donde pongo objetos para mostrar a la camara


    glutKeyboardFunc(key);      //Acciones para objetos del display
    glutSpecialFunc(pressKey);

    glutIdleFunc(idle);         

    glClearColor(0.3921,0.5843,0.9294,1);      //color de fondo
    
    //efecto de mejora de rendereo
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //dibua ultimo el que esta mas al fondo
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //tecnicas de luces de ambiente
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    //Manejo de luces (No necesario para el trabajo practico)
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glutMainLoop();

    return EXIT_SUCCESS;
}

