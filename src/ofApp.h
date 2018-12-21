#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "utils/box.h"
#include "utils/ray.h"
#include "Octree/Octree.h"
#include "particle/ParticleSystem.h"
#include "particle/ParticleEmitter.h"



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		void playRocketThrusterEffect();
		void loadVbo();


		//bool  doPointSelection(); // another way of selecting points

		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);

	

		ofEasyCam mainCam;
		ofCamera bottomCam;
		ofCamera frontCam;
		ofCamera trackCam;
		ofCamera *theCam;
		ofxAssimpModelLoader moon, lander;
		string moonPath = "geo/moon-houdini.obj";
		string landerPath = "geo/lander.obj";
		float altitude = 0;

		ofLight light;
		Box boundingBox;
		//vector<Box> level1, level2, level3;
	
		bool bAltKeyDown = false;
		bool bCtrlKeyDown = false;
		bool bWireframe = false;
		bool bDisplayPoints = false;
		
		//bool bPointSelected;
		
		bool bRoverLoaded = false;
		bool bTerrainSelected = true;
		bool bdrawOctree = false;
		bool bdrawLeaf = false;
		bool b_selectedNode = false;
		//ofVec3f selectedPoint;
		ofVec3f selectedVertex;

		// some bool for controls
		// two bool for checking collision
		bool groundTouched = false;
		bool completeStopped = false;
		bool hanging = false;

		const float selectionRange = 4.0;
		Octree octree;
		TreeNode selectedNode;

		// 8 levels for moon-houdini
		// 8 levels for moon-low-v1
		int levels = 8; 
		int drawlevels = 8;

		// Ship core
		ParticleSystem* shipsys = nullptr;
		Particle *core = nullptr;
		
		// emitter
		ParticleEmitter* emitter;
		bool startEmitter = false;
		//ofVec3f landerCenter; // emitter position

		// Forces
		float thrusterMag = 5.0f; // by default
		float autoPilotMag = 2.0f;
		float materialRestitution = 0.4f; // by default
		float moonGravityMag = 1.62f;
		ofVec3f zeroVec = ofVec3f(0,0,0);
		ofVec3f turbMin = ofVec3f(-0.7, -0.7, -0.7);
		ofVec3f turbMax = ofVec3f(0.7, 0.7, 0.7);
		ofVec3f gravityF = ofVec3f(0, -moonGravityMag, 0);
		ofVec3f OppositeGravityF = ofVec3f(0, moonGravityMag, 0);
		TurbulenceForce *turbulanceForce = nullptr;
		Thruster *thrusterForce = nullptr;
		GravityForce *moonGravity = nullptr;
		ImpulseForce *impulseForce = nullptr;
		GravityForce *supportGravityForce = nullptr; // the support force of the ground
		Thruster *hangingForce = nullptr;
		Thruster *autoPilotForce = nullptr;

		// Little Menu
		bool isGameStart = false;
		ofTrueTypeFont menuFont;
		string fontPath = "font/SpicyRice-Regular.ttf";

		// textures	
		ofTexture  particleTex;

		//shader 
		ofVbo vbo;
		ofShader shader;
		float particleRadius = 3.2;

		// sound
		ofSoundPlayer thrusterSound;

		// Light 
		ofLight pointLight;
		
};
