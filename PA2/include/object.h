#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"

class Object
{
  public:
    Object();
    ~Object();
    void Update(unsigned int dt);
    void Render();
    void ReverseRotation();
    void ReverseSpin();
    void PauseRotation();
    void PauseSpin();

    glm::mat4 GetModel();

  private:
    glm::mat4 model;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    float angle;
	  float translation;
    int spinDirection;
    int rotationDirection;
    bool isRotating;
    bool isSpinning;
};

#endif /* OBJECT_H */
