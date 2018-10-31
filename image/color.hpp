#ifndef __COLOR_H__
#define __COLOR_H__

#include "../geometry.hpp"

class Color
{
    private:
        float R, G, B;
    public:
        /*Color(unsigned char R = 0, unsigned char G = 0, unsigned char B = 0)
            : R(R), G(G), B(B) {}    */
        
        Color(float R = 0.0f, float G = 0.0f, float B = 0.0f)
        {
            this->R = R;// < 0.0f ? 0.0f : R;
            this->G = G;// < 0.0f ? 0.0f : G;
            this->B = B;// < 0.0f ? 0.0f : B;
                        
            this->R = R;// > 255.0f ? 255.0f : R;
            this->G = G;// > 255.0f ? 255.0f : G;
            this->B = B;// > 255.0f ? 255.0f : B;
        }
        
        Color(const Vector3 & colorVector)
        :   Color(colorVector.getX(), colorVector.getY(), colorVector.getZ()) {}
        
        Color operator+(const Color & rhs) const
        {
            return Color( R + rhs.getFR(),
                          G + rhs.getFG(),
                          B + rhs.getFB() );
        }
        
        Color & intensify(const Vector3 & rhs)
        {
            this->setR(this->R * rhs.getX());
            this->setG(this->G * rhs.getY());
            this->setB(this->B * rhs.getZ());
            
            return *this;
        }
        
        Color & operator+=(const Color & rhs)
        {
            setR( R + rhs.getFR() );
            setG( G + rhs.getFG() );
            setB( B + rhs.getFB() );

            return *this;
        }
        
        float getFR() const
        {
            return this->R;
        }
        
        float getFG() const
        {
            return this->G;
        }
        
        float getFB() const
        {
            return this->B;
        }
        
        unsigned char getR() const
        {
            if(this->R < 0.0f)
                return (unsigned char)0;
            
            if(this->R > 255.0f)
                return (unsigned char)255;
            
            return (unsigned char)this->R;
        }
        unsigned char getG() const
        {
            if(this->G < 0.0f)
                return (unsigned char)0;
            
            if(this->G > 255.0f)
                return (unsigned char)255;
                
            return (unsigned char)this->G;
        }
        
        unsigned char getB() const
        {
            if(this->B < 0.0f)
                return (unsigned char)0;
            
            if(this->B > 255.0f)
                return (unsigned char)255;
                
            return (unsigned char)this->B;
        }
        
        void setR(float R)
        {
            this->R = R;
            /*this->R = R < 0.0f ? 0.0f : R;
            this->R = R > 255.0f ? 255.0f : R;*/
        }
        
        void setG(float G)
        {
            this->G = G; /* < 0.0f ? 0.0f : G;
            this->G = G > 255.0f ? 255.0f : G;*/
        }
        void setB(float B)
        {
            this->B = B; /* < 0.0f ? 0.0f : B;
            this->B = B >= 255.0f ? 255.0f : B;*/
        }
        
        static Color Black()
        {
            return Color(0.0f, 0.0f, 0.0f);
        }
        
        static Color White()
        {
            return Color(255.0f, 255.0f, 255.0f);
        }
};

#endif
