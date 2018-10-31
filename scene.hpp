#ifndef __SCENE_H__
#define __SCENE_H__

#include "geometry.hpp"
#include "image/color.hpp"
#include "transformation.hpp"
#include <string>

class Scene
{
    public:
        
        ~Scene()
        {
           for(int i = 0; i < surfaces.size(); i++)
                delete surfaces[i];
        }
        
        Color backgroundColor;
        float shadowRayEpsilon;
        int maxRecursionDepth;
        std::vector<Camera> cameras;
        Vector3 ambientLight;
        std::vector<PointLight> pointLights;
        std::vector<Material> materials;
        std::vector<Position3> vertexData;
        std::vector<Surface*> surfaces;
        std::vector<Texture*> textures;
        std::vector<Scaling> scalings;
        std::vector<Rotation> rotations;
        std::vector<Translation> translations;
        std::vector<TexCoord*> texCoordData;
        
        void loadFromXml(const std::string& filepath);
        void generateImages();
        Color getRayColor(Ray & ray, int recursionDepth, bool);
        Color getReflectionColor(const Ray & ray, const HitInfo & hitInfo, int recursionDepth);
};

#endif
