#include "stylitNode.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>


#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}

std::vector<string> lpes = { "beauty", "LDE", "LSE", "LDDE", "interreflection", "style2" };

MObject StylelitNode::time;
MObject StylelitNode::outputMesh;
MTypeId StylelitNode::id(0x80000);
MObject StylelitNode::sourceBeauty;
MObject StylelitNode::sourceLDE;
MObject StylelitNode::sourceLSE;
MObject StylelitNode::sourceLDDE;
MObject StylelitNode::sourceLD12E;
MObject StylelitNode::style;
MObject StylelitNode::pylevel;
string StylelitNode::pluginPath;

MStatus StylelitNode::compute(const MPlug & plug, MDataBlock & data) {
	MStatus returnStatus;
    // message in scriptor editor
    MGlobal::displayInfo("Compute!!!!");

	if (plug == outputMesh) {

        /* Get Default source*/
        MDataHandle sourceData = data.inputValue(sourceBeauty, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        MString sourceValue = sourceData.asString();
        string sourceBeautyPath = sourceValue.asChar();

        sourceData = data.inputValue(sourceLDE, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        sourceValue = sourceData.asString();
        string sourceLDEPath = sourceValue.asChar();

        sourceData = data.inputValue(sourceLSE, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        sourceValue = sourceData.asString();
        string sourceLSEPath = sourceValue.asChar();

        sourceData = data.inputValue(sourceLDDE, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        sourceValue = sourceData.asString();
        string sourceLDDEPath = sourceValue.asChar();

        sourceData = data.inputValue(sourceLD12E, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        sourceValue = sourceData.asString();
        string sourceLD12EPath = sourceValue.asChar();

        string sourcePaths[] = { sourceBeautyPath , sourceLDEPath, sourceLSEPath, sourceLDDEPath, sourceLD12EPath };

        /* Get Default target*/
        MDataHandle styleData = data.inputValue(style, &returnStatus);
        McheckErr(returnStatus, "Error getting style data handle\n");
        MString styleValue = styleData.asString();

        /* Get Default pylevel*/
        MDataHandle pylevelData = data.inputValue(pylevel, &returnStatus);
        McheckErr(returnStatus, "Error getting pylevel data handle\n");
        int pylevelValue = pylevelData.asInt();

        /* Get output object */
        MDataHandle outputHandle = data.outputValue(outputMesh, &returnStatus);
        McheckErr(returnStatus, "ERROR getting polygon data handle\n");

        std::vector<unique_ptr<cv::Mat>> images(11);
        string beautyDirectory = sourceBeautyPath + "/..";

        for (int i = 0; i < 6; i++)
        {
            cv::Mat img = cv::imread(StylelitNode::pluginPath + "/images/source/source_" + lpes[i] + ".jpg");
            if (i == 5) {
                img = cv::imread(styleValue.asChar());
            }
            cv::Mat newimg;
            cv::Size s = img.size();
            s /= 2;
            cv::pyrDown(img, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            cv::imwrite(beautyDirectory + "/source_" + lpes[i] + ".jpg", newimg);
            cv::Mat imgNormalized;
            newimg.convertTo(imgNormalized, CV_32FC3);
            imgNormalized /= 255.0f;
            images[i] = make_unique<cv::Mat>(imgNormalized);
        }

        for (int i = 0; i < 5; i++)
        {
            cv::Mat img = cv::imread(sourcePaths[i]);
            MGlobal::displayInfo(sourcePaths[i].c_str());
            cv::Mat newimg;
            cv::Size s = img.size();
            s /= 2;
            cv::pyrDown(img, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
     
            cv::Mat imgNormalized;
            newimg.convertTo(imgNormalized, CV_32FC3);
            imgNormalized /= 255.0f;
            images[i + 6] = make_unique<cv::Mat>(imgNormalized);
        }

        unique_ptr<FeatureVector> fa = make_unique<FeatureVector>(images[0], images[1], images[2], images[3], images[4]);
        unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
        unique_ptr<FeatureVector> fap = make_unique<FeatureVector>(images[5], lde, lse, ldde, ld12e);
        unique_ptr<FeatureVector> fb = make_unique<FeatureVector>(images[6], images[7], images[8], images[9], images[10]);
        unique_ptr<Pyramid> pa = make_unique<Pyramid>(fa, pylevelValue);
        unique_ptr<Pyramid> pap = make_unique<Pyramid>(fap, pylevelValue);
        unique_ptr<Pyramid> pb = make_unique<Pyramid>(fb, pylevelValue);

        stylit.setA(std::move(pa));
        stylit.setAP(std::move(pap));
        stylit.setB(std::move(pb));
        stylit.setNeigbor(5);
        stylit.setMIU(2);
        stylit.setTmpPath(beautyDirectory);

        stylit.initialize();

        stylit.synthesize();
        MString command;
        command += "if (`window -exists ImagesWin`) {deleteUI ImagesWin;}";
        command += "window - t \"Synthesis\" ImagesWin;";
        command += "columnLayout;";
        const char* folderPath = beautyDirectory.c_str();
        command += "image - image \"" + MString(folderPath) + "/result.jpg" + "\";";
       // command += "showWindow ImagesWin;";

        MGlobal::executeCommand(command, true, false);

        data.setClean(plug);

	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}

void* StylelitNode::creator()
{
	return new StylelitNode;
}

MStatus StylelitNode::initialize() {
    MGlobal::displayInfo("Initialization!!!!");

	MStatus returnStatus;
	MFnTypedAttribute typedAttr;
	MFnEnumAttribute enumAttr;
    MFnNumericAttribute numericalAttr;

	StylelitNode::sourceBeauty = typedAttr.create("sourceBeautyPath", "sourceBeauty", MFnData::kString, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");
    StylelitNode::sourceLDE = typedAttr.create("sourceLDEPath", "sourceLDE", MFnData::kString, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");
    StylelitNode::sourceLSE = typedAttr.create("sourceLSEPath", "sourceLSE", MFnData::kString, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");
    StylelitNode::sourceLDDE = typedAttr.create("sourceLDDEPath", "sourceLDDE", MFnData::kString, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");
    StylelitNode::sourceLD12E = typedAttr.create("sourceLD12EPath", "sourceLD12E", MFnData::kString, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");

	StylelitNode::style = typedAttr.create("stylePath", "style", MFnData::kString, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating StylelitNode style attribute\n");

    StylelitNode::pylevel = numericalAttr.create("pyramidLevel", "pylevel", MFnNumericData::kInt, 0.0);
    McheckErr(returnStatus, "ERROR creating StylelitNode pylevel attribute\n");

    StylelitNode::outputMesh = typedAttr.create("outputMesh", "out", MFnData::kMesh, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating animCube output attribute\n");

    typedAttr.setStorable(false);
    typedAttr.setHidden(true);


	returnStatus = addAttribute(StylelitNode::sourceBeauty);
	McheckErr(returnStatus, "ERROR adding source attribute\n");
    returnStatus = addAttribute(StylelitNode::sourceLDE);
    McheckErr(returnStatus, "ERROR adding source attribute\n");
    returnStatus = addAttribute(StylelitNode::sourceLSE);
    McheckErr(returnStatus, "ERROR adding source attribute\n");
    returnStatus = addAttribute(StylelitNode::sourceLDDE);
    McheckErr(returnStatus, "ERROR adding source attribute\n");
    returnStatus = addAttribute(StylelitNode::sourceLD12E);
    McheckErr(returnStatus, "ERROR adding source attribute\n");

	returnStatus = addAttribute(StylelitNode::style);
	McheckErr(returnStatus, "ERROR adding style attribute\n");

    returnStatus = addAttribute(StylelitNode::pylevel);
    McheckErr(returnStatus, "ERROR adding pylevel attribute\n");

    returnStatus = addAttribute(StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR adding outputMesh attribute\n");

    

    returnStatus = attributeAffects(StylelitNode::sourceBeauty, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");
    returnStatus = attributeAffects(StylelitNode::sourceLDE, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");
    returnStatus = attributeAffects(StylelitNode::sourceLSE, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");
    returnStatus = attributeAffects(StylelitNode::sourceLDDE, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");
    returnStatus = attributeAffects(StylelitNode::sourceLD12E, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");


    returnStatus = attributeAffects(StylelitNode::style, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");

    returnStatus = attributeAffects(StylelitNode::pylevel, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");

	return MS::kSuccess;

}