#pragma once

#include "stylit.h"
#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <opencv2/opencv.hpp>


class StylelitNode : public MPxNode {
public:
	StylelitNode() {};
	~StylelitNode() override {};
	MStatus compute(const MPlug& plug, MDataBlock& data) override;

	static  void* creator();
	static  MStatus initialize();

	static MObject	time;
	static MObject	outputMesh;
	static MTypeId	id;
	static MObject sourceBeauty; // Beauty
	static MObject sourceLDE; // LDE
	static MObject sourceLSE; // LSE
	static MObject sourceLDDE; // LDDE
	static MObject sourceLD12E; // LD12E
	static MObject style; // Selected target style
	static MObject pylevel; // Level of pyramid
	static string pluginPath; // Plugin Path

	static MObject neighborSize; // Neighbor Size
	static MObject miu; // miu value for penalty
	static MObject resolution; // Desired output
	static MObject width; // Output width
	static MObject height; // Ouput height

	Stylit stylit;
};