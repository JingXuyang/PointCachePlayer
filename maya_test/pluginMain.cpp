//
// Copyright (C) maya_test
// 
// File: pluginMain.cpp
//
// Author: Maya Plug-in Wizard 2.0
//

#include <maya/MFnPlugin.h>
#include "jingxuyangNode.h"


MStatus initializePlugin( MObject obj )

{ 
	MStatus   status;
	MFnPlugin plugin( obj, "maya_test", "2019", "Any");

	status = plugin.registerNode( "jingxuyang", jingxuyang::id, jingxuyang::creator,
								  jingxuyang::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)

{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( jingxuyang::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
