#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "particleSys.h"
#include "GLSL.h"

using namespace std;

particleSys::particleSys(const ParticleSystemConfig& systemConfig) {
  // Initialize the particle system
  config = systemConfig;
  numP = config.numParticles;
  t = 0.0f;
  h = config.timeStep;
  g = config.gravity;
  emitterPos = config.emitterPosition;
  theCamera = glm::mat4(1.0);

  // Allocate memory for the particles
  // points = new GLfloat[numP * 3];
  // pointColors = new float[numP * 4];
  points.resize(numP * 3);
  pointColors.resize(numP * 4);
}

particleSys::~particleSys() {
  // Clean up the particles
  // delete[] points;
  // delete[] pointColors;
}

void particleSys::gpuSetup() {

  cout << "Emitter position: " << emitterPos.x << " " << emitterPos.y << " " << emitterPos.z << endl;
	for (int i=0; i < numP; i++) {
		points[i*3+0] = emitterPos.x;
		points[i*3+1] = emitterPos.y;
		points[i*3+2] = emitterPos.z;

		auto particle = make_shared<Particle>(config.particleConfig);
		particles.push_back(particle);
		particle->load(0.0f, emitterPos, config.particleConfig);
	}

	//generate the VAO
   glGenVertexArrays(1, &vertArrObj);
   glBindVertexArray(vertArrObj);

   //generate vertex buffer to hand off to OGL - using instancing
   glGenBuffers(1, &vertBuffObj);
   //set the current state to focus on our vertex buffer
   glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
   //actually memcopy the data - only do this once
   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numP * 3, points.data(), GL_STREAM_DRAW);

   glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numP * 4, pointColors.data(), GL_STREAM_DRAW);

    assert(glGetError() == GL_NO_ERROR);
}

void particleSys::reSet() {
	for (int i=0; i < numP; i++) {
		particles[i]->load(0.0f, emitterPos, config.particleConfig);
	}
}

void particleSys::drawMe(std::shared_ptr<Program> prog) {

 	glBindVertexArray(vertArrObj);
	int h_pos = prog->getAttribute("vertPos");
  GLSL::enableVertexAttribArray(h_pos);
  //std::cout << "Any Gl errors1: " << glGetError() << std::endl;
  glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
  //std::cout << "Any Gl errors2: " << glGetError() << std::endl;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(0, 1);
  glVertexAttribDivisor(1, 1);
  // Draw the points !
  glDrawArraysInstanced(GL_POINTS, 0, 1, numP);

  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);

  int h_col = prog->getAttribute("vertColor");
  GLSL::enableVertexAttribArray(h_col);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(h_col, 1);

	glDisableVertexAttribArray(0);

}

void particleSys::update() {

  vec3 pos;
  vec4 col;

  //update the particles
  for(auto particle : particles) {
        particle->update(t, h, g, emitterPos, config.particleConfig);
  }
  t += h;

  // Sort the particles by Z
  //temp->rotate(camRot, vec3(0, 1, 0));
  //be sure that camera matrix is updated prior to this update
  vec3 s, translation, sk;
  vec4 p;
  quat r;
  glm::decompose(theCamera, s, r, translation, sk, p);
  sorter.C = glm::toMat4(r);
  sort(particles.begin(), particles.end(), sorter);


  //go through all the particles and update the CPU buffer
   for (int i = 0; i < numP; i++) {
        pos = particles[i]->getPosition();
        col = particles[i]->getColor();
        points[i*3+0] =pos.x;
        points[i*3+1] =pos.y;
        points[i*3+2] =pos.z;
			/*  To do - how can you integrate unique colors per particle?
        pointColors[i*4+0] =col.r + col.a/10;
        pointColors[i*4+1] =col.g + col.g/10;
        pointColors[i*4+2] =col.b + col.b/10;
        pointColors[i*4+3] =col.a;
			*/

      // pointColors[i*4+0] =col.r + col.a/10;
      // pointColors[i*4+1] =col.g + col.g/10;
      // pointColors[i*4+2] =col.b + col.b/10;
      pointColors[i*4+0] = col.r;
      pointColors[i*4+1] = col.g;
      pointColors[i*4+2] = col.b;
      pointColors[i*4+3] =col.a;
  }

  //update the GPU data
   glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
  //  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*numP*3, NULL, GL_STREAM_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*numP*3, points.data());
   glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  //  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*numP*4, NULL, GL_STREAM_DRAW);
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*numP*4, pointColors.data());
   glBindBuffer(GL_ARRAY_BUFFER, 0);



}
