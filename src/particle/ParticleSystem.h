#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void remove(int);
	void update();
	void toggleOnOff(bool);
	void setLifespan(float);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
	bool enabled = true;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
	ofVec3f gravity;
public:
	void set(const ofVec3f &g) { gravity = g; }
	GravityForce(const ofVec3f & gravity);
	void updateForce(Particle *);
	ofVec3f getForce() {
		return this->gravity;
	}
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;
public:
	void set(const ofVec3f &min, const ofVec3f &max) { tmin = min; tmax = max; }
	TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
	TurbulenceForce() { tmin.set(0, 0, 0); tmax.set(0, 0, 0); }
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude = 1.0;
	float height = .2;
public:
	void set(float mag) { magnitude = mag; }
	void setHeight(float h) { height = h; }
	ImpulseRadialForce(float magnitude);
	ImpulseRadialForce() {}
	void updateForce(Particle *);
};

class CyclicForce : public ParticleForce {
	float magnitude = 1.0;
public:
	void set(float mag) { magnitude = mag; }
	CyclicForce(float magnitude);  
	CyclicForce() {}
	void updateForce(Particle *);
};

// Thruster force should be greater than abs(gravity)
class Thruster : public ParticleForce {
	float magnitude = 0;
	ofVec3f direction;
public:
	void set(ofVec3f dir, float mag) { this->direction = dir; this->magnitude = mag; }

	Thruster(ofVec3f dir);
	Thruster(){}
	void updateForce(Particle *);

	ofVec3f getForce() {
		return ofVec3f(0,magnitude,0);
	}
};

// for stopping the lander
class ImpulseForce : public ParticleForce {
	ofVec3f force;
public:
	ImpulseForce() {
		applyOnce = true;
		applied = true;
		force = ofVec3f(0,0,0);
	}
	void set(const glm::vec3 f, const glm::vec3 normal, float materialRes) {
		
		force = (materialRes + 1) * (glm::dot(f, normal)) * normal;
		//cout << force << endl;
		applied = false;
	}

	void updateForce(Particle * particle);

	ofVec3f getForce() {
		return this->force;
	}
};

