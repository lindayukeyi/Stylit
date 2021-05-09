#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MDoubleArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MDGModifier.h>
#include <maya/MPlugArray.h>
#include <maya/MVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>
#include <list>

#include "stylitNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin(obj, "MyPlugin", "1.0", "Any");
    std::string path = plugin.loadPath().asChar();
    StylelitNode::pluginPath = path;

    // Register Command
    status = plugin.registerNode("StylelitNode", StylelitNode::id, StylelitNode::creator, StylelitNode::initialize);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }
    
    path = "source \"" + path + "/menu.mel\";";
    MGlobal::executeCommand("global string $melPath;");
    MGlobal::executeCommand("$melPath = \"" + plugin.loadPath() + "\";");
    MGlobal::executeCommand(path.c_str(), true);
    
    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(StylelitNode::id);
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    return status;
}