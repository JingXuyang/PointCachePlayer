#ifndef _jingxuyangNode
#define _jingxuyangNode


#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 


class jingxuyang : public MPxNode
{
public:
						jingxuyang();
	virtual				~jingxuyang(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );
	virtual MStatus     setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs);

	static  void*		creator();
	static  MStatus		initialize();

public:
	static MObject		inputGeom;
	static MObject		outputGeom;
	static MTypeId		id;
	static MObject		lc_time;

};

#endif
