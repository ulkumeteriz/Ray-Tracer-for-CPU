#include "../geometry.hpp"
#include <cmath>
#include <iostream>

using namespace std;

#define PI 3.14159265359

float Sphere::discriminant(const Ray & ray) const
{
    const Vector3 & direction = ray.getDirection();
    const Position3 & rayOrigin = ray.getOrigin();
    
    const Vector3 common = center.to(rayOrigin);
    // see textbook p77
    float A = direction ^ common;
    float B = direction ^ direction ;
    float C = common ^ common;
    
    return (A * A) - B * (C - (radius * radius));

}

bool Sphere::isIntersecting (const Ray & ray) const
{
    if(discriminant(ray) >= 0.0)
        return true;
    return false;

}

bool Sphere::hit(const Ray & ray, HitInfo & hitInfo) const
{
    float disc = discriminant(ray);
    
    float A = ( ray.getOrigin() - center ) ^ ( ray.getDirection() * (-1) );
    float B = ray.getDirection() ^ ray.getDirection(); 
    
    // we have 2 intersection point
    if(disc > 0.0f) 
    {
        float t1 = ( A + sqrt(disc) ) / B ;
        float t2 = ( A - sqrt(disc) ) / B ;

        hitInfo.t = t1 > t2 ? t2 : t1;
        
        if(t2 < 0)
            hitInfo.t = t1;
        else if(t1 < 0)
            hitInfo.t = t2;
        
        /*if(hitInfo.t < 0.0f)
            return false;*/
        
        hitInfo.normal = (this->getCenter().to(ray.getPoint(hitInfo.t))).normalize();
        hitInfo.hitPosition = ray.getPoint(hitInfo.t);
        
        hitInfo.hasTexture = false;
        
        if(this->texture != NULL) 
        {
            hitInfo.hasTexture = true;
            hitInfo.decalMode = texture->decalMode;
            
            Vector3 hitPositionWRTSphere = center.to(hitInfo.hitPosition);
            
            unsigned char* textureImage = texture->image;
            
            double acosParam = hitPositionWRTSphere.getY() / this->radius;
            
            
            float theta =acos(hitPositionWRTSphere.getY() / this->radius);
            /*
            if(acosParam >= 1.0)
                theta = 0.0;
            else
                theta = acos(hitPositionWRTSphere.getY() / this->radius);
           
*/
            
            float fi = atan2(hitPositionWRTSphere.getZ(), hitPositionWRTSphere.getX());
            
            float u = (-fi + PI) / (2 * PI);
            float v = theta / PI ;
            

            float i = u * texture -> width;
            float j = v * texture -> height;
            
            // pixel indexes
            int nearest_x, nearest_y;
            
            nearest_x = i;
            if ( i - nearest_x > 0.5)
                nearest_x++;
                
            nearest_y = j;
            if( j - nearest_y > 0.5 )
                nearest_y++;
            
            //cout << "[" << nearest_x << ", " << nearest_y << "]" << endl;
            // since they will be inside in the Vector3
            float r, g, b;
            int width = texture->width;
            int height = texture->height;
            
            if (texture -> interpolation == nearest)
            {
                int colorIndex = (nearest_y * width + nearest_x) * 3;
                
                // get color from texture->image unsigned char array
                r = textureImage[colorIndex];
                g = textureImage[colorIndex + 1];
                b = textureImage[colorIndex + 2];
                
            }
            // bilinear
            else 
            {
                // floor 
                int p = i;
                int q = j;
                
                float dx = i - p;
                float dy = j - q;
                
                int colorIndex1 = ( q * width + p )*3;
                int colorIndex2 = ( q * width + p+1 )*3;
                int colorIndex3 = ( (q+1) * width + p )*3;
                int colorIndex4 = ( (q+1) * width + (p+1) )*3;
                
                r = textureImage[colorIndex1] * (1 - dx) * (1 - dy) +
                    textureImage[colorIndex2] * (dx) * (1 - dy) +
                    textureImage[colorIndex3] * (1 - dx) * (dy) +
                    textureImage[colorIndex4] * (dx) * (dy);
                    
                g = textureImage[colorIndex1 + 1] * (1 - dx) * (1 - dy) +
                    textureImage[colorIndex2 + 1] * (dx) * (1 - dy) +
                    textureImage[colorIndex3 + 1] * (1 - dx) * (dy) +
                    textureImage[colorIndex4 + 1] * (dx) * (dy);
                
                b = textureImage[colorIndex1 + 2] * (1 - dx) * (1 - dy) +
                    textureImage[colorIndex2 + 2] * (dx) * (1 - dy) +
                    textureImage[colorIndex3 + 2] * (1 - dx) * (dy) +
                    textureImage[colorIndex4 + 2] * (dx) * (dy);
                
            }
            hitInfo.textureColor = Vector3(r, g, b);
            
        }

            
        return true;    
    }
    // the ray grazes
    else if (disc == 0.0f) 
    {
        hitInfo.t = A / B;
        
        if(hitInfo.t > 0.0f)
        {
            hitInfo.normal = (this->getCenter().to(ray.getPoint(hitInfo.t))).normalize();
            hitInfo.hitPosition = ray.getPoint(hitInfo.t);
            return true;
        }
        else
        {
            return false;
        }
    }
    // no intersection
    else 
        return false;

}
