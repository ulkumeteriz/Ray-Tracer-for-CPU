#include "../geometry.hpp"
#include <iostream>

using namespace std;

Position3 Triangle::getVertex(int vertexId) const
{
    if(vertexId < 3)
        return *(this->vertex[vertexId]);
    else
        return Position3(-1.0f, -1.0f, -1.0f);
}

Vector3 Triangle::getNormal() const
{
    return this->normal;
}

Vector3 Triangle::computeNormal(const Position3 & vertex0,
                             const Position3 & vertex1,
                             const Position3 & vertex2 )
{
    Vector3 normal = vertex0.to(vertex1) * vertex0.to(vertex2);
    
    normal.normalize();
    
    return normal;
}

std::ostream &operator<<(std::ostream &output, const Triangle & triangle)
{
    output << "T[ " << *triangle.vertex[0] << ", " <<
                      *triangle.vertex[1] << ", " <<
                      *triangle.vertex[2] << " ]";
    return output;
}

void Triangle::fillLookUpTable()
{
    const Position3 & a = *vertex[0];
    const Position3 & b = *vertex[1];
    const Position3 & c = *vertex[2];
    
    // A: ax - bx
    float A = lookUpTable.Ax_Bx = a.getX() - b.getX();
    
    // B: ay - by
    float B = lookUpTable.Ay_By = a.getY() - b.getY();
    
    // C: az - bz
    float C = lookUpTable.Az_Bz = a.getZ() - b.getZ();
    
    // D: ax - cx
    float D = lookUpTable.Ax_Cx = a.getX() - c.getX();
    
    // F: ay - cy
    float F = lookUpTable.Ay_Cy = a.getY() - c.getY();
    
    // E: az - cz
    float E = lookUpTable.Az_Cz = a.getZ() - c.getZ();
    
    lookUpTable.A_C = Vector3( lookUpTable.Ax_Cx, lookUpTable.Ay_Cy, lookUpTable.Az_Cz );
    lookUpTable.A_B = Vector3( lookUpTable.Ax_Bx, lookUpTable.Ay_By, lookUpTable.Az_Bz );

}

/*bool Triangle::isIntersecting(const Ray & ray) const
{
    // TODO : it is not used but performance may be increased
    // see slide1 page 40
    const Vector3 & rayDirection = ray.getDirection();
    const Position3 & rayOrigin = ray.getOrigin();
    
    // if the angle between ray and normal bigger than 90 degree, 
    //.. there is no need for coloring this triangle,
    //.. simply ignore the intersection
    if( (normal ^ rayDirection) > 0) {
        return false;
    }
    
    Vector3 A_O = Vector3( vertex[0]->getX() - rayOrigin.getX(),
                           vertex[0]->getY() - rayOrigin.getY(),
                           vertex[0]->getZ() - rayOrigin.getZ() );
    
    float determinantA = Matrix::getDeterminant(lookUpTable.A_B, lookUpTable.A_C, rayDirection);

    if(determinantA == 0.0f)
        return false; 
    
    float Y = Matrix::getDeterminant(lookUpTable.A_B, A_O, rayDirection) / determinantA;

    if(Y < 0.0f || Y > 1) return false;

    float B = Matrix::getDeterminant(A_O, lookUpTable.A_C, rayDirection) / determinantA;
    
    if(B < 0 || B + Y > 1) return false;

    return true;

}*/


bool Triangle::hit(const Ray & ray, HitInfo & hitInfo) const
{
    const Vector3 & rayDirection = ray.getDirection();
    const Position3 & rayOrigin = ray.getOrigin();
    
    // if the angle between ray and normal bigger than 90 degree, 
    //.. there is no need for coloring this triangle,
    //.. simply ignore the intersection
    if( (normal ^ rayDirection) > 0) {
        return false;
    }
    
    Vector3 A_O = Vector3( vertex[0]->getX() - rayOrigin.getX(),
                           vertex[0]->getY() - rayOrigin.getY(),
                           vertex[0]->getZ() - rayOrigin.getZ() );
                           
    const Vector3 & A_B = lookUpTable.A_B;
    
    const Vector3 & A_C = lookUpTable.A_C;
     
     const float & a = A_B.getX();
     const float & b = A_B.getY();
     const float & c = A_B.getZ();
    
     const float & d = A_C.getX();
     const float & e = A_C.getY();
     const float & f = A_C.getZ();
     
     const float & g = rayDirection.getX();
     const float & h = rayDirection.getY();
     const float & i = rayDirection.getZ();
     
     const float & j = A_O.getX();
     const float & k = A_O.getY();
     const float & l = A_O.getZ();
     
     const float cv1 = e*i - h*f;
     const float cv2 = g*f - d*i;
     const float cv3 = d*h - e*g;
     const float cv4 = a*k - j*b;
     const float cv5 = j*c - a*l;
     const float cv6 = b*l - k*c;
     
     const float determinantA = a*cv1 + b*cv2 + c*cv3;
     
     if(determinantA == 0.0f)
        return false;
     
     float Y = (i*cv4 + h*cv5 + g*cv6) / determinantA;
     
     if(Y < 0.0f || Y > 1) return false;
     
     float B = (j*cv1 + k*cv2 + l*cv3) / determinantA;
     
     if(B < 0 || B + Y > 1) return false;
      
     float T = - (f*cv4 + e*cv5 + d*cv6) / determinantA;

     if(T <= 0.0f)
        return false;
        
        hitInfo.t = T;
        hitInfo.normal = this->getNormal();  
        hitInfo.hitPosition = ray.getPoint(hitInfo.t);

    // check texture!
    hitInfo.hasTexture = false;
    
    if(this->texture != NULL)
    {
        hitInfo.hasTexture = true;
        
        unsigned char* textureImage = texture->image;
        
        float u = texCoordData[0]->u + B * (texCoordData[1]->u - texCoordData[0]->u) + Y * (texCoordData[2]->u - texCoordData[0]->u);
        //u /=  texture->width;
        
        float v = texCoordData[0]->v + B * (texCoordData[1]->v - texCoordData[0]->v) + Y * (texCoordData[2]->v - texCoordData[0]->v);
        //v /= texture->height;
        
        if( u > 1.0 || u < 0.0 || v < 0.0 || v > 1.0)
        {
            if(texture->appearance == repeat)
            {
                int floorU = u;
                int floorV = v;
                
                if( u > 1.0)
                    u = u - floorU;
                
                if( v > 1.0 )
                    v = v - floorV;
                    
                if(u < 0.0f)
                    u = (u - floorU) + 1;
                 
                if(v < 0.0f)
                    v = (v - floorV) + 1;
            }
            
            else if(texture->appearance == clamp)
            {
                if(u > 1.0)
                    u = 1.0f;
                else if(u < 0.0)
                    u = 0.0f;
                    
                if(v > 1.0)
                    v = 1.0f;
                else if(v < 0.0)
                    v = 0.0f;
            }
        }
        // taken from sphere
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
           // cout << "im triangle and I returned " << hitInfo.textureColor << endl;
        // end
        
        
    }
    
    return true;

}













