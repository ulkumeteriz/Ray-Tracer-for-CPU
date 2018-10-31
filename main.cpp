#include "scene.hpp"

int main(int argc, char* argv[])
{
    Scene scene;
    
    scene.loadFromXml(argv[1]);
    scene.generateImages();
   
    return 0;
}
