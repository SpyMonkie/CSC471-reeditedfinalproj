#pragma once
#include <glm/glm.hpp>
using namespace glm;

// Configuration structure for particle properties
// default values set
struct ParticleConfig {
    // physical properties
    float charge = 1.0f; // +1 or -1
    bool randMass = false; // randomize mass otherwise use minMass
    float minMass = 1.0f; // mass
    float maxMass = 1.0f; // mass
    bool randDamping = false; // randomize damping otherwise use minDamping
    float minDamping = 0.01f; // viscous damping
    float maxDamping = 0.01f; // viscous damping

    // Position properties
    bool randPosition = false; // randomize position otherwise use minPosition
    vec3 minPosition = vec3(0.0f); // position
    vec3 maxPosition = vec3(0.0f); // position
    vec3 positionOffset = vec3(0.0f, 0.05f, 0.0f); // offset from emitter position

    // Velocity properties
    bool randVelocity = false; // randomize velocity otherwise use minVelocity
    vec3 minVelocity = vec3(-0.1f, 0.8f, -0.02f); // velocity
    vec3 maxVelocity = vec3(0.1f, 1.2f, 0.02f); // velocity

    // Lifespan properties
    bool randLifespan = false; // randomize lifespan otherwise use minLifespan
    float minLifespan = 1.0f; // how long this particle lives
    float maxLifespan = 1.0f; // how long this particle lives
    bool staggerInitialLifespan = false; // stagger the initial lifespan of particles
    float initialTimeMin = -5.0f; // minimum initial time
    float initialTimeMax = 0.0f;

    // Visual properties
    bool randScale = false; // randomize scale otherwise use minScale
    float minScale = 1.0f;
    float maxScale = 1.0f;
    bool randColor = false; // randomize color otherwise use startColor
    vec4 startColor = vec4(1.0f); // color
    vec4 endColor = vec4(1.0f); // color
    bool fadeAlpha = false; // fade alpha over time

    // Physics modifiers
    vec3 constantAcceleration = vec3(0.0f, 0.7f, 0.0f); // constant acceleration
    float velocityDamping = 0.05f;
    float gravityMultiplier = 0.1f;

};

struct ParticleSystemConfig {
    int numParticles = 300;
    float timeStep = 0.016f;
    vec3 gravity = vec3(0.0f, 0.0f, 0.0f);
    vec3 emitterPosition = vec3(0.0f);
    ParticleConfig particleConfig;
};
