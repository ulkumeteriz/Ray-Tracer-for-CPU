#include "../scene.hpp"
#include "../geometry.hpp"
#include "../filemanip/tinyxml2.h"
#include "../image/color.hpp"
#include "../image/image.hpp"
#include "../matrix4.hpp"
#include "../transformation.hpp"
#include "../jpeg.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <cmath>

using namespace std;

typedef struct MeshInstance{
        int base_mesh_id;
        int material_id;
        Transformation transformation;
        Texture* texturePtr;
    } MeshInstance;

std::ostream &operator<<(std::ostream &output, const Matrix4 & m)
{
    output << "M[ " << m[0][0]  << ", " << m[0][1]  << ", " << m[0][2]  << ", " << m[0][3]  << std::endl << "   " <<
                       m[1][0]  << ", " << m[1][1]  << ", " << m[1][2]  << ", " << m[1][3]  << std::endl << "   " <<
                       m[2][0]  << ", " << m[2][1]  << ", " << m[2][2]  << ", " << m[2][3]  << std::endl << "   " <<
                       m[3][0]  << ", " << m[3][1]  << ", " << m[3][2]  << ", " << m[3][3]  << std::endl << " ]";

    return output;
}

Color getDiffuseColorWithoutTexture(const Material & material, const HitInfo & hitInfo, const PointLight & pointLight)
{ 
    // hit to light
    Vector3 hit2light = pointLight.position - hitInfo.hitPosition;
                        
    // distanceSquare
    float distanceSq = hit2light ^ hit2light;

    // intensity
    Vector3 intensity = pointLight.intensity / distanceSq;

    hit2light.normalize();
    
    float normalDotLight = (hitInfo.normal ^ hit2light);
    
    if(normalDotLight < 0.0f)
        normalDotLight = 0.0f;
        
    Color diffuseColor = Color(material.diffuse.intensify(intensity) * normalDotLight);

    return diffuseColor;
}

Color getDiffuseColor(const Material & material, const HitInfo & hitInfo, const PointLight & pointLight)
{
    if(!hitInfo.hasTexture)
        return getDiffuseColorWithoutTexture(material, hitInfo, pointLight);
        
    if(hitInfo.decalMode == replace_kd)
    {
        Material newMaterial = material;
        newMaterial.diffuse = hitInfo.textureColor / 255;
        
        return getDiffuseColorWithoutTexture(newMaterial, hitInfo, pointLight);
    }
    else if(hitInfo.decalMode == blend_kd)
    {
        Material newMaterial = material;
        newMaterial.diffuse = ((hitInfo.textureColor / 255) + material.diffuse) / 2;
        
        return getDiffuseColorWithoutTexture(newMaterial, hitInfo, pointLight);
    }
    else if(hitInfo.decalMode == replace_all)
    {
        return Color(hitInfo.textureColor);
    }
}



/*
Color getDiffuseColor(const Material & material, Texture* texture, const HitInfo & hitInfo, const PointLight & pointLight)
{ 
    if(texture == NULL)
        return Color getDiffuseColor(const Material & material, const HitInfo & hitInfo, const PointLight & pointLight);

    // hit to light
    Vector3 hit2light = pointLight.position - hitInfo.hitPosition;
                        
    // distanceSquare
    float distanceSq = hit2light ^ hit2light;

    // intensity
    Vector3 intensity = pointLight.intensity / distanceSq;

    hit2light.normalize();
    
    float normalDotLight = (hitInfo.normal ^ hit2light);
    
    if(normalDotLight < 0.0f)
        normalDotLight = 0.0f;
        
    // TODO texture için material diffuse a değil başka şeye eriş
//typedef enum DecalMode { replace_kd, blend_kd, replace_all } DecalMode;
    if(texture->decalMode == replace_kd)
    {
        // get the new diffuse component, which is TextureColor / 255
        // TODO: triangle and sphere uses different ways to get the coordinate of the pixel..
    }
    Color diffuseColor = Color(material.diffuse.intensify(intensity) * normalDotLight);

    return diffuseColor;
}
*/

Color getSpecular(const Ray & ray, const Material & material, const HitInfo & hitInfo, const PointLight & pointLight)
{
    Vector3 hit2light = hitInfo.hitPosition.to(pointLight.position);
    float distanceSq = hit2light ^ hit2light;
    
    Vector3 l = hit2light;
    l.normalize();
    
    Vector3 intensity = pointLight.intensity / distanceSq;
    
    Vector3 h = (( -ray.getDirection()) + l ).normalize();
    
    float max = hitInfo.normal ^ h;
    max = max < 0.0f ? 0.0f : max;
    max = pow(max, material.phong_exponent);
    
    Vector3 colorVector = material.specular.intensify(intensity) * max;
    
    return Color(colorVector);
}

Color getAmbientColor(const Material & material, const Vector3 & ambientLight)
{
    return Color(ambientLight.intensify(material.ambient));
}


bool isLyingInShadow(const HitInfo & hitInfo, const PointLight & pointLight, const std::vector<Surface*> & surfaces, float shadowRayEpsilon)
{   
    // first, create the shadow ray
    Ray shadowRay(hitInfo.hitPosition, hitInfo.hitPosition.to(pointLight.position));
    
    Material dummyMaterial;
    HitInfo shadowRayHitInfo;
    
   // return shadowRay.getClosestHit(dummyMaterial, shadowRayHitInfo, surfaces, shadowRayEpsilon);
    
    
    if(shadowRay.getClosestHit(dummyMaterial, shadowRayHitInfo, surfaces, shadowRayEpsilon))
    {   
        float hitPointToLightT = shadowRay.getTValue(pointLight.position);
        
        if(hitPointToLightT < shadowRayHitInfo.t)
        {
            // cheers! point is closer to us
            return false;
        }
        else
        {
            // in shadow..
            return true;
        }
    }
    else
        return false;
}

Color Scene::getReflectionColor(const Ray & ray, const HitInfo & hitInfo, int recursionDepth)
{
    if(recursionDepth == 0)
    {
        return Color::Black();
    }

    Ray reflectionRay = ray.createReflectionRay(hitInfo);
    
    return getRayColor(reflectionRay, recursionDepth - 1, true);
    
}

Color Scene::getRayColor(Ray & ray, int recursionDepth, bool isRef)
{
    HitInfo hitInfo;
    Material material;
    
    if( ray.getClosestHit(material, hitInfo, this->surfaces, -1.0f) )
    {
        Color color(0.0f, 0.0f, 0.0f);
        
        // ambient
        if(!hitInfo.hasTexture || (hitInfo.hasTexture && hitInfo.decalMode != replace_all) )
            color += getAmbientColor(material, this->ambientLight);
        
        // traverse point lights
        for(int p = 0; p < this->pointLights.size(); p++ )
        {
            PointLight & pointLight = this->pointLights[p];
            
            // if light is not seenable, continue
            if(!isLyingInShadow(hitInfo, pointLight, this->surfaces, this->shadowRayEpsilon) )
            {
                 
                // diffuse
                color += getDiffuseColor(material, hitInfo, pointLight);
                
                // specular
                if(!hitInfo.hasTexture || (hitInfo.hasTexture && hitInfo.decalMode != replace_all ))
                    color += getSpecular(ray, material, hitInfo, pointLight);

            }
            
        }
        
        // reflection
        bool hasReflection = material.mirror.getX() != 0.0f || material.mirror.getY() != 0.0f || material.mirror.getZ() != 0.0f;
                
        if(hasReflection && !hitInfo.hasTexture ||(hitInfo.hasTexture && hitInfo.decalMode != replace_all ))
        {
            color += getReflectionColor(ray, hitInfo, recursionDepth).intensify(material.mirror);
        }      
        return color;           
    }
    else
    {
        return this->backgroundColor;
    }
}

void Scene::generateImages()
{
    // generate one image for each camera
    for(int i = 0; i < this->cameras.size(); i++)
    {
        Camera & camera = this->cameras[i];
        
        int imageWidth = camera.getImageW();
        int imageHeight = camera.getImageH();
        
        Image image(imageWidth, imageHeight);
        
        // generate the rays
        camera.generateRays();
        
        Ray ** rays = camera.getRays();
        
        for(int x = 0; x < imageWidth; x++)
        {
            for(int y = 0; y < imageHeight; y++)
            {
                Ray & ray = rays[x][y];
                
                image.setColor(x, y, this->getRayColor(ray, this->maxRecursionDepth, false));
             
            }
        }
        
        image.write(camera.image_name.data());
    }
    
}

void Scene::loadFromXml(const std::string& filepath)
{
    tinyxml2::XMLDocument file;
    std::stringstream stream;
    
    auto res = file.LoadFile(filepath.c_str());
    if (res)
    {
        throw std::runtime_error("Error: The xml file cannot be loaded.");
    }

    auto root = file.FirstChild();
    if (!root)
    {
        throw std::runtime_error("Error: Root is not found.");
    }

    //Get BackgroundColor
    auto element = root->FirstChildElement("BackgroundColor");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0 0 0" << std::endl;
    }
    int bgColorR, bgColorG, bgColorB;
    stream >> bgColorR >> bgColorG >> bgColorB;
    backgroundColor.setR(bgColorR);
    backgroundColor.setG(bgColorG);
    backgroundColor.setB(bgColorB);

    //Get ShadowRayEpsilon
    element = root->FirstChildElement("ShadowRayEpsilon");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0.001" << std::endl;
    }
    stream >> shadowRayEpsilon;

    //Get MaxRecursionDepth
    element = root->FirstChildElement("MaxRecursionDepth");
    if (element)
    {
        stream << element->GetText() << std::endl;
    }
    else
    {
        stream << "0" << std::endl;
    }
    stream >> maxRecursionDepth;

    //Get Cameras
    element = root->FirstChildElement("Cameras");
    element = element->FirstChildElement("Camera");
    Camera camera;
    while (element)
    {
        auto child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Gaze");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Up");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("NearPlane");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("NearDistance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageResolution");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageName");
        stream << child->GetText() << std::endl;

        stream >> camera.position.x >> camera.position.y >> camera.position.z;
        stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
        stream >> camera.up.x >> camera.up.y >> camera.up.z;
        stream >> camera.near_plane.x >> camera.near_plane.y >> camera.near_plane.z >> camera.near_plane.w;
        stream >> camera.near_distance;
        stream >> camera.image_width >> camera.image_height;
        stream >> camera.image_name;
        
        camera.rays = NULL;

        cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }

    //Get Lights
    element = root->FirstChildElement("Lights");
    auto child = element->FirstChildElement("AmbientLight");
    stream << child->GetText() << std::endl;
    stream >> ambientLight.x >> ambientLight.y >> ambientLight.z;
    element = element->FirstChildElement("PointLight");
    PointLight pointLight;
    while (element)
    {
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Intensity");
        stream << child->GetText() << std::endl;

        stream >> pointLight.position.x >> pointLight.position.y >> pointLight.position.z;
        stream >> pointLight.intensity.x >> pointLight.intensity.y >> pointLight.intensity.z;

        pointLights.push_back(pointLight);
        element = element->NextSiblingElement("PointLight");
    }

    //Get Materials
    element = root->FirstChildElement("Materials");
    element = element->FirstChildElement("Material");
    Material material;
    while (element)
    {
        child = element->FirstChildElement("AmbientReflectance");
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> material.ambient.x >> material.ambient.y >> material.ambient.z;
        }

        child = element->FirstChildElement("DiffuseReflectance");
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z;
        }
           
        child = element->FirstChildElement("SpecularReflectance");
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> material.specular.x >> material.specular.y >> material.specular.z;
        }
           
        child = element->FirstChildElement("MirrorReflectance");
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> material.mirror.x >> material.mirror.y >> material.mirror.z;
        }
        
        child = element->FirstChildElement("PhongExponent");
        if(child)
        {
            stream << child->GetText() << std::endl;
            stream >> material.phong_exponent;
        }

        materials.push_back(material);
        element = element->NextSiblingElement("Material");
    }
    
    
    // TODO Get Textures
    
    element = root->FirstChildElement("Textures");
    
    if(element)
    {
        element = element->FirstChildElement("Texture");
        
    }
    
    Texture *texturePtr;

    int width, height; 
    std::string imageName;
    while (element)
    {
        texturePtr = new Texture;
        Texture & texture = *texturePtr;
        
        child = element->FirstChildElement("ImageName");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Interpolation");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("DecalMode");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Appearance");
        stream << child->GetText() << std::endl;

        stream >> imageName;
        
        
        read_jpeg_header(imageName.data(), width, height);
        
        texture.width = width;
        texture.height = height;
        
        unsigned char * image = new unsigned char[width * height * 3];
        read_jpeg(imageName.data(), image, width, height);
        texture.image = image;
        
        // take the parameters as string then accordingly change the enumaration 
        std::string tempInterpolationType, tempDecalMode, tempAppearance;
        
        stream >> tempInterpolationType; //texture.interpolation;
        stream >> tempDecalMode; //texture.decalMode;
        stream >> tempAppearance; //texture.appearance;

        if (tempInterpolationType[0] == 'b')
            texture.interpolation = Interpolation::bilinear;
        else if (tempInterpolationType[0] == 'n')
            texture.interpolation = Interpolation::nearest;
            
        if (tempDecalMode[0] == 'b')
            texture.decalMode = DecalMode::blend_kd;
        else if (tempDecalMode[8] == 'k')
            texture.decalMode = DecalMode::replace_kd;
        else 
            texture.decalMode = DecalMode::replace_all;
            
        if (tempAppearance[0] == 'c')
            texture.appearance = Appearance::clamp;
        else 
            texture.appearance = Appearance::repeat;
            
        textures.push_back(texturePtr);
        
        element = element->NextSiblingElement("Texture");
    }

    //Transformations
    
    //Scalings
    element = root->FirstChildElement("Transformations");
    
    if(element)
        element = element->FirstChildElement("Scaling");
    
    while (element)
    {
        stream << element->GetText() << std::endl;
        
        float x, y, z;

        stream >> x >> y >> z;
        
        scalings.push_back(Scaling(x, y, z));
        
        element = element->NextSiblingElement("Scaling");
    }
    
    //Translations
    element = root->FirstChildElement("Transformations");
    
    if(element)
        element = element->FirstChildElement("Translation");
    
    while (element)
    {
        stream << element->GetText() << std::endl;
        
        float x, y, z;

        stream >> x >> y >> z;
        
        translations.push_back(Translation(x, y, z));
        
        element = element->NextSiblingElement("Translation");
    }

    //Rotations
    element = root->FirstChildElement("Transformations");
    
    if(element)
        element = element->FirstChildElement("Rotation");
    
    while (element)
    {
        stream << element->GetText() << std::endl;
        
        float angle, x, y, z;

        stream >> angle >> x >> y >> z;
        
        rotations.push_back(Rotation(angle, Vector3(x, y, z)));
        
        element = element->NextSiblingElement("Rotation");
    }
    

    //Get VertexData
    element = root->FirstChildElement("VertexData");
    stream << element->GetText() << std::endl;
    float vertex_x, vertex_y, vertex_z;
    while (!(stream >> vertex_x).eof())
    {
        stream >> vertex_y >> vertex_z;
        vertexData.push_back(Position3(vertex_x, vertex_y, vertex_z));
    }
    stream.clear();
    
    // Get TexCoordData
    element = root->FirstChildElement("TexCoordData");
    if(element)
    {
        stream << element->GetText() << std::endl;
    
        int coord_u, coord_v;
        
        while (!(stream >> coord_u).eof())
        {
            stream >> coord_v;
            
            TexCoord* texCoord= new TexCoord;
            
            texCoord->u = coord_u;
            texCoord->v = coord_v;
            
            texCoordData.push_back(texCoord);
        }
        stream.clear();
    }

    
    vector<MeshInstance> meshInstances;
    
    //Get Mesh Instances
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("MeshInstance");
    while(element)
    {
        int base_mesh_id, material_id;
        Transformation transformation;
        
        element->QueryAttribute("baseMeshId", &base_mesh_id);
        
        // material
        child = element->FirstChildElement("Material");
        if(child)
        {
            child = element->FirstChildElement("Material");
            stream << child->GetText() << std::endl;
            stream >> material_id;
        }
        
        //texture
        Texture* texturePtr = NULL;
        child = element->FirstChildElement("Texture");
        if(child)
        {
            int textureId;
            stream << child->GetText() << endl;
            stream >> textureId ;
            
            textureId--;
            texturePtr = textures[textureId];
        }
        
        // transformation
        child = element->FirstChildElement("Transformations");
        if(child)
        {
            stream << child->GetText() << std::endl;
            string transformationInfo;
            
            while(!(stream >> transformationInfo).eof())
            {
                // get index
                string indexString = transformationInfo.substr(1); int tIndex = stoi(indexString) - 1; 
                
                // scaling
                if(transformationInfo[0] == 's')
                {
                   transformation += scalings[tIndex];
                }
                
                // rotation
                else if(transformationInfo[0] == 'r')
                {
                    transformation += rotations[tIndex];
                }
                // translation
                else if(transformationInfo[0] == 't')
                {
                    transformation += translations[tIndex];
                }
            }
        }
        
        MeshInstance meshInstance;
        meshInstance.base_mesh_id = base_mesh_id;
        meshInstance.transformation = transformation;
        meshInstance.material_id = material_id;
        
        meshInstances.push_back(meshInstance);
        
        stream.clear();
        
        element = element->NextSiblingElement("MeshInstance");
        
    }
    
    stream.clear();

    //Get Meshes
    // simply add meshes as triangles, so that the problem simplifies
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Mesh");
    while (element)
    {
        int material_id;
        int mesh_id;
        
        // get mesh id
        element->QueryAttribute("id", &mesh_id);
        
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> material_id;
        
        bool needsTransformation = false;
        Transformation transformation;

        // get Texture
        Texture* texturePtr = NULL;
        child = element->FirstChildElement("Texture");
        if(child)
        {
            int textureId;
            stream << child->GetText() << endl;
            stream >> textureId ;
            
            textureId--;
            texturePtr = textures[textureId];
        }
        
        // get Transformations
        child = element->FirstChildElement("Transformations");
        if(child)
        {
            needsTransformation = true;
            
            stream << child->GetText() << std::endl;
            string transformationInfo;
            
            while(!(stream >> transformationInfo).eof())
            {
                
                // get index
                string indexString = transformationInfo.substr(1); int tIndex = stoi(indexString) - 1; 
                // scaling
                if(transformationInfo[0] == 's')
                {
                   transformation += scalings[tIndex];
                }
                
                // rotation
                else if(transformationInfo[0] == 'r')
                {
                    transformation += rotations[tIndex];
                }
                // translation
                else if(transformationInfo[0] == 't')
                {
                    transformation += translations[tIndex];
                }
            }
        }
        stream.clear();
        child = element->FirstChildElement("Faces");
        stream << child->GetText() << std::endl;
        int v0_id, v1_id, v2_id;
        while (!(stream >> v0_id).eof())
        {
            stream >> v1_id >> v2_id;

            Triangle * triangle;
            
            Position3 *v0, *v1, *v2;
            
            if(needsTransformation)
            {
                // TODO: maybe something else is needed
                v0 = new Position3(transformation.transform<Position3>(vertexData[v0_id - 1]));
                v1 = new Position3(transformation.transform<Position3>(vertexData[v1_id - 1]));
                v2 = new Position3(transformation.transform<Position3>(vertexData[v2_id - 1]));


            }
            else
            {
                // TODO: maybe some memory reallocation needed
                v0 = new Position3(vertexData[v0_id - 1]);
                v1 = new Position3(vertexData[v1_id - 1]);
                v2 = new Position3(vertexData[v2_id - 1]);
                
            }
            
            if(v2_id <= (int)texCoordData.size())
            {
                triangle = new Triangle( materials[material_id - 1],
                                           texturePtr,
                                           *v0,
                                           *v1,
                                           *v2,
                                           texCoordData[v0_id - 1],
                                           texCoordData[v1_id - 1],
                                           texCoordData[v2_id - 1] );
            }
            else
            {
                triangle = new Triangle( materials[material_id - 1],
                                           texturePtr,
                                           *v0,
                                           *v1,
                                           *v2,
                                           NULL,
                                           NULL,
                                           NULL );
            }
            

                                       
            // now, search if there are instances of this mesh
            for(int i = 0; i < (int)meshInstances.size(); i++)
            {
                if(meshInstances[i].base_mesh_id == mesh_id)
                {
                    Position3 *new_v0, *new_v1, *new_v2;
                    
                    new_v0 = new Position3(meshInstances[i].transformation.transform<Position3>(*v0));
                    new_v1 = new Position3(meshInstances[i].transformation.transform<Position3>(*v1));
                    new_v2 = new Position3(meshInstances[i].transformation.transform<Position3>(*v2));

                    Triangle * new_triangle;
                    
                    if(v2_id <= (int)texCoordData.size())
                    {
                        new_triangle = new Triangle( materials[meshInstances[i].material_id - 1],
                                                   texturePtr,
                                                   *new_v0,
                                                   *new_v1,
                                                   *new_v2,
                                                   texCoordData[v0_id - 1],
                                                   texCoordData[v1_id - 1],
                                                   texCoordData[v2_id - 1] );
                    }
                    else
                    {
                        new_triangle = new Triangle( materials[meshInstances[i].material_id - 1],
                                                   texturePtr,
                                                   *new_v0,
                                                   *new_v1,
                                                   *new_v2,
                                                   NULL,
                                                   NULL,
                                                   NULL );
                    }
                                               
                    surfaces.push_back((Surface*)(new_triangle));
                    
                }
            }
            
            surfaces.push_back((Surface*)(triangle));
        }
        stream.clear();

        element = element->NextSiblingElement("Mesh");
    }
    stream.clear();

    //Get Triangles
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Triangle");

    while (element)
    {
        int v0_id, v1_id, v2_id, material_id;
        
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> material_id;

        // TODO Get Texture
        // HERERERERE
        
        Texture* texturePtr = NULL;
        child = element->FirstChildElement("Texture");
        if(child)
        {
            int textureId;
            stream << child->GetText() << endl;
            stream >> textureId ;
            
            textureId--;
            texturePtr = textures[textureId];
        }
        
        // TODO Get Transformation
        
        bool needsTransformation = false;
        Transformation transformation;

        //GetTransformations
        child = element->FirstChildElement("Transformations");
        if(child)
        {
            needsTransformation = true;
            
            stream << child->GetText() << std::endl;
            string transformationInfo;
            
            while(!(stream >> transformationInfo).eof())
            {
                
                // get index
                string indexString = transformationInfo.substr(1);
                int tIndex = stoi(indexString) - 1;
                
                // scaling
                if(transformationInfo[0] == 's')
                {
                   transformation += scalings[tIndex];
                }
                
                // rotation
                else if(transformationInfo[0] == 'r')
                {
                    transformation += rotations[tIndex];
                }
                // translation
                else if(transformationInfo[0] == 't')
                {
                    transformation += translations[tIndex];
                }
            }
        }
        stream.clear();

        child = element->FirstChildElement("Indices");
        stream << child->GetText() << std::endl;
        stream >> v0_id >> v1_id >> v2_id;
                                       
                                       
       Position3 *v0, *v1, *v2;
       
       if(needsTransformation)
       {
                
           // TODO: maybe something else is needed
           v0 = new Position3(transformation.transform<Position3>(vertexData[v0_id - 1]));
           v1 = new Position3(transformation.transform<Position3>(vertexData[v1_id - 1]));
           v2 = new Position3(transformation.transform<Position3>(vertexData[v2_id - 1]));

       }
       else
       {
           // TODO: maybe some memory reallocation needed
           v0 = new Position3(vertexData[v0_id - 1]);
           v1 = new Position3(vertexData[v1_id - 1]);
           v2 = new Position3(vertexData[v2_id - 1]);
                

       }
       
       Triangle * triangle;
       
            if(v2_id <= (int)texCoordData.size())
            {
                triangle = new Triangle( materials[material_id - 1],
                                           texturePtr,
                                           *v0,
                                           *v1,
                                           *v2,
                                           texCoordData[v0_id - 1],
                                           texCoordData[v1_id - 1],
                                           texCoordData[v2_id - 1] );
            }
            else
            {
                triangle = new Triangle( materials[material_id - 1],
                                           texturePtr,
                                           *v0,
                                           *v1,
                                           *v2,
                                           NULL,
                                           NULL,
                                           NULL );
            }
            
        surfaces.push_back((Surface*)(triangle));
                
        element = element->NextSiblingElement("Triangle");
    }

    //Get Spheres
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Sphere");

    while (element)
    {
        int center_vertex_id, material_id;
        float radius;

        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> material_id;
        
        child = element->FirstChildElement("Center");
        stream << child->GetText() << std::endl;
        stream >> center_vertex_id;
        
        // vertex indexing starts from 1
        center_vertex_id--;
        
        Position3 center = Position3(vertexData[center_vertex_id].getX(), 
                                  vertexData[center_vertex_id].getY(), 
                                  vertexData[center_vertex_id].getZ() );

        child = element->FirstChildElement("Radius");
        stream << child->GetText() << std::endl;
        stream >> radius;
        
        // get Texture
        Texture* texturePtr = NULL;
        child = element->FirstChildElement("Texture");
        if(child)
        {
            int textureId;
            stream << child->GetText() << endl;
            stream >> textureId ;
            
            textureId--;
            texturePtr = textures[textureId];
        }
        
        bool needsTransformation = false;
        Transformation transformation;

        //GetTransformations
        child = element->FirstChildElement("Transformations");
        if(child)
        {
            needsTransformation = true;
            
            stream << child->GetText() << std::endl;
            string transformationInfo;
            while(!(stream >> transformationInfo).eof())
            {
                
                // get index
                string indexString = transformationInfo.substr(1);
                int tIndex = stoi(indexString) - 1;
                
                // scaling
                if(transformationInfo[0] == 's')
                {
                    // scale radius
                    // works only if it is uniform
                    radius *= scalings[tIndex].getTransformationMatrix()[0][0];

                    transformation += scalings[tIndex];

                }
                
                // rotation
                else if(transformationInfo[0] == 'r')
                {
                    center = rotations[tIndex].transform(center);
                    
                    transformation += rotations[tIndex];
                }
                // translation
                else if(transformationInfo[0] == 't')
                {
                    // translate center
                    center = translations[tIndex].transform(center);
                    transformation += translations[tIndex];
                }
            }
        }
        stream.clear();
        
        Sphere * sphere = new Sphere(center, radius, materials[material_id - 1], texturePtr);
        
        surfaces.push_back((Surface*)(sphere));

        element = element->NextSiblingElement("Sphere");
    }       
    
}






















