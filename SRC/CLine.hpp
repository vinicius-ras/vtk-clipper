#ifndef CLINE_HPP__
#define CLINE_HPP__

#include "CVector3.hpp"

/** Represents a line, which contains two points. */
class CLine
{
public:
	CVector3 p0, p1;   ///< The points that lie on the line, used to define it.
};


#endif
