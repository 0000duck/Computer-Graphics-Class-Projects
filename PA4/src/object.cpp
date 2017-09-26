#include "object.h"
#include <math.h>
#include <fstream>

using namespace std;

Object::Object(char * objectFile) {
  if(!LoadModel(objectFile)) {
    cout << "Unable to load model" << endl;
  }
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
}

Object::~Object()
{
  Vertices.clear();
  Indices.clear();
}

bool Object::LoadModel(char * objectFile) {
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
}

void Object::Update(unsigned int dt)
{
  model = origin;
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
