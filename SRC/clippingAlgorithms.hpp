#ifndef CLIPPINGALGORITHMS_HPP__
#define CLIPPINGALGORITHMS_HPP__

/** An enumeration which provides all the clipping algorithms of the application. */
enum EClippingAlgorithm
{
	evAlgorithmEraser2D,
	evAlgorithmClipper2D,
	evAlgorithmPolyClipper2D,

	evAlgorithmEraser3D,
	evAlgorithmClipper3D,

	evAlgorithmNone,
	evAlgorithmLocked               ///< A special algorithm type which indicates the current algorithm is locked.
};

#endif
