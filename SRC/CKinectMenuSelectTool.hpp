#ifndef CKINECTMENUSELECTTOOL_HPP__
#define CKINECTMENUSELECTTOOL_HPP__

#include "CKinectMenu.hpp"

enum EClippingAlgorithm;

/** A menu for the Kinect Rendering Panel which allows the user to select a Clipping Tool. */
class CKinectMenuSelectTool : public CKinectMenu
{
private:
	// PROPERTIES
	EClippingAlgorithm m_lastClippingAlgorithm;   ///< Will be used to detect when the user has changed the Clipping Algorithm.


public:
	// METHODS
	explicit CKinectMenuSelectTool( vtkSmartPointer<vtkRenderer> renderer );

	// OVERRIDEN METHODS: CKinectMenu
	virtual void processMenuItems( void );
};

#endif
