#include "../geometry.hpp"
#include <vector>
#include <iostream>

using namespace std;

float Ray::getTValue(const Position3 & hitPosition) const
{
    Vector3 displacement = origin.to(hitPosition);
    
    if(direction.getX() != 0.0f)
        return displacement.getX() / direction.getX();
    else if(direction.getY() != 0.0f)
        return displacement.getY() / direction.getY();
    else if(direction.getZ() != 0.0f)
        return displacement.getZ() / direction.getZ();
    else return -1.0f;
}       

bool Ray::getClosestHit(Material & material, // return the material of the hit object
                   HitInfo & hitInfo,   // return the hit info
                   const std::vector<Surface*> surfaces,
                   float epsilon) // feed surfaces
{
    bool hit = false;
    
    for(int i = 0; i < surfaces.size(); i++)
    {
        Surface & surface = *surfaces[i];
        
        HitInfo currentSurfaceHitInfo;
        
        // do I hit the current surface?        
        if(surface.hit(*this, currentSurfaceHitInfo))
        {
            // if I did not hit a surface before, make it the first hit
            if(!hit || (currentSurfaceHitInfo.t < hitInfo.t))
            {
                if(currentSurfaceHitInfo.t > epsilon)
                {
                    hit = true;
                    hitInfo = currentSurfaceHitInfo;
                    material = surface.getMaterial();
                }
            }
        }
    }
    
    return hit;
}

Position3 Ray::getPoint(const float & t) const
{
    Vector3 vector = direction * t;
    
    Position3 result(   vector.getX() + origin.getX(),
                        vector.getY() + origin.getY(),
                        vector.getZ() + origin.getZ()
                    );
    return result;
}

// reflection ray of this ray
Ray Ray::createReflectionRay(const HitInfo & hitInfo) const
{
	Ray reflectionRay;
	
	reflectionRay.setOrigin(hitInfo.hitPosition);
	
	// r = d - 2(d_dot_n)n , see textbook pg 87 equation 4.5
	Vector3 reflectionDirection = this->direction - 
	                              (hitInfo.normal * (2.0 * (direction ^ hitInfo.normal)));
	
	reflectionRay.setDirection(reflectionDirection);
	
	return reflectionRay;
}

