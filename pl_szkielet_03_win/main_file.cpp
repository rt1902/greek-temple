#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <lodepng.h>
#include "constants.h"
#include "objloader.hpp"
#include <iostream>

using namespace glm;
using namespace std;

glm::vec3 cameraPos   = glm::vec3(0.0f, 3.0f,  40.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 12.2f;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last fra
float lastX = 700, lastY = 350;
float yaw1, pitch1;
bool firstMouse = true;
float aspect=1.7; //Aktualny stosunek szerokości do wysokości okna
GLuint vertexbuffer[3]; //0 - temple, 1 - ground, 2 - background
GLuint uvbuffer[3];
GLuint normalsbuffer[3];
GLuint texture[3]; //0 - marble; 1 - rock ground, 2 - background
struct obj {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> vertex_normals;
};
unsigned short obj_count=3;
struct obj obj_temple;
struct obj obj_ground;
struct obj obj_background;

float rgba_ambient[]={1,1,1,1};
float rgba_diffuse[]={1,1,1,1};
float rgba_statue_ambient[]={0.95,0.9,0.8,1};
float rgba_statue_diffuse[]={0.8,0.8,0.8,1};
float rgba_specular[]={0,0,0,1};
float rgba_emission[]={0,0,0,1};
float rgba_sun_emission[]={1,1,1,1};
unsigned short shininess=0;

float sun_light_pos[]={15,140,160,1};
float temple_light_pos[]={8,8,-10,1};
float sun_light_ambient[]={0.4,0.4,0.4,1};
float sun_light_diffuse[]={0.5,0.5,0.5,1};
float temple_light_ambient[]={0.1,0.1,0.1,1};
float temple_light_diffuse[]={0.3,0.3,0.3,1};
float light_specular[]={1,1,1,1};
//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obługi zmiany rozmiaru bufora ramki
void windowResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); //Obraz ma być generowany w oknie o tej rozdzielczości
    if (height!=0) {
	aspect=(float)width/(float)height; //Stosunek szerokości do wysokości okna
    } else {
	    aspect=1;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw1   += xoffset;
    pitch1 += yoffset;

    if(pitch1 > 89.0f)
        pitch1 = 89.0f;
    if(pitch1 < -89.0f)
        pitch1 = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch1));
    front.y = sin(glm::radians(pitch1));
    front.z = sin(glm::radians(yaw1)) * cos(glm::radians(pitch1));
    cameraFront = glm::normalize(front);
}
//Procedura obsługi klawiatury
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
    glfwSetFramebufferSizeCallback(window, windowResize); //Zarejestruj procedurę obsługi zmiany rozdzielczości bufora ramki
    glfwSetKeyCallback(window, key_callback); //Zarejestruj procedurę obsługi klawiatury
    glfwSetCursorPosCallback(window, mouse_callback);

    glClearColor(0.34, 0.8, 1, 1); //Ustaw kolor czyszczenia ekranu

    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rgba_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rgba_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rgba_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, rgba_emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glGenBuffers(obj_count, vertexbuffer);
    glGenBuffers(obj_count, normalsbuffer);
	glGenBuffers(obj_count, uvbuffer);

    loadOBJ("./objects/temple.obj", obj_temple.vertices, obj_temple.uvs, obj_temple.normals);
    loadOBJ("./objects/ground.obj", obj_ground.vertices, obj_ground.uvs, obj_ground.normals);
    loadOBJ("./objects/background.obj", obj_background.vertices, obj_background.uvs, obj_background.normals);

    std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
    unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
    unsigned error = lodepng::decode(image, width, height, "./textures/marble_texture.png");

    glGenTextures(3,texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_temple.vertices.size() * sizeof(glm::vec3), &obj_temple.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_temple.normals.size() * sizeof(glm::vec3), &obj_temple.normals[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_temple.uvs.size() * sizeof(glm::vec2), &obj_temple.uvs[0], GL_STATIC_DRAW);

    std::vector<unsigned char> image2;   //Alokuj wektor do wczytania obrazka
    unsigned width2, height2;   //Zmienne do których wczytamy wymiary obrazka
    unsigned error2 = lodepng::decode(image2, width2, height2, "./textures/ground_texture2.png");

    glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image2.data());

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_ground.vertices.size() * sizeof(glm::vec3), &obj_ground.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_ground.normals.size() * sizeof(glm::vec3), &obj_ground.normals[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_ground.uvs.size() * sizeof(glm::vec2), &obj_ground.uvs[0], GL_STATIC_DRAW);

    std::vector<unsigned char> image4;   //Alokuj wektor do wczytania obrazka
    unsigned width4, height4;   //Zmienne do których wczytamy wymiary obrazka
    unsigned error4 = lodepng::decode(image4, width4, height4, "./textures/background.png");
    glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width4, height4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image4.data());

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, obj_background.vertices.size() * sizeof(glm::vec3), &obj_background.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, obj_background.normals.size() * sizeof(glm::vec3), &obj_background.normals[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, obj_background.uvs.size() * sizeof(glm::vec2), &obj_background.uvs[0], GL_STATIC_DRAW);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float angle_x,float angle_y) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
    mat4 P=perspective(65.0f*PI/180.0f,aspect,1.0f,4400.0f); //Wylicz macierz rzutowania P

    cameraPos.y = 13.0f; //postac na 1 poziomie

    mat4 V=lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    mat4 M=mat4(1);
    glMatrixMode(GL_PROJECTION); //Włącz tryb modyfikacji macierzy rzutowania
    glLoadMatrixf(value_ptr(P)); //Załaduj macierz rzutowania
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wyczyść bufor kolorów (czyli przygotuj "płótno" do rysowania)

    glMatrixMode(GL_MODELVIEW);
    M=rotate(M,angle_x,vec3(1.0f,0.0f,0.0f));
    M=rotate(M,angle_y,vec3(0.0f,1.0f,0.0f));
    glLoadMatrixf(value_ptr(V*M));

    glLightfv(GL_LIGHT0,GL_AMBIENT,sun_light_ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,sun_light_diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT0,GL_POSITION,sun_light_pos);

    glLightfv(GL_LIGHT1,GL_AMBIENT,temple_light_ambient);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,temple_light_diffuse);
    glLightfv(GL_LIGHT1,GL_SPECULAR,light_specular);
    glLightfv(GL_LIGHT1,GL_POSITION,temple_light_pos);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    glVertexPointer(3,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[0]);
    glNormalPointer(GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
    glTexCoordPointer(2,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES,0,obj_temple.vertices.size());

    glBindTexture(GL_TEXTURE_2D,texture[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glVertexPointer(3,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[1]);
    glNormalPointer(GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[1]);
    glTexCoordPointer(2,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES,0,obj_ground.vertices.size());

    glBindTexture(GL_TEXTURE_2D,texture[2]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, rgba_sun_emission);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[2]);
    glVertexPointer(3,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer[2]);
    glNormalPointer(GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[2]);
    glTexCoordPointer(2,GL_FLOAT,0,NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES,0,obj_background.vertices.size());
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, rgba_emission);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glfwSwapBuffers(window);
}

int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno
	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów
	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}
	window = glfwCreateWindow(1920, 1080, "OpenGL", NULL, NULL);
	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora
	GLenum err;
	if ((err=glewInit()) != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	initOpenGLProgram(window); //Operacje inicjujące
	float angle_x=0.0f; //Aktualny kąt obrotu obiektu wokół osi x
	float angle_y=0.0f; //Aktualny kąt obrotu obiektu wokół osi y
	glfwSetTime(0); //Wyzeruj timer
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Główna pętla
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
	    float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float cameraSpeed = 0.1f * deltaTime;
	    glfwSetTime(0); //Wyzeruj timer
		drawScene(window,angle_x,angle_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}
	glDeleteBuffers(obj_count, vertexbuffer);
	glDeleteBuffers(obj_count, normalsbuffer);
	glDeleteTextures(3,texture);
	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
