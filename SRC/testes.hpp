#ifndef TESTES_HPP__
#define TESTES_HPP__

#include <cmath>
#include "CVector3.hpp"
#include "CLine.hpp"

class vtkRenderer;
class vtkImageData;

template<typename X>
bool estaEntre( const X &val, const X &first, const X &last )
{
	return ( val >= first && val <= last );
}

inline double getRandomColor();


void criaActorByBounds( vtkRenderer *aRenderer, double *volBounds, double r, double g, double b );
void getVoxelBounds( vtkImageData *imgData, int cellX, int cellY, int cellZ, double *output );
void criaActorFromCellToCell( vtkRenderer *aRenderer, vtkImageData *imgData,
			   int cellX1, int cellY1, int cellZ1,
			   int cellX2, int cellY2, int cellZ2,
			   double r, double g, double b);


static const int APPLICATION_DESIRED_FPS = 40;



#endif
