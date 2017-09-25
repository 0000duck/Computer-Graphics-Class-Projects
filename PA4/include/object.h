#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"

class Object
{
  public:
    Object();
    Object(char * objectFile);
    ~Object();
    void Update(unsigned int dt);
    void Render();
    void ReverseRotation();
    void ReverseSpin();
    void PauseRotation();
    void PauseSpin();
    void SetPlanetOrbiting(Object * planet);

    glm::mat4 GetModel();
    glm::mat4 GetTranslationMatrix();

  private:
    glm::mat4 model;
    glm::mat4 origin;
    glm::mat4 translationMatrix;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    float angle;
	  float translation;
    float spinSpeed;
    float rotationSpeed;
    float radius;
    int spinDirection;
    int rotationDirection;
    bool isRotating;
    bool isSpinning;
    bool isMoon;
    Object * m_planet;
};

#endif /* OBJECT_H */
