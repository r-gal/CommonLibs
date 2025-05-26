#include "Vector.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "CommonDef.hpp"
//#include "SignalList.hpp"

#include "math.h"

iVector3D operator -(const iPoint3D &a,const iPoint3D &b) 
{ 
  return iVector3D(a.x-b.x,a.y-b.y,a.z-b.z);
}
iVector3D operator +(const iPoint3D &a,const iPoint3D &b) 
{ 
  return iVector3D(a.x+b.x,a.y+b.y,a.z+b.z);
}

iVector3D operator -(const iVector3D &a,const iVector3D &b) 
{ 
  return iVector3D(a.x-b.x,a.y-b.y,a.z-b.z);
}
iVector3D operator +(const iVector3D &a,const iVector3D &b) 
{ 
  return iVector3D(a.x+b.x,a.y+b.y,a.z+b.z);
}

bool operator ==(const iPoint3D &a,const iPoint3D &b)
{
  return((a.x==b.x)&&(a.y== b.y )&&(a.z == b.z));
}
/*
float q31_to_float(q31 in)
{

}

q31 float_to_q31(float in)
{

}
*/
 #if USE_FLOAT_VECTORS == 1

Vector3D operator -(const Point3D &a,const Point3D &b) 
{ 
  return(Vector3D(a.X-b.X,a.Y-b.Y,a.Z-b.Z));
}

Point3D::operator iPoint3D  const()
{
  iPoint3D ret;
  ret.x = (float)X*16000;
  ret.y = (float)Y*16000;
  ret.z = (float)Z*16000;
  return ret;
};

Point3D Point3D::operator+ (Vector3D val)
{
  Point3D ret;
  ret.X = X + val.X;
  ret.Y = Y + val.Y;
  ret.Z = Z + val.Z;
  return ret;

}


Vector3D Vector3D::operator*= ( float multi)
{
    X = X * multi;
    Y = Y * multi;
    Z = Z * multi;

    return *this;    
}

Vector Vector::operator* ( float multi)
{
    Vector ret;
    ret.X = X * multi;
    ret.Y = Y * multi;

    return ret;    
}

Vector Vector::operator*= ( float multi)
{
    X = X * multi;
    Y = Y * multi;

    return *this;    
}

Vector Vector::operator+ (Vector val)
{
    Vector ret;
    ret.X = X + val.X;
    ret.Y = Y + val.Y;
    return ret; 
}

Vector Vector::operator+= (Vector val)
{
    X = X + val.X;
    Y = Y + val.Y;

    return *this; 

}

Vector Vector::operator- (Vector val)
{
    Vector ret;
    ret.X = X - val.X;
    ret.Y = Y - val.Y;
    return ret; 
}

Vector Vector::operator- (void)
{
    X = -X ;
    Y = -Y ;

    return *this; 

}


float Vector::Length(void)
{
  float length = (this->X * this->X ) + (this->Y * this->Y );
  if(length > 0)
  {
    length = sqrtf(length);
  }
  return length;

}

float Vector::Normalize(void)
{
  float length = (this->X * this->X ) + (this->Y * this->Y );
  if(length > 0)
  {
    length = sqrtf(length);
    this->X /= length;
    this->Y /= length; 
  }
  else
  {
    length = 0;
  }
  return length;
};

void Vector3D::Abs(void)
{
  this->X = abs(this->X);
  this->Y = abs(this->Y);
  this->Z = abs(this->Z);
};

float Vector3D::Length(void)
{
  float length = (this->X * this->X ) + (this->Y * this->Y )+ (this->Z * this->Z );
  if(length > 0)
  {
    length = sqrtf(length);
  }
  return length;

}

float Vector3D::Normalize(void)
{
  float length = (this->X * this->X ) + (this->Y * this->Y )+ (this->Z * this->Z );
  if(length > 0)
  {
    length = sqrtf(length);
    this->X /= length;
    this->Y /= length; 
    this->Z /= length;
  }
  else
  {
    length = 0;
  }
  return length;

};

#endif

iVector3D iVector3D::operator*= ( int multi)
{
    x = x * multi;
    y = y * multi;
    z = z * multi;

    return *this;    
}

iVector iVector::operator*= ( int multi)
{
    x = x * multi;
    y = y * multi;

    return *this;    
}

#if USE_CORDIC == 1


CORDIC_HandleTypeDef hcordic;
CORDIC_ConfigTypeDef sConfig;

int GetBestScaler(int a,int b, int c)
{
  a = abs(a);
  b= abs(b);
  c = abs(c);

  int x = a;
  if(b>x) { x = b; }
  if(c>x) { x = c; }

  int scaler = 0;

  while(((x & 0xF0000000) == 0) && (scaler < 20))
  {
    x <<= 1;
    scaler++;
  }
  return scaler;


}

void VectorMath_c::Init(void)
{

  hcordic.Instance = CORDIC;
  if (HAL_CORDIC_Init(&hcordic) != HAL_OK)
  {
    //Error_Handler();
  }

  sConfig.InSize = CORDIC_INSIZE_32BITS;
  sConfig.NbRead = CORDIC_NBREAD_2;
  sConfig.NbWrite = CORDIC_NBWRITE_2;
  sConfig.OutSize = CORDIC_OUTSIZE_32BITS;
  sConfig.Precision = CORDIC_PRECISION_15CYCLES;
  sConfig.Scale = CORDIC_SCALE_0;

}


#define CORDIC_LENGTH_SCALE 1024

int VectorMath_c::GetLength(iVector3D* a)
{
      sConfig.Function = CORDIC_FUNCTION_MODULUS;
      sConfig.NbRead =  CORDIC_NBREAD_1;
      sConfig.NbWrite = CORDIC_NBWRITE_2;
      HAL_CORDIC_Configure(&hcordic, &sConfig);

      int scaler =GetBestScaler(a->x ,a->y, a->z);

      int32_t data[2];
      data[0] = a->x<< scaler;
      data[1] = a->y<< scaler;

      HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

      data[1] = a->z<< scaler;

      HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

      data[0] >>= scaler;
      return data[0] ;
}

iVectorNorm3D iVector3D::Normalize(void)
{
  iVectorNorm3D wersor;

  if((this->x==0)&&(this->y==0)&&(this->z==0))
  {
    wersor.x = 0;
    wersor.y = 0;
    wersor.z = 0;
    wersor.length = 0;
  }
  else
  {
    sConfig.Function = CORDIC_FUNCTION_MODULUS;
    sConfig.NbRead =  CORDIC_NBREAD_2;
    sConfig.NbWrite = CORDIC_NBWRITE_2;
    HAL_CORDIC_Configure(&hcordic, &sConfig);

    int scaler =GetBestScaler(this->x ,this->y, this->z);

    int32_t data[2];
    data[0] = this->x<< scaler;
    data[1] = this->y<< scaler;
    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);
    int angleXY = data[1];
    data[1] = this->z<< scaler;
    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);
    int angleVZ = data[1];
    wersor.angleZ = angleVZ;
    wersor.length = data[0] >> scaler;

    sConfig.Function = CORDIC_FUNCTION_SINE;
    sConfig.NbRead =  CORDIC_NBREAD_2;
    sConfig.NbWrite = CORDIC_NBWRITE_2;
    HAL_CORDIC_Configure(&hcordic, &sConfig);

    data[0] = angleVZ;
    data[1] = 1<<30;

    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

    wersor.z = data[0];
    data[0] = angleXY;
    wersor.angleXY = angleXY;

    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

    wersor.x = data[1];
    wersor.y = data[0];

    wersor.x /=(16*256);
    wersor.y /=(16*256);
    wersor.z /=(16*256);
    wersor.x  *= 256;
    wersor.y  *= 256;
    wersor.z  *= 256;
    if(this->x == 0) { wersor.x = 0; }
    if(this->y == 0) { wersor.y = 0; }
    if(this->z == 0) { wersor.z = 0; }
/*
    uint32_t m = wersor.x & 0x01;
    wersor.x += m;
    m = wersor.y & 0x01;
    wersor.y += m;
    m = wersor.z & 0x01;
    wersor.z += m;
*/

  }
  return wersor;
}


iVectorNorm iVector::Normalize(void)
{
  iVectorNorm wersor;

  if((this->x==0)&&(this->y==0))
  {
    wersor.x = 0;
    wersor.y = 0;
    wersor.length = 0;
  }
  else
  {
    sConfig.Function = CORDIC_FUNCTION_MODULUS;
    sConfig.NbRead =  CORDIC_NBREAD_2;
    sConfig.NbWrite = CORDIC_NBWRITE_2;
    HAL_CORDIC_Configure(&hcordic, &sConfig);

    int scaler =GetBestScaler(this->x ,this->y, 0);

    int32_t data[2];
    data[0] = this->x<< scaler;
    data[1] = this->y<< scaler;
    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);
    int angleXY = data[1];
    wersor.length = data[0] >> scaler;

    sConfig.Function = CORDIC_FUNCTION_SINE;
    sConfig.NbRead =  CORDIC_NBREAD_2;
    sConfig.NbWrite = CORDIC_NBWRITE_2;
    HAL_CORDIC_Configure(&hcordic, &sConfig);

    data[1] = 1<<30;
    data[0] = angleXY;
    wersor.angleXY = angleXY;

    HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

    wersor.x = data[1];
    wersor.y = data[0];

    wersor.x /=(16*256);
    wersor.y /=(16*256);
    wersor.x  *= 256;
    wersor.y  *= 256;
    if(this->x == 0) { wersor.x = 0; }
    if(this->y == 0) { wersor.y = 0; }

  }
  return wersor;
}


iVector3D VectorMath_c::Wersor2Vector(iVectorNorm3D* wersor)
{
  iVector3D ret;
  ret.x = (int64_t)wersor->x * wersor->length/(1<<26);
  ret.y = (int64_t)wersor->y * wersor->length/(1<<26);
  ret.z = (int64_t)wersor->z * wersor->length/(1<<26);
  return ret;
}

PlaneRadial VectorMath_c::PlaneVector2Radial(iVector3D* vector)
{
      sConfig.Function = CORDIC_FUNCTION_MODULUS;
      sConfig.NbRead =  CORDIC_NBREAD_2;
      sConfig.NbWrite = CORDIC_NBWRITE_2;
      HAL_CORDIC_Configure(&hcordic, &sConfig);

      int scaler =GetBestScaler(vector->x, vector->y, vector->z);

      int32_t data[2];
      data[0] = vector->x << scaler;
      data[1] = vector->y<< scaler;

      HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

      PlaneRadial ret;

      ret.radius = data[0];//  >> scaler;
      ret.angle = data[1] / 32; /* convert to 6.26 format */

      ret.scaler = scaler;
      ret.z = vector->z << scaler;

      return ret;




}

#define PI_6_26 210828714

int VectorMath_c::GetArcLength(int startAngle, int endAngle, int r)
{
  /* input: angle format 6.26, r [um/16] */
  /* return length in [um/16] */
  if(endAngle < startAngle) 
  {
    endAngle += 2 * (2<<25);
  }  


  int dAngle = endAngle - startAngle  ;

  int64_t length = PI_6_26;
  length = length* r;
    length = length >> 26;
  length = length * dAngle;
  

  length = length >> 26;




  return length;
}


iPoint3D VectorMath_c::GetArcPoint(iPoint3D* centrePoint, int angle, int radius, int radiusScaler)
{
  iPoint3D ret = *centrePoint;
  /* modulo angle and convert to 1.31 format */

  angle <<= 5;


  sConfig.Function = CORDIC_FUNCTION_COSINE;
  sConfig.NbRead =  CORDIC_NBREAD_2;
  sConfig.NbWrite = CORDIC_NBWRITE_2;
  HAL_CORDIC_Configure(&hcordic, &sConfig);


  int32_t data[2];
  data[0] = angle;
  data[1] = radius;

  HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

  ret.x += data[0] >> radiusScaler;
  ret.y += data[1] >> radiusScaler;

  return ret;

}

iVector3D VectorMath_c::GetVersorDiffence(iVectorNorm3D* a,iVectorNorm3D* b)
{
  iVector3D ret;

  ret.x = a->x - b->x;
  ret.y = a->y - b->y;
  ret.z = a->z - b->z;

  return ret;
}

uint32_t VectorMath_c::Sqrt(uint32_t in)
{
  int scaler =  0;

  sConfig.Function = CORDIC_FUNCTION_SQUAREROOT;
  sConfig.NbRead =  CORDIC_NBREAD_1;
  sConfig.NbWrite = CORDIC_NBWRITE_1;
  HAL_CORDIC_Configure(&hcordic, &sConfig);


  int32_t data[2];
  data[0] = in << scaler;
  HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

  return data[0] >> scaler;
}

#endif

 #if USE_FLOAT_VECTORS == 1
float VectorMath_c::AngleBetween(Vector v1, Vector v2)
{
  /* expected: 

  Vstart = (0,10)
  Vend = (0,-2) -> angle = -180 (-PI)
  Vend = (-2,0) -> angle = 90 (PI/2)
  Vend = (0,2) -> angle = 0 (0)
  Vend = (2,0) -> angle = -90 (-PI/2)
  */


  float v1Length = v1.Length();
  float v2Length = v2.Length();

  float lengths = v1Length*v2Length;

  if(lengths != 0)
  {
    float result = (v1.X * v2.X) + (v1.Y * v2.Y);
    float resultOrtog = (v1.X* v2.Y) - (v1.Y * v2.X);
    result = result / lengths;

    result = acosf(result);

    if(resultOrtog <=0 ) { result = - result; }
    return result;
  }
  else
  {
    return 0;
  }
}
#endif

iPoint3D iPoint3D::operator +( const iVector3D & d )
{
    iPoint3D ret;
    ret.x = this->x + d.x;
    ret.y = this->y + d.y;
    ret.z = this->z + d.z;
    return ret;
}

iPoint3D & iPoint3D::operator +=( const iVector3D & d )
{
    this->x += d.x;
    this->y += d.y;
    this->z += d.z;
    return * this;

}

Point3D & Point3D::operator +=( const Vector3D & d )
{
    this->X += d.X;
    this->Y += d.Y;
    this->Z += d.Z;
    return * this;

}

Vector3D Radial::ToVector3D(void)
{
  Vector tmp = ToVector();
  Vector3D ret(tmp.X,tmp.Y,Z);
  return ret;
}

Vector Radial::ToVector(void)
{
  Vector ret;
  ret.X = cosf(angle)*radius;
  ret.Y= sinf(angle)*radius;
  return ret;
}

Radial Vector::ToRadial(void)
{
     Radial ret;
     ret.angle = atan2f(Y,X);
     ret.radius = Length();
     ret.Z = 0;

     return ret;
/*
      sConfig.Function = CORDIC_FUNCTION_MODULUS;
      sConfig.NbRead =  CORDIC_NBREAD_2;
      sConfig.NbWrite = CORDIC_NBWRITE_2;
      HAL_CORDIC_Configure(&hcordic, &sConfig);

      float angle = atan2f(X,Y);

      int scaler =GetBestScaler(vector->x, vector->y, vector->z);

      int32_t data[2];
      data[0] = vector->x << scaler;
      data[1] = vector->y<< scaler;

      HAL_CORDIC_Calculate(&hcordic, data, data, 1 , 1000);

      PlaneRadial ret;

      ret.radius = data[0];
      ret.angle = data[1] / 32; 

      ret.scaler = scaler;
      ret.z = vector->z << scaler;

      return ret;*/

}

Radial Vector3D::ToRadial(void)
{
  Radial ret = Vector(X,Y).ToRadial();
  ret.Z=Z;
  return ret;

}




iVector3D iVector3D::FromPlane(PLANE_SELECT_et planeSelect)
{
  switch(planeSelect)
  {
    case PLANE_XY:
      return iVector3D(x,y,z);
    case PLANE_XZ:
      return iVector3D(y,z,x);
    case PLANE_YZ:
      return iVector3D(z,x,y);
    default:
      return iVector3D(0,0,0);
  }
}

iVector3D iVector3D::ToPlane(PLANE_SELECT_et planeSelect)
{
  switch(planeSelect)
  {
    case PLANE_XY:
      return iVector3D(x,y,z);
    case PLANE_XZ:
      return iVector3D(z,x,y);
    case PLANE_YZ:
      return iVector3D(y,z,x);
    default:
      return iVector3D(0,0,0);
  }
}

iPoint3D iPoint3D::FromPlane(PLANE_SELECT_et planeSelect)
{

  switch(planeSelect)
  {
    case PLANE_XY:
      return iPoint3D(x,y,z);
    case PLANE_XZ:
      return iPoint3D(y,z,x);
    case PLANE_YZ:
      return iPoint3D(z,x,y);
    default:
      return iPoint3D(0,0,0);
  }

}

iPoint3D iPoint3D::ToPlane(PLANE_SELECT_et planeSelect)
{
  switch(planeSelect)
  {
    case PLANE_XY:
      return iPoint3D(x,y,z);
    case PLANE_XZ:
      return iPoint3D(z,x,y);
    case PLANE_YZ:
      return iPoint3D(y,z,x);
    default:
      return iPoint3D(0,0,0);
  }
}

Point3D Point3D::FromPlane(PLANE_SELECT_et planeSelect)
{

  switch(planeSelect)
  {
    case PLANE_XY:
      return Point3D(X,Y,Z);
    case PLANE_XZ:
      return Point3D(Y,Z,X);
    case PLANE_YZ:
      return Point3D(Z,X,Y);
    default:
      return Point3D(0,0,0);
  }

}

Point3D Point3D::ToPlane(PLANE_SELECT_et planeSelect)
{
  switch(planeSelect)
  {
    case PLANE_XY:
      return Point3D(X,Y,Z);
    case PLANE_XZ:
      return Point3D(Z,X,Y);
    case PLANE_YZ:
      return Point3D(Y,Z,X);
    default:
      return Point3D(0,0,0);
  }
}


Vector3D Vector3D::FromPlane(PLANE_SELECT_et planeSelect)
{

  switch(planeSelect)
  {
    case PLANE_XY:
      return Vector3D(X,Y,Z);
    case PLANE_XZ:
      return Vector3D(Y,Z,X);
    case PLANE_YZ:
      return Vector3D(Z,X,Y);
    default:
      return Vector3D(0,0,0);
  }

}

Vector3D Vector3D::ToPlane(PLANE_SELECT_et planeSelect)
{
  switch(planeSelect)
  {
    case PLANE_XY:
      return Vector3D(X,Y,Z);
    case PLANE_XZ:
      return Vector3D(Z,X,Y);
    case PLANE_YZ:
      return Vector3D(Y,Z,X);
    default:
      return Vector3D(0,0,0);
  }
}