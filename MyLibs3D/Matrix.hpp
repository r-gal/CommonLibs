#ifndef MATRIX_LIB
#define MATRIX_LIB

#include <stdint.h>
#include "CommonDef.hpp"
#include "SignalList.hpp"
#include "Vector.hpp"

  void MatrixTest(void);


class RotMatrix_c
{
  

  public:
  float data[3][3];
  RotMatrix_c(void);

  RotMatrix_c(float angle, char axe);

   RotMatrix_c operator *( RotMatrix_c &m) ;

  float& operator()(int r, int c) 
  {
    return data[r][c];
  }

  static void PrintMatrix(RotMatrix_c* rm);

};

Vector3D operator *( RotMatrix_c &m,Vector3D &v) ;
Point3D operator *( RotMatrix_c &m, Point3D &v) ;



#endif