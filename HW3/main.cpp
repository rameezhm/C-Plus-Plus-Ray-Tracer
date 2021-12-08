#include <stdlib.h>
#include <iostream>
#include <FreeImage.h>
#include <limits>
// OSX systems need their own headers
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
// Use of degrees is deprecated. Use radians for GLM functions
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include "Scene.h"
#include "Ray.h"
#include "Intersection.h"
#include "Triangle.h"


static const int width = 160;
static const int height = 120;
static const char* title = "Scene viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);
static Scene scene;

#include "hw3AutoScreenshots.h"

void printHelp(){
    std::cout << R"(
    Available commands:
      press 'H' to print this message again.
      press Esc to quit.
      press 'O' to save a screenshot.
      press the arrow keys to rotate camera.
      press 'A'/'Z' to zoom.
      press 'R' to reset camera.
      press 'L' to turn on/off the lighting.
    
      press Spacebar to generate images for hw3 submission.
    
)";
}

void initialize(void){
    printHelp();
    glClearColor(background[0], background[1], background[2], background[3]); // background color
    glViewport(0,0,width,height);
    
    // Initialize scene
    scene.init();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    scene.draw();
    
    glutSwapBuffers();
    glFlush();
    
}

void saveScreenShot(const char* filename = "test.png"){
    int currentwidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentheight = glutGet(GLUT_WINDOW_HEIGHT);
    Screenshot imag = Screenshot(currentwidth,currentheight);
    imag.save(filename);
}

void keyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: // Escape to quit
            exit(0);
            break;
        case 'h': // print help
            printHelp();
            break;
        case 'o': // save screenshot
            saveScreenShot();
            break;
        case 'r':
            scene.camera -> aspect_default = float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT));
            scene.camera -> reset();
            glutPostRedisplay();
            break;
        case 'a':
            scene.camera -> zoom(0.9f);
            glutPostRedisplay();
            break;
        case 'z':
            scene.camera -> zoom(1.1f);
            glutPostRedisplay();
            break;
        case 'l':
            scene.shader -> enablelighting = !(scene.shader -> enablelighting);
            glutPostRedisplay();
            break;
        case ' ':
            hw3AutoScreenshots();
            glutPostRedisplay();
            break;
        default:
            glutPostRedisplay();
            break;
    }
}
void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP: // up
            scene.camera -> rotateUp(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: // down
            scene.camera -> rotateUp(10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT: // right
            scene.camera -> rotateRight(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT: // left
            scene.camera -> rotateRight(10.0f);
            glutPostRedisplay();
            break;
    }
}

void saveimg(std::vector<BYTE> pixels, const char* filename) {

    // You need to get your image into the pixel vector.  How you do so is up to you.
    // Also, make sure you follow the directions in the writeup, and call FreeImage_Initialise() before using this function.

    FIBITMAP* img = FreeImage_ConvertFromRawBits(pixels.data(), width, height, width * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);

    std::cout << "Saving screenshot: " << filename << std::endl;

    FreeImage_Save(FIF_PNG, img, filename, 0);
}

Ray rayThruPixel(Camera* cam, int i, int j) {
    float a = 2.f * (float(i) + 0.5) / float(width) - 1; // alpha
    float b = 1.f - 2.f * (float(j) + 0.5) / float(height); //beta

    glm::vec3 w = normalize(cam->eye - cam->target);
    glm::vec3 u = normalize(cross(cam->up, w));
    glm::vec3 v = cross(w, u);

    glm::vec3 dir = normalize(a * u + b * v - w);
    return Ray(cam->eye, dir);
}

Intersection intersect(Ray* ray, Triangle* tri) {
    float pd = dot(tri->n, tri->p1);
    float t = -(dot(tri->n, ray->origin) + pd) / dot(tri->n, ray->dir);
    return Intersection(ray, tri, t);
}


// find closest intersection in scene
Intersection intersect(Ray* ray) {
    float mindist = std::numeric_limits<float>::infinity();
    Intersection hit;
    for each (Triangle* obj in scene) {
        Intersection hit_temp = intersect(ray, obj);
        if (hit_temp.dis < mindist) {
            // TODO: handle negative distance
            mindist = hit_temp.dis;
            hit = hit_temp;
        }
    }
    return hit;
}

//generate rays to light sources from a given point
std::vector<Ray> genRaysToLights(glm::vec3 pt) {

}

//generates color from shading model
glm::vec3 shadingModel(Intersection* hit) {

}

std::vector<Ray> genMirrorRays() {

}

glm::vec3 findColor(Intersection* hit, int depth) {
    if (depth < 3) { // limit on recursion depth
        std::vector<Ray> toLights = genRaysToLights(hit->poi());
        glm::vec3 color = shadingModel(hit);

        // recurse on mirrored rays of light
        std::vector<Ray> mirror = genMirrorRays();
        for each (Ray m in mirror) {
            Intersection hit2 = intersect(&m);
            color = color + findColor(&hit2, depth + 1);
        }
        
        return color;
    } else {
        return glm::vec3(0, 0, 0);
    }
}

std::vector<BYTE> raytrace() {
    std::vector<BYTE> image;
    Camera* cam = scene.camera;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            Ray ray = rayThruPixel(cam, i, j);
            Intersection hit = intersect(&ray);
            glm::vec3 color = findColor(&hit);

            image.push_back(int(color.x * 255));
            image.push_back(int(color.y * 255));
            image.push_back(int(color.z * 255));
        }
    }
    return image;
}



int main(int argc, char** argv)
{
    /*


    // BEGIN CREATE WINDOW
    glutInit(&argc, argv);
    
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
#endif
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    // END CREATE WINDOW
    
    initialize();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    
    glutMainLoop();

    */
    FreeImage_Initialise();
    std::vector<BYTE> img = raytrace();
    saveimg(img, "output.png");


	return 0;   /* ANSI C requires main to return int. */
}
