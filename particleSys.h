#pragma once
#ifndef __particleS__
#define __particleS__

#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"
#include "Program.h"
#include "ParticleConfig.h"

using namespace glm;
using namespace std;

class ParticleSorter {
public:
   bool operator()(const shared_ptr<Particle> p0, const shared_ptr<Particle> p1) const
   {
      // Particle positions in world space
      const vec3 &x0 = p0->getPosition();
      const vec3 &x1 = p1->getPosition();
      // Particle positions in camera space
      vec4 x0w = C * vec4(x0.x, x0.y, x0.z, 1.0f);
      vec4 x1w = C * vec4(x1.x, x1.y, x1.z, 1.0f);
      return x0w.z < x1w.z;
   }

   mat4 C; // current camera matrix
};

class particleSys {
private:
	vector<shared_ptr<Particle>> particles;
	float t, h; // Current time and time step
	vec3 g; // Gravity
	int numP; // Number of particles
	vec3 emitterPos; // Emitter position
	ParticleSystemConfig config;
	ParticleSorter sorter;

	// GPU related data
	// GLfloat* points; // Particle positions
	std::vector<GLfloat> points;
	mat4 theCamera; // Camera matrix
	unsigned vertArrObj, vertBuffObj; // Vertex Array Object, Vertex Buffer Object
	// float* pointColors; // Particle colors
	std::vector<float> pointColors;
	GLuint colorbuffer; // Color buffer

public:
	particleSys(const ParticleSystemConfig& system);
	~particleSys();
	void drawMe(std::shared_ptr<Program> prog);
	void gpuSetup();
	void update();
	void reSet();
	void setCamera(mat4 inC) {theCamera = inC;}

	void setEmitterPosition(const vec3& pos) { emitterPos = pos; }
	void setGravity(const vec3& grav) { g = grav; }
	void setParticleConfig(const ParticleConfig& pconfig) { config.particleConfig = pconfig; }
	void setTimeStep(float timeStep) { config.timeStep = timeStep; }
};


#endif
