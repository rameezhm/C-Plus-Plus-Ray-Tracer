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
#include <Obj.h>

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
            //scene.shader -> enablelighting = !(scene.shader -> enablelighting);
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

// ---------------- INTERSECTION FUNCTIONS ----------------------------------------------------------------------------

// compute intersection between ray and triangle
Intersection intersect(Ray* ray, Triangle* tri) {
    float pd = dot(tri->n, tri->p1);
    float t = -(dot(tri->n, ray->origin) + pd) / dot(tri->n, ray->dir);

    // is intersection inside triangle
    //edges
    glm::vec3 e1 = tri->p2 - tri->p1;
    glm::vec3 e2 = tri->p3 - tri->p2;
    glm::vec3 e3 = tri->p1 - tri->p3;

    glm::vec3 pt = ray->origin + t * ray->dir;

    glm::vec3 c1 = cross(e1, pt - tri->p1);
    glm::vec3 c2 = cross(e2, pt - tri->p2);
    glm::vec3 c3 = cross(e3, pt - tri->p3);

    float val1 = dot(tri->n, cross(e1, c1));
    float val2 = dot(tri->n, cross(e2, c2));
    float val3 = dot(tri->n, cross(e3, c3));

    bool inside = val1 > 0 &&  val2 > 0 &&  val3 > 0;

    return Intersection(ray, tri, t, inside);
}


// find closest intersection in scene
Intersection intersect(Ray* ray) {
    float mindist = std::numeric_limits<float>::infinity();
    Intersection hit;

    for each (Triangle obj in scene.triList) { 
        Intersection hit_temp = intersect(ray, &obj);
        //intersection must be in triangle and not have negative distance
        if (hit_temp.dis < mindist && hit_temp.in && hit_temp.dis > 0) {
            mindist = hit_temp.dis;
            hit = hit_temp;
        }
    }
    return hit;
}

// ------------ COLORING FUNCTIONS ------------------------------------------------------------------------------------

//generate rays to light sources from a given point
std::vector<Ray> genRaysToLights(Ray* in) {
    glm::vec3 pt = in->origin;
    std::vector<Ray> rays;
    for (auto& l: scene.light) {
        glm::vec3 lightPos = glm::vec3(l.second->position.x, l.second->position.y, l.second->position.z);
        // origin of new ray is slightly elevated to avoid intersection with current object
        rays.push_back(Ray(pt + (in->dir) * -0.1f, normalize( lightPos - pt) ) ); 
    }
    return rays;
}

//gets lights corresponding to the rays in genRaysToLights
std::vector<Light*> getLights() {
    std::vector<Light*> lights;
    for (auto& l : scene.light) {
        lights.push_back(l.second);
    }
    return lights;
}

//generates color from shading model
glm::vec3 shadingModel(Intersection* hit, std::vector<Ray> toLights) {
    // material properties for silver (using silver for shading model)
    glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 specular = glm::vec3(0.9f, 0.9f, 0.9f);
    float shininess = 50.0f;

    std::vector<Light*> lights = getLights();
    glm::vec3 color(0,0,0);
    for (int i = 0; i < toLights.size(); i++) {
        Intersection lhit = intersect(&toLights[i]);

        // if the ray to the light source intersects some part of the scene, then don't shade it (shadow)
        if (!lhit.in) {
            return glm::vec3(0, 0, 0);
        }
        //otherwise use standard shading model
        glm::vec3 lColor = glm::vec3(lights[i]->color.x, lights[i]->color.y, lights[i]->color.z);
        color = color + diffuse * lColor * glm::max(dot(-hit->ray->dir, toLights[i].dir),0.f);
    }
    return color;
}


// generate mirror reflected rays
std::vector<Ray> genMirrorRays(Ray* in, std::vector<Ray> fromLight, glm::vec3 origin) {
    std::vector<Ray> out;
    for each (Ray i in fromLight) {
        // origin of new ray is slightly elevated to avoid intersection with current object
        out.push_back(Ray(origin + in->dir * -0.1f, normalize(2 * dot(-in->dir, i.dir) * -in->dir - i.dir)));
    }

    return out;
}

// find the color of a given pixel by adding results of multiple light bounces
glm::vec3 findColor(Intersection* hit, int depth) {
    if (depth < 2 && hit->in) { // limit on recursion depth
        std::vector<Ray> toLights = genRaysToLights(hit->ray);
        glm::vec3 color = shadingModel(hit, toLights);

        // recurse on mirrored rays of light
        std::vector<Ray> mirror = genMirrorRays(hit->ray, toLights, hit->poi());
        for each (Ray m in mirror) {
            Intersection hit2 = intersect(&m);
            color = color + findColor(&hit2, depth + 1);
        }
        
        return color;
    } else {
        return glm::vec3(0, 0, 0);
    }
}

// --------- RAYTRACING FRAMEWORK -------------------------------------------------------------------------------------

std::vector<BYTE> raytrace() {
    std::vector<BYTE> image;
    Camera* cam = scene.camera;
    std::cout << "TriList Size: " << scene.triList.size() << std::endl;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            Ray ray = rayThruPixel(cam, i, j);
            Intersection hit = intersect(&ray);
            glm::vec3 color = findColor(&hit, 0);

            image.push_back(int(color.x * 255));
            image.push_back(int(color.y * 255));
            image.push_back(int(color.z * 255));
        }
        if (j % 1 == 0) {
            std::cout << "Row " << j << " of " << height << " done" << std::endl;
        }
    }
    std::cout << "Raytrace complete" << std::endl;
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
    scene.init(); 
    scene.draw();
    std::vector<BYTE> img = raytrace();
    saveimg(img, "output.png");


	return 0;   /* ANSI C requires main to return int. */
}
