#ifndef CVECTOR_HPP__
#define CVECTOR_HPP__

#include <cmath>
#include <vector>

class CVector3;

// VECTOR OPERATORS
inline const CVector3 operator +( const CVector3 &v1, const CVector3 &v2 );
inline const CVector3 operator -( const CVector3 &v1, const CVector3 &v2 );
inline const CVector3 operator *( const CVector3 &v1, double scalar );
inline const CVector3 operator *( double scalar, const CVector3 &v1 );

// CLASS
/** Represents a vector of 3 components: x, y and z. */
class CVector3
{
public:
	// PROPERTIES
	double x;   ///< The value for the X component of the vector.
	double y;   ///< The value for the Y component of the vector.
	double z;   ///< The value for the Z component of the vector.

	// STATIC METHODS
	inline static const double dot( const CVector3 &v1, const CVector3 &v2 )
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	inline static const CVector3 cross( const CVector3 &v1, const CVector3 &v2 )
	{
		double rx = v1.y * v2.z - v1.z * v2.y;
		double ry = v1.z * v2.x - v1.x * v2.z;
		double rz = v1.x * v2.y - v1.y * v2.x;

		return CVector3( rx, ry, rz );
	}


	// METHODS
	inline CVector3()
		: x(0),
		y(0),
		z(0)
	{
	};
	inline CVector3( double x, double y, double z )
		: x( x ),
		y( y ),
		z( z )
	{
	}


	inline void normalize( void )
	{
		double vLen = sqrt( dot( *this, *this ) );

		this->x /= vLen;
		this->y /= vLen;
		this->z /= vLen;
	}


};


// TYPEDEFS
typedef std::vector<CVector3> TVector3Array;



// VECTOR OPERATORS
inline const CVector3 operator +( const CVector3 &v1, const CVector3 &v2 )
{
	return CVector3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}


inline const CVector3 operator -( const CVector3 &v1, const CVector3 &v2 )
{
	return CVector3( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}


inline const CVector3 operator *( const CVector3 &v1, double scalar )
{
	return CVector3( v1.x * scalar, v1.y * scalar, v1.z * scalar );
}


inline const CVector3 operator *( double scalar, const CVector3 &v1 )
{
	return v1 * scalar;
}

#endif
