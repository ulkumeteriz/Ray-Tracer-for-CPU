#include "../geometry.hpp"

// Position3
// getX
float Position3::getX() const
{
    return x;
}

// getY
float Position3::getY() const
{
    return y;
}

// getZ
float Position3::getZ() const
{
    return z;
}

// setX
void Position3::setX(float x)
{
    this -> x = x;
}

// setY
void Position3::setY(float )
{
    this -> y = y;
}

// setZ
void Position3::setZ(float z)
{
    this -> z = z;
}

Vector3 Position3::operator-(const Position3 & rhs) const
{
    Vector3 result;
    
    result.setX( this->getX() - rhs.getX() );
    result.setY( this->getY() - rhs.getY() );
    result.setZ( this->getZ() - rhs.getZ() );
    
    return result;
}

// a vector from this to rhs
Vector3 Position3::to(const Position3 & rhs) const
{
    return rhs.operator-(*this);
}

std::ostream &operator<<(std::ostream &output, const Position3 & position)
{
    output << "P( " << position.getX() << ", " << position.getY() << ", " << position.getZ() << " )";
    return output;
}

float Position3::distanceSquare(const Position3 & rhs) const
{
    return ( x - rhs.x ) * ( x - rhs.x ) + ( y - rhs.y ) * ( y - rhs.y ) + ( z - rhs.z ) * ( z - rhs.z ) ;
}



