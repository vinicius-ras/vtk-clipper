#include "CGeometryAlgorithms.hpp"

// dist_Point_to_Line(): get the distance of a point to a line.
//    Input:  a Point P and a Line L (in any dimension)
//    Return: the shortest distance, squared, from P to L
// distanceToBasepoint: distância de L.p0 até a projeção do ponto P na linha L.
/** Retrieves the square distace from a point to a given infinite line.
 * @param point The point to be used in the calculation.
 * @param line The line to be used in the calculation.
 * @param basePoint This parameter will return the projection of the given point into the given line.
 * @return Returns the distance from the point to the line, which is actually the distance from "point" to "basePoint".
 */
double CGeometryAlgorithms::getPointToLineSqDistance( const CVector3 &point, const CLine &line, CVector3 &basePoint )
{
    CVector3 v = line.p1 - line.p0;
    CVector3 w = point - line.p0;

	double c1 = CVector3::dot(w,v);
    double c2 = CVector3::dot(v,v);
    double b = c1 / c2;

    basePoint = line.p0 + b * v;
	return getPointToPointSqDistance(point, basePoint);
}


/** Calculates the distance between two given points.
 * @param p1 The first point to be used in the calculation.
 * @param p2 The second point to be used in the calculation. */
double CGeometryAlgorithms::getPointToPointSqDistance( const CVector3 &p1, const CVector3 &p2 )
{
	CVector3 v = p2 - p1;
	return CVector3::dot(v, v);
}
