#include "object.h"
#include <math.h>
#include <fstream>

using namespace std;

Object::Object()
{
  /*
    # Blender File for a Cube
    o Cube
    v 1.000000 -1.000000 -1.000000
    v 1.000000 -1.000000 1.000000
    v -1.000000 -1.000000 1.000000
    v -1.000000 -1.000000 -1.000000
    v 1.000000 1.000000 -0.999999
    v 0.999999 1.000000 1.000001
    v -1.000000 1.000000 1.000000
    v -1.000000 1.000000 -1.000000
    s off
    f 2 3 4
    f 8 7 6
    f 1 5 6
    f 2 6 7
    f 7 8 4
    f 1 4 8
    f 1 2 4
    f 5 8 6
    f 2 1 6
    f 3 2 7
    f 3 7 4
    f 5 1 8
  */

  Vertices = {
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}
  };

  Indices = {
    2, 3, 4,
    8, 7, 6,
    1, 5, 6,
    2, 6, 7,
    7, 8, 4,
    1, 4, 8,
    1, 2, 4,
    5, 8, 6,
    2, 1, 6,
    3, 2, 7,
    3, 7, 4,
    5, 1, 8
  };

  // The index works at a 0th index
  for(unsigned int i = 0; i < Indices.size(); i++)
  {
    Indices[i] = Indices[i] - 1;
    cout << Indices[i] << endl;
  }

  for(int i = 0; i < Vertices.size(); i++)
  {
    cout << "Vertex: " << "(" << Vertices[i].vertex.x << ", " << Vertices[i].vertex.y << ", " << Vertices[i].vertex.z << ")" << endl;
    cout << "Color: " << "(" << Vertices[i].color.x << ", " << Vertices[i].color.y << ", " << Vertices[i].color.z << ")" << endl;
  }

  angle = 0.0f;

  glGenBuffers(1, &VB);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

  spinDirection = 1;
  rotationDirection = 1;
  isSpinning = true;
  isRotating = true;
  origin = glm::mat4(1.0);
  radius = 10;
  spinSpeed = 1;
  rotationSpeed = 1;
}

Object::Object(char * objectFile) {
  ifstream fin(objectFile);
  string line;
  while(!fin.eof()) {
    char firstLetter;
    fin >> ws;
		fin >> firstLetter;
    cout << "First Letter: " << firstLetter << endl;
    if(firstLetter == 'v') {
      float x, y, z;
      fin >> x;
      fin >> y;
      fin >> z;
      glm::vec3 tempVertex = glm::vec3(x, y, z);
      cout << "(" << tempVertex.x << ", " << tempVertex.y << ", " << tempVertex.z << ")" << endl;
      glm::vec3 tempColor = glm::vec3(x, y, z);
      Vertex temp = Vertex(tempVertex, tempColor);
      Vertices.push_back(temp);
    }else if(firstLetter == 'f') {
      unsigned int temp;
      fin >> temp;
      cout << "(" << temp;
      Indices.push_back(temp);
      fin >> temp;
      cout << ", " << temp;
      Indices.push_back(temp);
      fin >> temp;
      cout << ", " << temp << ")" << endl;
      Indices.push_back(temp);
    }else {
      string s;
      getline(fin, s);
    }
    fin >> ws;
	}
  fin.close();
  // The index works at a 0th index
  for(unsigned int i = 0; i < Indices.size(); i++)
  {
    Indices[i] = Indices[i] - 1;
    cout << Indices[i] << endl;
  }

  for(int i = 0; i < Vertices.size(); i++)
  {
    cout << "Vertex: " << "(" << Vertices[i].vertex.x << ", " << Vertices[i].vertex.y << ", " << Vertices[i].vertex.z << ")" << endl;
    cout << "Color: " << "(" << Vertices[i].color.x << ", " << Vertices[i].color.y << ", " << Vertices[i].color.z << ")" << endl;
  }

  angle = 0.0f;

  glGenBuffers(1, &VB);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

  spinDirection = 1;
  rotationDirection = 1;
  isSpinning = true;
  isRotating = true;
  origin = glm::mat4(1.0);
  radius = 10;
  spinSpeed = 1;
  rotationSpeed = 1;
}

void Object::SetPlanetOrbiting(Object * planet) {
  m_planet = planet;
  isMoon = true;
  spinSpeed = 0.5;
  rotationSpeed = 5;
  radius = 5;
}

Object::~Object()
{
  Vertices.clear();
  Indices.clear();
}

void Object::ReverseRotation() {
  rotationDirection *= -1;
  isRotating = true;
}

void Object::ReverseSpin() {
  spinDirection *= -1;
  isSpinning = true;
}

void Object::PauseRotation(){
  isRotating = !isRotating;
}

void Object::PauseSpin(){
  isSpinning = !isSpinning;
}

void Object::Update(unsigned int dt)
{
  float multiplier = 1000;
  if(isSpinning) {
    angle += dt * M_PI/1000 * spinSpeed * spinDirection;
  }

  if(isRotating) {
    translation += dt * M_PI/1000 * rotationSpeed * rotationDirection;
  }
  translationMatrix = glm::translate(glm::vec3(sin(translation/10) * radius, 0.0, cos(translation/10) * radius));

  if(isMoon) {
    translationMatrix *= m_planet->GetTranslationMatrix();
  }
  model = glm::rotate(translationMatrix * origin, (angle), glm::vec3(0.0, 1.0, 0.0));
}

glm::mat4 Object::GetModel()
{
  return model;
}

glm::mat4 Object::GetTranslationMatrix()
{
  return translationMatrix;
}

void Object::Render()
{
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

  glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}
