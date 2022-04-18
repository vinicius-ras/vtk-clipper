#ifndef CKINECTMENUTRANSLATIONSTAGE_HPP__
#define CKINECTMENUTRANSLATIONSTAGE_HPP__

#include "CKinectMenu.hpp"

#include <vtkSmartPointer.h>
#include <vtkActor2D.h>
#include <vtkTransform.h>

/** A menu for the Kinect Rendering Panel which allows the user to translate the currently selected Clipping Tool's cursor. */
class CKinectMenuTranslationStage : public CKinectMenu
{
public:
	// TYPEDEFS
	/** An enumeration providing the axes for translation. */
	enum ETranslationAxes
	{
		evAxisX,   ///< Indicates the X translation axis.
		evAxisY,   ///< Indicates the Y translation axis.
		evAxisZ    ///< Indicates the Z translation axis.
	};

	// METHODS
	explicit CKinectMenuTranslationStage( vtkSmartPointer<vtkRenderer> renderer );
	~CKinectMenuTranslationStage();

	// OVERRIDEN METHODS: CKinectMenu
	virtual void processMenuItems( void );


private:
	// METHODS
	void applyCursorTranslation( ETranslationAxes axis, vtkSmartPointer<vtkTransform> camTransform, double translationValue );
	void startClippingAlgorithm( void );


	// PROPERTIES
	vtkSmartPointer<vtkActor2D> m_textActor;
};

#endif
