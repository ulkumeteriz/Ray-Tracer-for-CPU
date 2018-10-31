#include "../geometry.hpp"
#include <cmath>
#include <iostream>

// getX
float Vector3::getX() const
{
    return x;
}

// getY
float Vector3::getY() const
{
    return y;
}

// getZ
float Vector3::getZ() const
{
    return z;
}

// setX
void Vector3::setX(float x)
{
    this -> x = x;
}

// setY
void Vector3::setY(float y)
{
    this -> y = y;
}

// setZ
void Vector3::setZ(float z)
{
    this -> z = z;
}

// unary (-) operator
Vector3 Vector3::operator-() const
{
    return *this * (-1);
}

// Vector3 substraction
Vector3 Vector3::operator-(const Vector3 & rhs) const
{
    Vector3 result;
    
    result.setX( this->getX() - rhs.getX() );
    result.setY( this->getY() - rhs.getY() );
    result.setZ( this->getZ() - rhs.getZ() );
    
    return result;
}

// Vector3 addition
Vector3 Vector3::operator+(const Vector3 & rhs) const
{
    Vector3 result;
    
    result.setX( this->getX() + rhs.getX() );
    result.setY( this->getY() + rhs.getY() );
    result.setZ( this->getZ() + rhs.getZ() );
    
    return result;
}

// dot product
float Vector3::operator^(const Vector3 & rhs) const
{
    return this->getX() * rhs.getX()
         + this->getY() * rhs.getY()
         + this->getZ() * rhs.getZ();
}

// cross product
Vector3 Vector3::operator*(const Vector3 & rhs) const
{
    Vector3 result;
    
    result.setX( this->getY() * rhs.getZ() - this->getZ() * rhs.getY() );
    result.setY( this->getZ() * rhs.getX() - this->getX() * rhs.getZ() );
    result.setZ( this->getX() * rhs.getY() - this->getY() * rhs.getX() );
    
    return result;
}

// normalize
Vector3 & Vector3::normalize()
{
    float norm = this->getNorm();
    
    // normalize by dividing the vector by its norm
    *this = *this / norm;
    
    return *this;
}

// scalar multiplication
Vector3 Vector3::operator*(float rhs) const
{
    Vector3 result;
    
    result.setX( this->getX() * rhs );
    result.setY( this->getY() * rhs );
    result.setZ( this->getZ() * rhs );
    
    return result;
}

// scalar division
Vector3 Vector3::operator/(float rhs) const
{
    if(rhs == 0.0)
    {
        // division by zero!
        return Vector3(0.0, 0.0, 0.0);
    }

    Vector3 result;
    
    result.setX( this->getX() / rhs );
    result.setY( this->getY() / rhs );
    result.setZ( this->getZ() / rhs );
    
    return result;
}

// get norm
float Vector3::getNorm() const
{
    float normSquare = this->getX() * this->getX()
                     + this->getY() * this->getY()
                     + this->getZ() * this->getZ();
                     
    return sqrt( normSquare );
}

// operator overloadings with different order
Vector3 operator*(float lhs, const Vector3 & rhs)
{
    return rhs * lhs;
}

Vector3 operator/(float lhs, const Vector3 & rhs)
{
    return rhs / lhs;
}

// intensify
Vector3 Vector3::intensify(const Vector3 & intensityVector) const
{

    return Vector3( this->getX() * intensityVector.getX(),
                    this->getY() * intensityVector.getY(),
                    this->getZ() * intensityVector.getZ() );
}

std::ostream &operator<<(std::ostream &output, const Vector3 & vector)
{
    output << "V( " << vector.getX() << ", " << vector.getY() << ", " << vector.getZ() << " )";
    return output;
}

        Vector3 Vector3::generateDifferentlyDirectedVector() const
        {
            float absX = x > 0.0f ? x : -x;
            float absY = y > 0.0f ? y : -y;
            float absZ = z > 0.0f ? z : -z;
            
            if(absX <= absY && absX <= absZ)
            {
                return Vector3(0, z, -y);
            }
            else if( absY <= absX && absY <= absZ)
            {
                return Vector3(-z, 0, x);
            }
            else if(absZ <= absX && absZ <= absZ)
            {
                return Vector3(-y, x, 0);
            }
        }




