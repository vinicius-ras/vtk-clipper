#ifndef CCLIPPING_ALGORITHM_HPP__
#define CCLIPPING_ALGORITHM_HPP__

#include <vtkObject.h>

/** An abstract class, from which all the clipping algorithm implementations inherit.
 * This class defines generic methods that should be implemented by the clipping algorithms,
 * into their respective specialized classes.
 */
class CClippingAlgorithm : public vtkObject
{
public:
	// METHODS
	CClippingAlgorithm();
	virtual ~CClippingAlgorithm();

	// PURE VIRTUAL METHODS
	/** This method is called for each voxel that should be tested on the volumetric image.
	 * In each call, this method is passed the coordinates which represent the voxel's position
	 * into the 3D space. The implementations should then evaluate the given coordinates and return
	 * a value which indicate if the voxel represented by the given coordinates should be hidden or not, according to
	 * the specific clipping algorithm.
	 * @param cellX The X coordinate of the 3D-space point which represents the voxel.
	 * @param cellY The Y coordinate of the 3D-space point which represents the voxel.
	 * @param cellZ The Z coordinate of the 3D-space point which represents the voxel.
	 */
	virtual bool hideVoxel( double cellX, double cellY, double cellZ ) = 0;
};

#endif
