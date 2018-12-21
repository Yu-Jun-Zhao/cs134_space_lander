#include "Particle.h"


Particle::Particle() {

	// initialize particle with some reasonable values first;
	//
	velocity.set(0, 0, 0);
	acceleration.set(0, 0, 0);
	position.set(0, 0, 0);
	forces.set(0, 0, 0);
	lifespan = 5;
	birthtime = 0;
	radius = .1;
	damping = .99;
	mass = 1;
	color = ofColor::aquamarine;
}

void Particle::draw() {
	ofSetColor(color);
//	ofSetColor(ofMap(age(), 0, lifespan, 255, 10), 0, 0);
	ofDrawSphere(position, radius);
}


void Particle::integrate() {

	// check for 0 framerate to avoid divide errors
	//
	float framerate = ofGetFrameRate();
	if (framerate < 1.0) return;

	// interval for this step
	//
	float dt = 1.0 / framerate;

	// update acceleration with accumulated paritcles forces
	// remember :  (f = ma) OR (a = 1/m * f)
	//
	ofVec3f accel = acceleration;    // start with any acceleration alrefady on the particle
	accel += (forces * (1.0 / mass));
	velocity += (accel * dt);

	
	// eventually damping causes the velocity to reach a terminal case.
	//velocity *= damping;

	// update position based on velocity
	//
	position += (velocity * dt);

	// clear forces on particle (they get re-added each step)
	//
	forces.set(0, 0, 0);
	//cout << position << endl;
}

void Particle::reset() {
	velocity.set(0, 0, 0);
	acceleration.set(0, 0, 0);
	forces.set(0, 0, 0);

}

//  return age in seconds
//
float Particle::age() {
	return (ofGetElapsedTimeMillis() - birthtime)/1000.0;
}


