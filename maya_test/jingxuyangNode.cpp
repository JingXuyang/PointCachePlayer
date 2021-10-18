//
// Copyright (C) maya_test
// 
// File: jingxuyangNode.cpp
//
// Dependency Graph Node: jingxuyang
//
// Author: Maya Plug-in Wizard 2.0
//

#include "jingxuyangNode.h"

#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFn.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItGeometry.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MTime.h>
#include <maya/MFnMesh.h>

#include <fstream>
#include <vector>
#include <string>
using namespace std;


MTypeId jingxuyang::id( 0x00002 );
MObject jingxuyang::inputGeom;
MObject jingxuyang::outputGeom;
MObject jingxuyang::lc_time;

jingxuyang::jingxuyang() {}
jingxuyang::~jingxuyang() {}

void writeLog(MString message)
{
	
	// set log path
	const MString logPath = "D:/Test/aaa.log";
	// create log and start 
	MGlobal::startErrorLogging(logPath);
	// write log mesage
	MGlobal::doErrorLogEntry(message);
	// stop log
	MGlobal::stopErrorLogging();
	// close log
	MGlobal::closeErrorLog();

}

float* readBinary(string assetname, unsigned int frame)
{	
	string folderName = "D:\\Test\\PcpTest\\" + assetname + "\\" ;
	string filename = assetname + "." + std::to_string(frame).c_str() + ".bin";
	std::ifstream ifp(filename, std::ios::binary | std::ios::in);

	// 打不开文件，报错退出
	if (!ifp)
	{
		cout << "Error open ifp" << endl;
		exit(1);
	}
	
	// 获取文件大小
	ifp.seekg(0, std::ios::end);
	int filesize = ifp.tellg();
	ifp.seekg(0, std::ios::beg);
	
	// 获取 item 个数
	int items = filesize / 4;
	
	// 动态申请长度为 items 的内存空间
	float* arr = new float[items];
	
	// 读取文件数据，存入数组中
	ifp.read((char*)arr, items * sizeof(float));
	/* cout << ifp.gcount() << " bytes read " << endl;*/

	ifp.close();

	return arr;
}

vector<string> split(const string &str, const string &pattern)
{
	char * strc = new char[strlen(str.c_str()) + 1];
	strcpy(strc, str.c_str());   //string转换成C-string
	vector<string> res;
	char* temp = strtok(strc, pattern.c_str());
	while (temp != NULL)
	{
		res.push_back(string(temp));
		temp = strtok(NULL, pattern.c_str());
	}
	delete[] strc;
	return res;
}

MDagPath& getMDagPath(const MObject& obj)
{
	MDagPath mDagPath;
	MDagPath::getAPathTo(obj, mDagPath);
	return mDagPath;
}

MStatus jingxuyang::setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs)
{
	MStatus status = MS::kSuccess;

	if (plugBeingDirtied.attribute() != inputGeom)
	{
		return status;
	}

	MPlug outArrayPlug(thisMObject(), outputGeom);

	if (plugBeingDirtied.isElement())
	{
		MPlug outElemPlug = outArrayPlug.elementByLogicalIndex(plugBeingDirtied.logicalIndex(), &status);
		affectedPlugs.append(outElemPlug);
		affectedPlugs.append(outArrayPlug);
	}
	else
	{
		unsigned int i, n = outArrayPlug.numElements();
		for (i = 0; i < n; ++i)
		{
			MPlug outElemPlug = outArrayPlug.elementByPhysicalIndex(i, &status);
			affectedPlugs.append(outElemPlug);
		}
		affectedPlugs.append(outArrayPlug);
	}

	return status;

}

MStatus jingxuyang::compute(const MPlug& plug, MDataBlock& data)
{	

	MStatus returnStatus;

	if (plug == outputGeom)
	{	
		MPlug inputArrayPlug(thisMObject(), inputGeom);
		MPlug outputArrayPlug(thisMObject(), outputGeom);

		unsigned int inputNum = inputArrayPlug.numConnectedElements();
		unsigned int outputNum = outputArrayPlug.numConnectedElements();
		if (inputNum != outputNum)
		{
			returnStatus.perror("Error getting input time and output nums\n");
			return returnStatus;
		}

		/* Get time */
		MDataHandle timeData = data.inputValue(lc_time, &returnStatus);
		if (!returnStatus)
		{
			returnStatus.perror("Error getting time data handle\n");
			return returnStatus;
		}
		MTime time = timeData.asTime();
		int frame = (int)time.as(MTime::kFilm);

		for (unsigned int i = 0; i < inputArrayPlug.numElements(); i++)
		{
			MPlug inputPlug = inputArrayPlug.elementByPhysicalIndex(i);
			// MGlobal::displayInfo(MString("inputPlug: ") + inputPlug.name());
			MPlug outputPlug = outputArrayPlug.elementByPhysicalIndex(i);
			// MGlobal::displayInfo(MString("outputPlug: ") + outputPlug.name());

			// Skip the following if input or output plug is not connected
			if (!(inputPlug.isConnected() && outputPlug.isConnected()))
			{
				continue;
			}

			MDataHandle inputData = inputPlug.asMDataHandle();
			MDataHandle outputData = data.outputValue(outputPlug);

			// Copy inout data to output
			outputData.copy(inputData);

			// Read the input value from the handle.
			MObject inObj = inputData.asMesh();

			// Get input plug source node name
			MPlug inSourcePlug = inputPlug.source(&returnStatus);
			MObject inSourceObj = inSourcePlug.node();
			MFnDagNode inSourceDagNode(inSourceObj);
			
			std::string nodeName = inSourceDagNode.partialPathName().asChar();
			std::string delimiter = ":";
			size_t pos = 0;
			std::string nameSpace;
			while ((pos = nodeName.find(delimiter)) != std::string::npos) {
				nameSpace = nodeName.substr(0, pos);
				nodeName.erase(0, pos + delimiter.length());
			}
			std::string meshName = nodeName;

			// Read bianry
			string folderName = "D:\\Test\\PcpTest\\" + nameSpace +"\\" + meshName;
			string filename = folderName + "\\" + meshName + "." + std::to_string(frame).c_str() + ".bin";
			std::ifstream ifp(filename, std::ios::binary | std::ios::in);
			if (!ifp)
			{
				string msg = "[No file to open] " + filename;
				MGlobal::displayWarning(msg.c_str());
				return MS::kUnknownParameter;
			}
			// 获取文件大小
			ifp.seekg(0, std::ios::end);
			int filesize = ifp.tellg();
			ifp.seekg(0, std::ios::beg);
			// 获取 item 个数
			int items = filesize / 4;
			// 申请长度为 items 的内存空间
			float* arr = new float[items];
			// 读取文件数据，存入数组中
			ifp.read((char*)arr, items * sizeof(float));
			 cout << ifp.gcount() << " bytes read " << endl;
			ifp.close();

			// Get input mesh vertex nums
			MFnMesh inputMesh(inObj);
			int verticesNum = inputMesh.numVertices();

			// Do an iteration where we get each point and set it to a new value
			MItGeometry iter(outputData, inObj, false);
			for (int index = 0; !iter.isDone(); iter.next())
			{
				MPoint point(arr[index * 3], arr[index * 3 + 1], arr[index * 3 + 2]);
				iter.setPosition(point);
				index++;
			}

			delete arr;

		}

		data.setClean(outputGeom);

	}else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;

}

/*MStatus jingxuyang::compute( const MPlug& plug, MDataBlock& data )
{

	MStatus returnStatus;

	if (plug == outputGeom || plug.array() == outputGeom)
	{
		MDataHandle inputData = data.inputValue(inputGeom, &returnStatus);
		MDataHandle outputData = data.outputValue(outputGeom, &returnStatus);

		// Read the input value from the handle.
		MObject inMeshObj = inputData.asMesh();

		// Get input mesh vertex nums
		MFnMesh inputMesh(inMeshObj);
		int verticesNum = inputMesh.numVertices();

		// get the input geometry
		MDataHandle hInputGeom = data.inputValue(inputGeom, &returnStatus);
		MDataHandle hOutput = data.outputValue(plug);
		hOutput.copy(hInputGeom);

		// do an iteration where we get each point and set it to a new value
		MItGeometry iter(hOutput, inMeshObj, false);
		for (; !iter.isDone(); iter.next()) {
			MPoint point(iter.position().x + 10, iter.position().y + 10, iter.position().z + 10);
			iter.setPosition(point);
		}

		// Copy the inMesh to the outMesh so we can operate directly on the outMesh
		//
		//outputData.set(inputData.asMesh());
		MPointArray pointArray;
		pointArray.setLength(verticesNum);
		for (int i = 0; i < verticesNum; i++)
		{
		double x = double(i) + 10;
		double y = double(i) + 10;
		double z = double(i) + 10;
		MPoint newPoint(x, y, z);
		MGlobal::displayInfo(MString(std::to_string(newPoint.x).c_str()));
		MGlobal::displayInfo(MString(std::to_string(newPoint.y).c_str()));
		MGlobal::displayInfo(MString(std::to_string(newPoint.z).c_str()));
		pointArray.append(newPoint);
		}
		inputMesh.setPoints(pointArray);

		// Iter the mesh and set vertex position
		MItMeshVertex meshVertexIt(inMeshObj, &returnStatus);
		int vertexNum = meshVertexIt.count();

		for (int i = 0; !meshVertexIt.isDone(); meshVertexIt.next()) {
		//MObject mboj = meshVertexIt.currentItem(&returnStatus);
		int vidx = meshVertexIt.index();
		MGlobal::displayInfo(MString("index :"));
		MGlobal::displayInfo(MString(std::to_string(vidx).c_str()));
		MPoint currentPos = meshVertexIt.position();
		MPoint point(currentPos.x + 10, currentPos.y + 10, currentPos.z + 10);
		returnStatus = meshVertexIt.setPosition(point);

		++i;
		}
		
		
		// copy the input mesh to the output mesh
		MFnMesh newMeshFn;
		MFnMeshData dataCreator;
		MObject newMeshData = dataCreator.create();
		newMeshFn.copy(data.inputValue(inMeshObj).asMeshTransformed(), newMeshData);
		data.outputValue(outMeshObj).set(newMeshData);
		
			
		
	}
	else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;

}*/

void* jingxuyang::creator()
{
	return new jingxuyang();
}

MStatus jingxuyang::initialize()	
{
	MFnUnitAttribute unitAttr;
	MStatus				stat;
	MFnTypedAttribute	tAttr;

	lc_time = unitAttr.create("time", "tm", MFnUnitAttribute::kTime, 0.0, &stat);
	inputGeom = tAttr.create("inputGeom", "ing", MFnData::kMesh, MObject::kNullObj, &stat);
	tAttr.setArray(true);
	tAttr.setStorable(true);
	tAttr.setHidden(false);
	outputGeom = tAttr.create("outputGeom", "outg", MFnData::kMesh, MObject::kNullObj, &stat);
	tAttr.setWritable(true);
	tAttr.setStorable(false);
	tAttr.setArray(true);

	stat = addAttribute(inputGeom);
	stat = addAttribute(outputGeom);
	stat = addAttribute(lc_time);

	stat = attributeAffects(inputGeom, outputGeom);
	stat = attributeAffects(lc_time, jingxuyang::outputGeom);

	return MS::kSuccess;

}

