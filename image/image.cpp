#include "image.hpp"
#include "color.hpp"
#include "ppm.h"
#include<string>


Image::Image(int width, int height)
    : width(width), height(height), imageArray(nullptr)
{
    if(width != 0 && height != 0)
    {
        this->imageArray = new unsigned char[width * height * sizeof(unsigned char) * 3];
    }
}

Image::~Image()
{
    if(this->imageArray != nullptr)
        delete[] this->imageArray;
        
    this->imageArray = nullptr;
}

unsigned char const * const Image::getImageArray() const
{
    return this->imageArray;
}

void Image::setColor(int positionX, int positionY, Color color)
{
    if(positionX > this->width || positionY > this->height)
        return;
    
    int index = positionX + (this->width * positionY);
    index *= 3;

    this->imageArray[index++] = color.getR();
    this->imageArray[index++] = color.getG();
    this->imageArray[index++] = color.getB();
}

void Image::write(std::string fileName) const
{
    if(this->imageArray == nullptr)
        return;

    write_ppm(fileName.data(), this->imageArray, this->width, this->height);
}

Color Image::getColor(int positionX, int positionY) const
{
    if(positionX > this->width || positionY > this->height)
        return Color::Black();
    
    int index = positionX + (positionX * positionY);
    
    return Color(
        this->imageArray[index],
        this->imageArray[index + 1],
        this->imageArray[index + 2]
    );
}
