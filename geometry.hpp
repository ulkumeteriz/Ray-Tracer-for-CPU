#ifndef __GEOMETRY__H__
#define __GEOMETRY__H__

#include <vector>
#include <string>
#include <iostream>

typedef enum Interpolation { nearest, bilinear } Interpolation;
typedef enum DecalMode { replace_kd, blend_kd, replace_all } DecalMode;
typedef enum Appearance { repeat, clamp } Appearance;

class Position3;
class Vector3;
class Ray;
class Camera;
//class Matrix;
class Triangle;
struct Vec4f;
struct PointLight;
struct Material;
struct Face;
struct Mesh;
struct Sphere;
struct HitInfo;
struct Texture;
struct TexCoord;
class Surface;

typedef struct TexCoord
{
    int u;
    int v;
} TexCoord;


class Position3
{
    private:
        float x, y, z;
        
    public:
        Position3(float x = 0.0, float y = 0.0, float z = 0.0)
            : x(x), y(y), z(z) {}
        
        float getX() const;
        float getY() const;
        float getZ() const;
        
        void setX(float x);
        void setY(float y);
        void setZ(float z);
        
        // substraction of two points yields a vector
        Vector3 operator-(const Position3 & rhs) const;
        
        // a vector from this to rhs
        Vector3 to(const Position3 & rhs) const;
        
        float distanceSquare(const Position3 & rhs) const;

        friend class Scene;
        
        friend std::ostream &operator<<(std::ostream &output, const Position3 & position);
};

class Vector3
{
    private:
        float x, y, z;
        
    public:
        Vector3(float x = 0.0, float y = 0.0, float z = 0.0)
            : x(x), y(y), z(z) {}

        float getX() const;
        float getY() const;
        float getZ() const;
        
        void setX(float x);
        void setY(float y);
        void setZ(float z);
        
        // unary (-) operator
        Vector3 operator-() const;
        
        // substraction
        Vector3 operator-(const Vector3 & rhs) const;
        
        // addition
        Vector3 operator+(const Vector3 & rhs) const;
        
        // dot product
        float operator^(const Vector3 & rhs) const;
        
        // cross product
        Vector3 operator*(const Vector3 & rhs) const;
        
        // normalize: make it unit vector, then, return self
        Vector3 & normalize();
        
        // scalar multiplication
        Vector3 operator*(float rhs) const;
        
        // scalar division
        Vector3 operator/(float rhs) const;
        
        // get norm
        float getNorm() const;
        
        // intensify
        Vector3 intensify(const Vector3 & intensityVector) const;
        
        // generate a vector with different direction
        Vector3 generateDifferentlyDirectedVector() const;
        
        friend class Scene;
        
        friend std::ostream &operator<<(std::ostream &output, const Vector3 & vector);
};

class Ray
{
    private:
        Position3 origin;
        Vector3 direction;
        
    public:
        Ray() {
            this->origin = Position3();
            this->direction = Vector3();
        }
        
        Ray(Position3 origin, Vector3 direction)
            : origin(origin), direction(direction) { this->direction.normalize(); };
            
            
        Position3 getOrigin() const { return this->origin; }
        Vector3 getDirection() const { return this->direction; }
        
        void setOrigin(Position3 position) { this->origin = position; }
        void setDirection(Vector3 direction) { this->direction = direction.normalize(); }
        
        Ray createReflectionRay(const HitInfo &) const;
        Position3 getPoint(const float & t) const;
        
        bool getClosestHit(Material & material, // return the material of the hit object
                           HitInfo & hitInfo,   // return the hit info
                           const std::vector<Surface*> surfaces,
                           float epsilon); // feed surfaces
                           
        float getTValue(const Position3 & hitPosition) const;
       
};


struct Vec4f
{
    float x, y, z, w;
};

class Camera
{
    private:
        Position3 position;
        Vector3 gaze;
        Vector3 up;
        Vec4f near_plane;
        float near_distance;
        int image_width, image_height;
        std::string image_name;
        Ray** rays;
        
        void deleteRays();

    public:
        ~Camera(); // destruct rays
        
        Ray** getRays() const; // width x height
        int getImageW() const;
        int getImageH() const;
        std::string getImageName() const;
        
        void generateRays();
        
        friend class Scene;
};

struct PointLight
{
    Position3 position;
    Vector3 intensity;
};

struct Material
{
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;
    Vector3 mirror;
    float phong_exponent;
};


/*class Matrix
{
    public:
        static Vector3 getDeterminantIntermediateVector(const Vector3 & a, const Vector3 & b);
        static float getDeterminant(const Vector3 & a, const Vector3 & b, const Vector3 & c);
        static float getDeterminantByUsingIntermediateVector(const Vector3 & intermediateVector, const Vector3 & c);
};*/

// abstract class for surfaces that can be hit by a ray
class Surface
{
    protected:
        const Material & material;
        Texture* texture;
        Surface(const Material& material, Texture* texture) : material(material), texture(texture) {}

        
    public:
        // returns true if ray hits the surface and records the hit position
        // .. in hitPosition object
        virtual bool hit(const Ray & ray, HitInfo & hitInfo) const = 0;
        
        const Material& getMaterial() const
        {
            return this->material;
        } 
};

// TODO: positions are not deleted

class Triangle : public Surface
{
    private:
        const Position3 * vertex[3];
        const Vector3 normal;
        const TexCoord* texCoordData[3];
        
        void fillLookUpTable();
        
        struct LookUpTable
        {
            // A: vertex[0], B: vertex[1], C: vertex[2]
            float Ax_Bx;
            float Ay_By;
            float Az_Bz;
            float Ax_Cx;
            float Ay_Cy;
            float Az_Cz;
            Vector3 A_C;
            Vector3 A_B;

        };
        
        LookUpTable lookUpTable;
        
    public:
        Triangle( const Material & material,
                  Texture* texture,
                  const Position3 & vertex0,
                  const Position3 & vertex1,
                  const Position3 & vertex2,
                  TexCoord* t0,
                  TexCoord* t1,
                  TexCoord* t2 )
                     : Surface(material, texture),
                       vertex{&vertex0, &vertex1, &vertex2},
                       normal(computeNormal(vertex0, vertex1, vertex2)),
                       texCoordData{t0,t1,t2}
                       {
                            fillLookUpTable();
                       }
        
        Position3 getVertex(int vertexId) const;
        Vector3 getNormal() const;
    
        static Vector3 computeNormal(const Position3 & vertex0,
                                     const Position3 & vertex1,
                                     const Position3 & vertex2 );
                                     
        //bool isIntersecting(const Ray & ray) const;
                                     
        friend std::ostream &operator<<(std::ostream &output, const Triangle & triangle);
        
        bool hit(const Ray & ray, HitInfo & hitInfo) const;
};

class Sphere : public Surface
{
    private:
        Position3 center;
        float radius;
        
    public:
        Position3 getCenter() const { return this->center; }
        float getRadius() const { return this->radius; }
        
        Sphere(Position3 center, float radius, const Material & material, Texture* texture)
            : Surface(material, texture), center(center), radius(radius) {}
        
        bool isIntersecting(const Ray & ray) const;
        
        float discriminant(const Ray & ray) const;
       
       bool hit(const Ray & ray, HitInfo & hitInfo) const;
};

struct HitInfo
{
    Vector3 normal;
    Position3 hitPosition;
    Vector3 textureColor;
    DecalMode decalMode;
    bool hasTexture;
    float t;
};

struct Texture
{
    Interpolation interpolation;
    DecalMode decalMode;
    Appearance appearance;
    unsigned char* image = NULL;
    int width;
    int height;
};


#endif
