#ifndef CGEOMETRYALGORITHMS_HPP__
#define CGEOMETRYALGORITHMS_HPP__

#include "CLine.hpp"

/** This class is specialized in executing algorithms related to geometry and space. */
class CGeometryAlgorithms
{
private:
	CGeometryAlgorithms() { }   // This class cannot be instantiated.


public:
	// STATIC METHODS
	static double getPointToLineSqDistance( const CVector3 &point, const CLine &line, CVector3 &basePoint );
	static double getPointToPointSqDistance( const CVector3 &v1, const CVector3 &v2 );

};

#endif
