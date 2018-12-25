
// author: < Yu Jun Zhao > 
//  Date: <11/18/2018>

/*
	Press O for octree
	Press L for just the leaf nodes
	Even if a octree leaf node does contain more than one point (due to lack of levels), 
	the sphere will only be drawn at the location of the first point of the the leaf box.

	
	// MainCam is F1
	// FrontCam is F2
	// bottomCam is F3
	// TrackCam is F4
	
	//Controls----
	Array keys:
	Up for +y
	Down for -y
	Left for -x
	Right for +x
	Ctrl up for +z
	Ctrl down for -z

	H for hanging
	hanging must be enabled to use Mouse control
	click on mesh, the lander will head to that dir.

*/


#include "ofApp.h"
#include "utils/Util.h"



//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	// Load Font
	if (!menuFont.load(fontPath, 100)) cout << "missing font" << endl;


		// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load shader
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
#endif

	// load sound
	if (!thrusterSound.load("sound/rocket-thrust-effect.wav")) {
		cout << "no sound file found" << endl;
		ofExit();
	}

	// load light
	pointLight.setup();
	pointLight.enable();
	pointLight.setPointLight();
	pointLight.setPosition(0, 100, 0);
	pointLight.setDiffuseColor(ofColor::white);
	pointLight.setSpecularColor(ofColor::white);

	// Load cam
	mainCam.setPosition(39.45, 8.96, 29.37);
	mainCam.setNearClip(.1);
	mainCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	mainCam.lookAt(glm::vec3(-0.32,-0.66,-0.67), glm::vec3(0,1,0));
	ofSetVerticalSync(true);
	mainCam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	frontCam.setPosition(0, 0, 0);
	frontCam.lookAt(glm::vec3(0,0,-1));
	bottomCam.setNearClip(0.3);

	bottomCam.setPosition(0, 0, 0);
	bottomCam.lookAt(glm::vec3(0, -1, 0));
	bottomCam.setNearClip(0.1);

	trackCam.setPosition(0, 10, -80);
	trackCam.lookAt(bottomCam);
	trackCam.setNearClip(0.1);

	theCam = &mainCam;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	if (moon.loadModel(moonPath)) {
		cout << "loading moon model" << endl;
		moon.setRotation(0, 180, 0, 0, 1);
		moon.setScaleNormalization(false);
		cout << "complete loading moon model" << endl;

		cout << "creating octree" << endl;
		octree.create(moon.getMesh(0), levels);
		cout << "complete creating octree" << endl;
	}
	else {
		cout << "Error Can't load moon model" << endl;
		ofExit(0);
	}

	// load lander
	if (lander.loadModel(landerPath)) {
		cout << "loading lander" << endl;
		lander.setScaleNormalization(false);
		
		lander.setScale(1, 1, 1);
		lander.setPosition(-110,35,0);
		lander.setRotation(0, 180, 0, 0, 1);

		bRoverLoaded = true;

		cout << "complete loading lander" << endl;
		cout << "lander position: " << lander.getPosition() << endl;
	}
	else {
		cout << "Error Can't load lander model" << endl;
		ofExit(0);
	}
	// Set up ship particle system
	shipsys = new ParticleSystem();
	core = new Particle();
	core->lifespan = INFINITY;
	core->mass = 1;
	core->position = lander.getPosition();
	shipsys->add(*core);
	shipsys->addForce(new TurbulenceForce(turbMin, turbMax));
	// Since add/push_back creates a copy of the particle, I need to reset what it points to.
	core = &(shipsys->particles[0]);

	// create emitter and add forces
	shipsys->addForce(new Thruster());
	shipsys->addForce(new GravityForce(gravityF));
	shipsys->addForce(new ImpulseForce()); // for stopping the lander // at index 3
	shipsys->addForce(new GravityForce(zeroVec));
	shipsys->addForce(new Thruster());
	shipsys->addForce(new Thruster());
	turbulanceForce = (TurbulenceForce *)(shipsys->forces[0]);
	thrusterForce = (Thruster *)(shipsys->forces[1]); 
	moonGravity = (GravityForce *)(shipsys->forces[2]);
	impulseForce = (ImpulseForce *)(shipsys->forces[3]);
	supportGravityForce = (GravityForce *)(shipsys->forces[4]);
	hangingForce = (Thruster *)(shipsys->forces[5]);
	autoPilotForce = (Thruster *)(shipsys->forces[6]);

	emitter = new ParticleEmitter();
	emitter->setEmitterType(DiscEmitter);
	emitter->setOneShot(true);
	emitter->setGroupSize(50);
	emitter->particleColor = ofColor::yellow;
	emitter->particleRadius = 0.02f;
	emitter->setLifespanRange(ofVec2f(0.1, 0.5));
	emitter->setRandomLife(true);
	emitter->setVelocity(ofVec3f(0, 7, 0));
	emitter->sys->addForce(new TurbulenceForce(ofVec3f(-5, 0, -5), ofVec3f(5, 0, 5)));

	// by default set to full screen
	ofSetFullscreen(true);

}

//--------------------------------------------------------------

void ofApp::update() {
	if (isGameStart) {
		frontCam.setPosition(glm::vec3(core->position.x, core->position.y + 2.0f, core->position.z));
		bottomCam.setPosition(core->position);
		trackCam.lookAt(bottomCam, glm::vec3(0,1,0));
		lander.setPosition(core->position.x, core->position.y, core->position.z);

		emitter->setPosition(ofVec3f(core->position.x, core->position.y + 0.5f, core->position.z));

		if (startEmitter) {
			emitter->start();
		}
		else {
			emitter->stop();
		}

		// calculate altitude
		// might combine collision calculation and altitude calculation to one calculation
		Ray ray = Ray(Vector3(core->position.x, core->position.y, core->position.z),
			Vector3(0, -1, 0)); // since it always points down
		TreeNode altitudeNode;
		if (octree.intersect(ray, octree.root, altitudeNode)) {
			altitude = glm::length(octree.mesh.getVertex(altitudeNode.points[0]) - glm::vec3(core->position));

		}

		TreeNode intersectedNode;
		//turbulanceForce->set(zeroVec, zeroVec);

		// lander touches the ground
		if (!completeStopped && octree.intersect(core->position, octree.root, intersectedNode)) {
			groundTouched = true;
			//cout << "intersected" << endl;
			glm::vec3 vec = glm::vec3(core->velocity);

			impulseForce->set(ofGetFrameRate() * -1 * vec,
				octree.mesh.getNormal(intersectedNode.points[0]), materialRestitution);

			turbulanceForce->set(zeroVec, zeroVec);

			//supportGravityForce->set(OppositeGravityF);
		}
		else if(completeStopped){
			groundTouched = true;
		}
		else {
			groundTouched = false;
			// HANGING
			// thrustforce equals to gravity so that it stays in space.
			if (hanging) {
				//bruteforce approach
				core->velocity.y = 0;
				hangingForce->set(ofVec3f(0, 1, 0), moonGravityMag);
				
				if (b_selectedNode) {
					
					//core->reset();
					ofVec3f toSelected = selectedVertex - core->position;
					float distanceXZ = ofVec2f(toSelected.x, toSelected.z).length();
					ofVec3f dir = toSelected.normalize();
					//cout << distanceXZ << endl;
					if (distanceXZ >= 0 && distanceXZ <= 2) {
						core->reset();
						autoPilotForce->set(zeroVec, 0);

					}
					else {
						autoPilotForce->set(ofVec3f(dir.x, 0, dir.z), autoPilotMag);
					}
				}
				else {
					autoPilotForce->set(zeroVec, 0);

				}
			}
			else {
				hangingForce->set(zeroVec, 0);
			}
			
		}

		// If below a certain altitude it is "touching" the ground.
		if (groundTouched) {
			supportGravityForce->set(OppositeGravityF);
		}
		else {
			supportGravityForce->set(zeroVec);
		}

		emitter->update();
		shipsys->update();

		// Since the velocity will always not equal to 0
		// it is necessary to specify what zero is.
		//cout << core->velocity << endl;
		//cout << "groundTouched: " << groundTouched << "   ";
		//cout << "completeStopped: " << completeStopped << endl;
		playRocketThrusterEffect();

		if (groundTouched && (core->velocity.y <= 0.025f && core->velocity.y >= -0.025f)) {
			//cout << "velocity: " << core->velocity.y << endl;
			shipsys->toggleOnOff(false);
			
			completeStopped = true;
		}
		
		
	}
}
//--------------------------------------------------------------
void ofApp::draw(){
	

	ofBackground(ofColor::black);

	theCam->begin();
	ofPushMatrix();
	

	//-------------------------
	//pointLight.draw();

	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		moon.drawWireframe();
		if (bRoverLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		moon.drawFaces();

		if (bRoverLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		moon.drawVertices();
	}

	ofNoFill();
	//ofSetColor(ofColor::white);
	
	if (bdrawOctree) {
		octree.draw(drawlevels,0);
	}
	else if (bdrawLeaf) {
		octree.drawLeafNodes();
	}

	// draw the sphere
	if (b_selectedNode) {
		ofSetColor(ofColor::yellow);
		ofDrawSphere(selectedVertex, 5);
	}

	//------------------------
	//draw the emitter particle

	loadVbo();
	ofSetColor(ofColor::yellow);

	glDepthMask(GL_FALSE);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();
	ofEnableAlphaBlending();

	shader.begin();
	
	//emitter->draw();
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter->sys->particles.size());
	particleTex.unbind();

	shader.end();
	glDepthMask(GL_TRUE);

	ofDisablePointSprites();
	ofDisableBlendMode();
	ofDisableAlphaBlending();

	ofPopMatrix();

	

	theCam->end();

  
	string str; 
	if (!isGameStart) {
		// Draw some states
		str = "PRESS SPACEBAR TO START";
		ofSetColor(ofColor::yellow);
		menuFont.drawString(str, 30, ofGetWindowHeight() / 2 - 50);
	}
	
	
	//str = "Frame Rate: " + std::to_string(ofGetFrameRate());
	str = "Altitude: " + std::to_string(altitude);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);


}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	
	case 'l':
		bdrawLeaf = !bdrawLeaf;
		break;
	case 'o':
		bdrawOctree = !bdrawOctree; // toggle for draw octree
		break;
	case 'r':
		mainCam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		mainCam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	// lander control
	case OF_KEY_UP:
		if (bCtrlKeyDown && (!groundTouched || !completeStopped || !hanging)) thrusterForce->set(ofVec3f(0, 0, 1), thrusterMag);
		else if (!hanging) {
			thrusterForce->set(ofVec3f(0, 1, 0), thrusterMag);

			startEmitter = true;
		}
		groundTouched = false; 
		completeStopped = false;
		shipsys->toggleOnOff(true); // turn on system
		break;
	case OF_KEY_DOWN:
		if (bCtrlKeyDown && (!groundTouched || !completeStopped || !hanging)) thrusterForce->set(ofVec3f(0, 0, -1), thrusterMag);
		else if(!groundTouched || !completeStopped) thrusterForce->set(ofVec3f(0, -1, 0), thrusterMag);
		break;
	case OF_KEY_LEFT:
		if (!groundTouched || !completeStopped || !hanging)
			thrusterForce->set(ofVec3f(-1, 0, 0), thrusterMag);
		break;
	case OF_KEY_RIGHT:
		if (!groundTouched || !completeStopped || !hanging)
			thrusterForce->set(ofVec3f(1, 0, 0), thrusterMag);
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'h':
		hanging = !hanging;
		startEmitter = hanging;
		b_selectedNode = false;
		core->reset();
		break;
	case 'i':
		
		break;
	case OF_KEY_F1:
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &frontCam;
		break;
	case OF_KEY_F3:
		theCam = &bottomCam;
		break;
	case OF_KEY_F4:
		theCam = &trackCam;
		break;
	case OF_KEY_ALT:
		mainCam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	// when released up arrow key, stop the emitter 
	case OF_KEY_UP:
		
		startEmitter = false;
		thrusterForce->set(ofVec3f(0, 0, 0), 0);
		
		break;
	case OF_KEY_DOWN:
		
		thrusterForce->set(ofVec3f(0, 0, 0), 0);
		break;
	case OF_KEY_LEFT:
		
		thrusterForce->set(ofVec3f(0, 0, 0), 0);
		break;
	case OF_KEY_RIGHT:
		
		thrusterForce->set(ofVec3f(0, 0, 0), 0);
		break;
	case ' ':
		if (!isGameStart) isGameStart = true;
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	// works only if left click
	if (hanging && button == 0) {
		ofVec3f mouse(mouseX, mouseY);
		ofVec3f rayPoint = theCam->screenToWorld(mouse);
		ofVec3f rayDir = rayPoint - theCam->getPosition();
		rayDir.normalize();
		Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
			Vector3(rayDir.x, rayDir.y, rayDir.z));
		float startTime = ofGetElapsedTimeMicros();
		vector<TreeNode> listOfIntersected;

		if (octree.intersect(ray, octree.root, listOfIntersected)) {
			b_selectedNode = true;

			// For selecting the closest point to the cam.
			float closest = INT_MAX;
			unsigned int closestIndex = 0;
			for (unsigned int i = 0; i < listOfIntersected.size(); i++) {
				glm::vec3 vertex = octree.mesh.getVertex(listOfIntersected[i].points[0]);
				float distance = glm::length(vertex - theCam->getPosition());
				//cout << "i: " << i << ", vertex: " << vertex << "distance from cam: " << distance << endl;
				if (closest > distance) {
					closest = distance;
					closestIndex = i;
				}

			}
			selectedVertex = octree.mesh.getVertex(listOfIntersected[closestIndex].points[0]);
		}
		else {

			b_selectedNode = false;
		}
	}
   
}



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}


// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), theCam->getZAxis(), point);

	if (lander.loadModel(dragInfo.files[0])) {
		cout << "loading lander" << endl;
		lander.setScaleNormalization(false);
		//lander.setScale(.005, .005, .005);
		lander.setScale(1,1,1);
		lander.setPosition(point.x, point.y, point.z);
		bRoverLoaded = true;

		cout << "complete loading lander" << endl;
		cout << "lander position: " << lander.getPosition() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	glm::vec3 mouse(mouseX, mouseY, 0);
	ofVec3f rayPoint = theCam->screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - theCam->getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}


// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	if (emitter->sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter->sys->particles.size(); i++) {
		points.push_back(emitter->sys->particles[i].position);
		sizes.push_back(ofVec3f(particleRadius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}


void ofApp:: playRocketThrusterEffect() {
	if (startEmitter) {
		if (!thrusterSound.isPlaying()) {
			thrusterSound.play();
		}
	}
	else {
		thrusterSound.stop();
	}
}

