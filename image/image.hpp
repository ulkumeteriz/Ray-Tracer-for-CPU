#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "color.hpp"
#include<string>

class Image
{
    private:
        unsigned char* imageArray;
        
        int width, height;
    
    public:
        Image(int width, int height);
            
        ~Image();
        
        unsigned char const * const getImageArray() const;
        
        void setColor(int positionX, int positionY, Color color);
        
        Color getColor(int positionX, int positionY) const;
        
        void write(std::string fileName) const;      
};

#endif
