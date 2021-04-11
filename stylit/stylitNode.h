#pragma once

#include "stylit.h"
#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>

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
	static MObject source; // Folder of LPEs
	static MObject style; // Selected target style

	Stylit stylit;
};