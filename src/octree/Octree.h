#pragma once
#include "ofMain.h"
#include "../utils/box.h"
#include "../utils/ray.h"


class TreeNode {
public:
	Box box;
	vector<int> points;
	vector<TreeNode> children;
};

class Octree {
public:
	
	void create(const ofMesh & mesh, int numLevels);
	void subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level);
	bool intersect(const ofVec3f &, const TreeNode & node, TreeNode & nodeRtn);
	bool intersect(const Ray &, const TreeNode &, vector<TreeNode> &);
	bool intersect(const Ray &, const TreeNode &, TreeNode &);

	void draw(TreeNode & node, int numLevels, int level);
	void draw(int numLevels, int level) {
		draw(root, numLevels, level);
	}
	void drawLeafNodes(TreeNode & node);
	void drawLeafNodes() { drawLeafNodes(root); };
	static void drawBox(const Box &box);
	static Box meshBounds(const ofMesh &);
	int getMeshPointsInBox(const ofMesh &mesh, const vector<int> & points, Box & box, vector<int> & pointsRtn);
	void subDivideBox8(const Box &b, vector<Box> & boxList);

	ofMesh mesh;
	TreeNode root;
	

	const ofColor colors[10]{ ofColor::red, ofColor::green, ofColor::blue, ofColor::yellow,
							ofColor::purple, ofColor::white, ofColor::orange, ofColor::brown,
							ofColor::black, ofColor::pink };
};