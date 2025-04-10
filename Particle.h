//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ParticleConfig.h"

using namespace glm;

class MatrixStack;
class Program;
class Texture;

class Particle
{
public:
	Particle(const ParticleConfig& config);
	virtual ~Particle();
	void load(float initialTime, vec3 emitterPos, const ParticleConfig& config);
	void rebirth(float t, vec3 emitterPos, const ParticleConfig& config);
	void update(float t, float h, const vec3& g, const vec3 emitterPos, const ParticleConfig& config);
	const vec3 &getPosition() const { return x; };
	const vec3 &getVelocity() const { return v; };
	const vec4 &getColor() const { return color; };

private:
	float charge; // +1 or -1
	float m; // mass
	float d; // viscous damping
	vec3 x; // position
	vec3 v; // velocity
	float lifespan; // how long this particle lives
	float tEnd;     // time this particle dies
	float scale;
	vec4 startColor; // color
	vec4 endColor; // color
	vec4 color; // color
};

#endif // LAB471_PARTICLE_H_INCLUDED
