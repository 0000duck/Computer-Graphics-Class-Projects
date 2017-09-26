#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"

class Object
{
  public:
    Object(char * objectFile);
    ~Object();
    void Update(unsigned int dt);
    void Render();

    glm::mat4 GetModel();
    glm::mat4 GetTranslationMatrix();

  private:
    bool LoadModel(char * objectFile);
    glm::mat4 model;
    glm::mat4 origin;
    glm::mat4 translationMatrix;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    float angle;
    Object * m_planet;
};

#endif /* OBJECT_H */
