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
MObject StylelitNode::source;
MObject StylelitNode::style;


MStatus StylelitNode::compute(const MPlug & plug, MDataBlock & data) {
	MStatus returnStatus;
    // message in scriptor editor
    MGlobal::displayInfo("Compute!!!!");

	if (plug == outputMesh) {

        /* Get Default grammer*/
        MDataHandle sourceData = data.inputValue(source, &returnStatus);
        McheckErr(returnStatus, "Error getting grammer data handle\n");
        MString sourceValue = sourceData.asString();
        string sourcePath = sourceValue.asChar();

        /* Get Default grammer*/
        MDataHandle styleData = data.inputValue(style, &returnStatus);
        McheckErr(returnStatus, "Error getting style data handle\n");
        MString styleValue = styleData.asString();

        /* Get output object */
        MDataHandle outputHandle = data.outputValue(outputMesh, &returnStatus);
        McheckErr(returnStatus, "ERROR getting polygon data handle\n");

        std::vector<unique_ptr<cv::Mat>> images(11);
        
        for (int i = 0; i < 6; i++)
        {
            cv::Mat img = cv::imread("D:/CIS660/AuthoringTool/Stylit/stylit/images/source/source_" + lpes[i] + ".jpg");
            string str = sourcePath + "images/source/source_" + lpes[i] + ".jpg";
            MGlobal::displayInfo(str.c_str());
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
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            cv::imwrite(sourcePath + "/source_" + lpes[i] + ".jpg", newimg);
            cv::Mat imgNormalized;
            newimg.convertTo(imgNormalized, CV_32FC3);
            imgNormalized /= 255.0f;
            images[i] = make_unique<cv::Mat>(imgNormalized);
        }

        for (int i = 0; i < 5; i++)
        {
            cv::Mat img = cv::imread(sourcePath + "/target_" + lpes[i] + ".jpg");
            MGlobal::displayInfo(sourcePath.c_str());
            cv::Mat newimg;
            cv::Size s = img.size();
            s /= 2;
            cv::pyrDown(img, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            s /= 2;
            cv::pyrDown(newimg, newimg, s);
            cv::imwrite(sourcePath + "/targetr_" + lpes[i] + ".jpg", newimg);
            cv::Mat imgNormalized;
            newimg.convertTo(imgNormalized, CV_32FC3);
            imgNormalized /= 255.0f;
            images[i + 6] = make_unique<cv::Mat>(imgNormalized);
        }

        unique_ptr<FeatureVector> fa = make_unique<FeatureVector>(images[0], images[1], images[2], images[3], images[4]);
        unique_ptr<cv::Mat> lde(nullptr), lse(nullptr), ldde(nullptr), ld12e(nullptr);
        unique_ptr<FeatureVector> fap = make_unique<FeatureVector>(images[5], lde, lse, ldde, ld12e);
        unique_ptr<FeatureVector> fb = make_unique<FeatureVector>(images[6], images[7], images[8], images[9], images[10]);
        unique_ptr<Pyramid> pa = make_unique<Pyramid>(fa, 1);
        unique_ptr<Pyramid> pap = make_unique<Pyramid>(fap, 1);
        unique_ptr<Pyramid> pb = make_unique<Pyramid>(fb, 1);

        stylit.setA(std::move(pa));
        stylit.setAP(std::move(pap));
        stylit.setB(std::move(pb));
        stylit.setNeigbor(5);
        stylit.setMIU(2);
        stylit.setTmpPath(sourcePath);

        stylit.initialize();

        stylit.synthesize();

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

	StylelitNode::source = typedAttr.create("sourcePath", "source", MFnData::kString, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating StylelitNode source attribute\n");
	

	StylelitNode::style = typedAttr.create("stylePath", "style", MFnData::kString, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating StylelitNode style attribute\n");

    StylelitNode::outputMesh = typedAttr.create("outputMesh", "out", MFnData::kMesh, MObject::kNullObj, &returnStatus);
    McheckErr(returnStatus, "ERROR creating animCube output attribute\n");

    typedAttr.setStorable(false);
    typedAttr.setHidden(true);

	returnStatus = addAttribute(StylelitNode::source);
	McheckErr(returnStatus, "ERROR adding source attribute\n");

	returnStatus = addAttribute(StylelitNode::style);
	McheckErr(returnStatus, "ERROR adding style attribute\n");

    returnStatus = addAttribute(StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR adding outputMesh attribute\n");

    

    returnStatus = attributeAffects(StylelitNode::source, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");

    returnStatus = attributeAffects(StylelitNode::style, StylelitNode::outputMesh);
    McheckErr(returnStatus, "ERROR in attributeAffects\n");

	return MS::kSuccess;

}