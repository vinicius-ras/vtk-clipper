#ifndef CKINECTMENUROTATIONSTAGE_HPP__
#define CKINECTMENUROTATIONSTAGE_HPP__

#include "CKinectMenu.hpp"

#include <vtkSmartPointer.h>
#include <vtkActor2D.h>
#include <vtkTransform.h>

/** A menu for the Kinect Rendering Panel which allows the user to rotate the currently selected Clipping Tool's cursor. */
class CKinectMenuRotationStage : public CKinectMenu
{
public:
	// TYPEDEFS
	/** An enumeration providing the axes of rotation. */
	enum ERotationAxes
	{
		evAxisX,   ///< Indicates the X rotation axis.
		evAxisY,   ///< Indicates the Y rotation axis.
		evAxisZ    ///< Indicates the Z rotation axis.
	};

	// METHODS
	explicit CKinectMenuRotationStage( vtkSmartPointer<vtkRenderer> renderer );
	~CKinectMenuRotationStage();

	// OVERRIDEN METHODS: CKinectMenu
	virtual void processMenuItems( void );


private:
	// METHODS
	void applyCursorRotation( ERotationAxes axis, vtkSmartPointer<vtkTransform> camTransform, double rotationValue );


	// PROPERTIES
	vtkSmartPointer<vtkActor2D> m_textActor;
};

#endif
