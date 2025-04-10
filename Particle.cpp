//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(const ParticleConfig& config) {
	// Constructor initialization with default values
	charge = 1.0f;
	m = 1.0f;
	d = 0.01f;
	x = vec3(0.0f);
	v = vec3(0.0f);
	lifespan = 1.0f;
	tEnd = 0.0f;
	scale = 1.0f;
	startColor = config.startColor;
	endColor = config.endColor;
	color = startColor;

}

Particle::~Particle()
{
}

void Particle::load(float initialTime, vec3 emitterPos, const ParticleConfig& config)
{
	float startTime = config.staggerInitialLifespan ?
		randFloat(config.initialTimeMin, config.initialTimeMax) :
		initialTime;

	rebirth(startTime, emitterPos, config);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 emitterPos, const ParticleConfig& config)
{
	// Physical properties
	charge = config.charge;
	m = config.randMass ? randFloat
		(config.minMass, config.maxMass) : config.minMass;

	d = config.randDamping ? randFloat
		(config.minDamping, config.maxDamping) : config.minDamping;

	// Position properties
	x = config.randPosition ? vec3(
		randFloat(config.minPosition.x, config.maxPosition.x),
		randFloat(config.minPosition.y, config.maxPosition.y),
		randFloat(config.minPosition.z, config.maxPosition.z)
	) : config.minPosition;

	x += emitterPos + config.positionOffset;

	v = config.randVelocity ? vec3(
		randFloat(config.minVelocity.x, config.maxVelocity.x),
		randFloat(config.minVelocity.y, config.maxVelocity.y),
		randFloat(config.minVelocity.z, config.maxVelocity.z)
	) : config.minVelocity;

	lifespan = config.randLifespan ? randFloat
		(config.minLifespan, config.maxLifespan) : config.minLifespan;

	tEnd = t + lifespan;

	scale = config.randScale ? randFloat
		(config.minScale, config.maxScale) : config.minScale;

	startColor = config.randColor ? vec4(
		randFloat(config.startColor.r, config.endColor.r),
		randFloat(config.startColor.g, config.endColor.g),
		randFloat(config.startColor.b, config.endColor.b),
		randFloat(config.startColor.a, config.endColor.a)
	) : config.startColor;

	endColor = config.endColor;
	color = startColor;

}

void Particle::update(float t, float h, const vec3 &g, const vec3 emitterPos, const ParticleConfig& config)
{
	if (t > tEnd) {
		rebirth(t, emitterPos, config);
		return;
	}

	// Physics
	vec3 acceleration = vec3(0);

	acceleration += config.constantAcceleration;
	acceleration -= config.velocityDamping * v;
	acceleration += config.gravityMultiplier * g;

	v += h * acceleration;
	x += h * v;

	// update visual properties
	float lifeRatio = (tEnd - t) / lifespan;

	if (config.fadeAlpha) {
		color = vec4(
			mix(endColor.r, startColor.r, lifeRatio),
			mix(endColor.g, startColor.g, lifeRatio),
			mix(endColor.b, startColor.b, lifeRatio),
			config.fadeAlpha ? mix(endColor.a, startColor.a, lifeRatio) : startColor.a
		);
	} else {
		color = startColor;
		color.a = 0.5f + 0.5f * sin(lifeRatio * M_PI);
	}

}
