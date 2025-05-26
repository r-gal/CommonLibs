#include "Matrix.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "CommonDef.hpp"
#include "SignalList.hpp"

#include "math.h"

RotMatrix_c::RotMatrix_c(void)
{
      data[0][0] = 1;
      data[0][1] = 0;
      data[0][2] = 0;
      data[1][0] = 0;
      data[1][1] = 1;
      data[1][2] = 0;
      data[2][0] = 0;
      data[2][1] = 0;
      data[2][2] = 1;
}

RotMatrix_c::RotMatrix_c(float angle, char axe)
{
  float cosAngle = cosf(angle);
  float sinAngle = sinf(angle);

  switch(axe)
  {
    default:
    case 'X':
      data[0][0] = 1;
      data[0][1] = 0;
      data[0][2] = 0;
      data[1][0] = 0;
      data[1][1] = cosAngle;
      data[1][2] = -sinAngle;
      data[2][0] = 0;
      data[2][1] = sinAngle;
      data[2][2] = cosAngle;
      break;

    case 'Y':
      data[0][0] = cosAngle;
      data[0][1] = 0;
      data[0][2] = sinAngle;
      data[1][0] = 0;
      data[1][1] = 1;
      data[1][2] = 0;
      data[2][0] = -sinAngle;
      data[2][1] = 0;
      data[2][2] = cosAngle;
      break;

    case 'Z':
      data[0][0] = cosAngle;
      data[0][1] = -sinAngle;
      data[0][2] = 0;
      data[1][0] = sinAngle;
      data[1][1] = cosAngle;
      data[1][2] = 0;
      data[2][0] = 0;
      data[2][1] = 0;
      data[2][2] = 1;
      break;
  }
}

RotMatrix_c RotMatrix_c::operator *( RotMatrix_c &m) 
{
  RotMatrix_c res;

  for(int r = 0;r < 3;r++)
  {
    for(int c = 0;c < 3;c++)
    {
      float val = 0;

      for(int i=0;i< 3;i++)
      {
        val += this->data[r][i] * m.data[i][c];
      }
      res.data[r][c] = val;
    }
  }

  return res;
}

Vector3D operator *(RotMatrix_c &m, Vector3D &v) 
{
  float vData[3] = {v.X,v.Y,v.Z};
  float vOutData[3];
  

  for(int r = 0;r < 3;r++)
  {

    float val = 0;

    for(int i=0;i< 3;i++)
    {
      val += m.data[r][i] * vData[i];
    }
    vOutData[r] = val;

  }

  Vector3D res(vOutData[0],vOutData[1],vOutData[2]);
  return res;
}

Point3D operator *(RotMatrix_c &m, Point3D &v) 
{
  float vData[3] = {v.X,v.Y,v.Z};
  float vOutData[3];
  

  for(int r = 0;r < 3;r++)
  {

    float val = 0;

    for(int i=0;i< 3;i++)
    {
      val += m.data[r][i] * vData[i];
    }
    vOutData[r] = val;

  }

  Point3D res(vOutData[0],vOutData[1],vOutData[2]);
  return res;
}



void RotMatrix_c::PrintMatrix(RotMatrix_c* rm)
{
   printf("[%.2f, %.2f, %.2f]\n", (*rm)(0,0), (*rm)(0,1), (*rm)(0,2));
   printf("[%.2f, %.2f, %.2f]\n", (*rm)(1,0), (*rm)(1,1), (*rm)(1,2));
   printf("[%.2f, %.2f, %.2f]\n", (*rm)(2,0), (*rm)(2,1), (*rm)(2,2));
}

void MatrixTest(void)
{
 
   RotMatrix_c rmx(0,'X');
   RotMatrix_c::PrintMatrix(&rmx);

   RotMatrix_c rmy(0,'Y');
   RotMatrix_c::PrintMatrix(&rmy);

   RotMatrix_c rmz(0,'Z');
   RotMatrix_c::PrintMatrix(&rmz);


}