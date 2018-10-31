#ifndef __MATRIX4_H__
#define __MATRIX4_H__

#include "geometry.hpp"
#include <iostream>

using namespace std;
// 
class Matrix4
{
    private:
        float** matrixArray;
        
        void releaseMemory()
        {
            if(matrixArray != NULL)
            {
                for(int i = 0; i < 4; i++)
                {
                    if(matrixArray[i] != NULL)
                        delete[] matrixArray[i];
                    
                    matrixArray[i] = NULL;
                }
            
                delete[] matrixArray;
            }
            
            matrixArray = NULL;
        }
        
    public:
        float** getMatrixArray() const
        {
            return this->matrixArray;
        }
        
        // default constructor
        Matrix4()
        {
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    // initialize as identity matrix
                    matrixArray[i][j] = (i * 4 + j) % 5 ? 0 : 1;
                }
            }
        }
        
        // constructor
        Matrix4(float** matrixParam)
        {
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {
                if(matrixParam[i] == NULL)
                    throw "Null pointer while constructing Matrix4";
                    
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    // initialize as identity matrix
                    matrixArray[i][j] = matrixParam[i][j];
                }
            }
        }
        
        
        Matrix4(float matrixParam[4][4])
        {
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {        
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    // initialize as identity matrix
                    matrixArray[i][j] = matrixParam[i][j];
                }
            }
        }
        
        // constructor
        Matrix4(float* matrixParam)
        {
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    // initialize as identity matrix
                    matrixArray[i][j] = matrixParam[i * 4 + j];
                }
            }
        }
        
        // distructor
        ~Matrix4()
        {
            releaseMemory();
        }
        
        // copy constructor
        Matrix4(const Matrix4 & rhs)
        {
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    matrixArray[i][j] = rhs.matrixArray[i][j];
                }
            }
            
        }
        
        // assignment operator
        Matrix4 & operator=(const Matrix4 & rhs)
        {
            releaseMemory();
            
            matrixArray = new float*[4];
            
            for(int i = 0; i < 4; i++)
            {
                matrixArray[i] = new float[4];
                
                for(int j = 0; j < 4; j++)
                {
                    matrixArray[i][j] = rhs.matrixArray[i][j];
                }
            }
            
            return *this;
        }
        
        // matrix multiplication
        Matrix4 operator*(const Matrix4 & rhs) const
        {
            const Matrix4 & lhs = *this;
            
            Matrix4 result;
            
            for(int i = 0; i < 4; i++)
            {
                for(int j = 0; j < 4; j++)
                {
                    float sum = 0.0f;
                    
                    for(int index = 0; index < 4; index++)
                    {
                        sum += lhs.matrixArray[i][index] * rhs.matrixArray[index][j];
                    }
                    
                    result.matrixArray[i][j] = sum;
                }
            }
            
            return result;
        }
        
        // Matrix4 * Vector3
        Vector3 operator*(const Vector3 & rhs) const
        {
            float x = matrixArray[0][0] * rhs.getX() +
                      matrixArray[0][1] * rhs.getY() +
                      matrixArray[0][2] * rhs.getZ();
                      
            float y = matrixArray[1][0] * rhs.getX() +
                      matrixArray[1][1] * rhs.getY() +
                      matrixArray[1][2] * rhs.getZ();

            float z = matrixArray[2][0] * rhs.getX() +
                      matrixArray[2][1] * rhs.getY() +
                      matrixArray[2][2] * rhs.getZ();
                      
            return Vector3(x, y, z);
        }
        
        // Matrix4 * Position3
        Position3 operator*(const Position3 & rhs) const
        {
            float x = matrixArray[0][0] * rhs.getX() +
                      matrixArray[0][1] * rhs.getY() +
                      matrixArray[0][2] * rhs.getZ() +
                      matrixArray[0][3];
                      
            float y = matrixArray[1][0] * rhs.getX() +
                      matrixArray[1][1] * rhs.getY() +
                      matrixArray[1][2] * rhs.getZ() +
                      matrixArray[1][3];

            float z = matrixArray[2][0] * rhs.getX() +
                      matrixArray[2][1] * rhs.getY() +
                      matrixArray[2][2] * rhs.getZ() +
                      matrixArray[2][3];
                      
            return Position3(x, y, z);
        }
        
        float* operator[](int rowIndex) const
        {
            return matrixArray[rowIndex];
        } 
        
};

#endif
