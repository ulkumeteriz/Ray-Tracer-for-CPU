#include "../geometry.hpp"
#include <string>
#include <iostream>

using namespace std;

void Camera::deleteRays()
{
    if(this->rays != NULL)
    {
    
        int imageWidth = this->getImageW();

        for(int i = 0; i < imageWidth; i++)
        {
            if(this->rays[i] != NULL)
            {
                delete[] this->rays[i];
                this->rays[i] = NULL;
            }
        }
        
        delete[] this->rays;
        this->rays = NULL;
    }
}

Camera::~Camera()
{
    this->deleteRays();
}

Ray** Camera::getRays() const
{
    return this->rays;
}

int Camera::getImageW() const
{
    return this->image_width;
}

int Camera::getImageH() const
{
    return this->image_height;
}

std::string Camera::getImageName() const
{
    return this->image_name;
}

void Camera::generateRays()
{
    // first, delete the rays that are already generated
    this->deleteRays();
    
    // get the fields to make the computations clearer
    const int imageWidth = this->getImageW();
    const int imageHeight = this->getImageH();
    
    Vector3 gaze = this->gaze;
    
    const Position3 cameraPosition = this->position;
    
    const float left   = this->near_plane.x;
    const float right  = this->near_plane.y;
    const float bottom = this->near_plane.z;
    const float top    = this->near_plane.w;
    const float d      = this->near_distance;
    
    Vector3 vecV = this->up;
    vecV.normalize();
    
    Vector3 vecW = -gaze;
    gaze.normalize();
    vecW.normalize();
    
    Vector3 vecU = vecV * vecW; // cross product
    vecU.normalize();
   
    vecV = vecW * vecU;
    
    // normally, the computations are done as follows
    // however, to increase the speed, some of the parameters will be used as constants
    float uConstant = (right - left) / imageWidth;
    float vConstant = (top - bottom) / imageHeight;
    
    Vector3 initialDirection = gaze * d;
    
    this->rays = new Ray*[imageWidth];
    
    for(int i = 0; i < imageWidth; i++)
    {
        this->rays[i] = new Ray[imageHeight];
        
        float u = left + (i + 0.5) * uConstant;
        
        for(int j = 0; j < imageHeight; j++)
        {
            float v = top - (j + 0.5) * vConstant;
            
            Vector3 direction = initialDirection + vecU * u + vecV * v;
            
            // normalize the direction
            direction.normalize();
            
            this->rays[i][j] = Ray(cameraPosition, direction);
        }
    }
    
}






