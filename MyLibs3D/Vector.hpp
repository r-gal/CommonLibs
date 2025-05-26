#ifndef VECTOR_LIB
#define VECTOR_LIB

#include <stdint.h>
#include "CommonDef.hpp"
#include "CNC2_def.hpp"
//#include "SignalList.hpp"

#define WERSOR2SPEED_SCALE ((1<<26)/1024)

typedef int32_t q31;



class iPoint3D;

#if USE_FLOAT_VECTORS == 1
class Vector;
class Vector3D;
class Point;
class Point3D;
class Radial;

class Vector
{
  public:
  float X,Y;

  float Normalize(void);
  float Length(void);
  Radial ToRadial(void);

  Vector(void){}
  Vector(float X_,float Y_) : X(X_), Y(Y_) {}
  

  Vector operator*  (float multi);
  Vector operator*= (float multi);
  Vector operator+ (Vector val);
  Vector operator+= (Vector val);

  Vector operator- (Vector val);
  Vector operator- (void);
};
class Vector3D
{
  public:
  float X,Y,Z;

  Vector3D(void){}
  Vector3D(float X_,float Y_, float Z_) : X(X_), Y(Y_), Z(Z_) {}

  float Normalize(void);
  float Length(void);
  void Abs(void);
  Radial ToRadial(void);

  Vector3D operator*= (float multi);

  Vector3D ToPlane(PLANE_SELECT_et plane);
  Vector3D FromPlane(PLANE_SELECT_et plane);
};
class Point
{
  public:
  float X,Y;
};
class Point3D
{
  public:
  float X,Y,Z;

  Point3D(void){}
  Point3D(float X_,float Y_, float Z_) : X(X_), Y(Y_), Z(Z_) {}

  Point3D ToPlane(PLANE_SELECT_et plane);
  Point3D FromPlane(PLANE_SELECT_et plane);

  operator iPoint3D  const();
  Point3D operator+ (Vector3D val);
  Point3D & operator +=( const Vector3D & d );
};
#endif






class iVector3D;


class iVectorNorm;
class iVectorNorm3D;

class iVector
{
   public:
  int x;
  int y;
  iVector operator*= (int multi);
  iVectorNorm Normalize();
};

class iPoint3D
{
   public:
  int x;
  int y;
  int z;

  iPoint3D(void){}
  iPoint3D(int X_,int Y_, int Z_) : x(X_), y(Y_), z(Z_) {}

  iPoint3D ToPlane(PLANE_SELECT_et plane);
  iPoint3D FromPlane(PLANE_SELECT_et plane);

  iPoint3D operator +( const iVector3D & d );
  iPoint3D & operator +=( const iVector3D & d );
#if USE_FLOAT_VECTORS == 1
  //Point3D(int x_,int y_, int z_)
  operator Point3D  const()
  {
    Point3D ret;
    ret.X = ((float)x)/(16000);
    ret.Y = ((float)y)/(16000);
    ret.Z = ((float)z)/(16000);
    return ret;
  };
  #endif
};



class iVector3D
{
   public:
  int x;
  int y;
  int z;
  iVector3D operator*= (int multi);

  iVector3D(void){}
  iVector3D(int X_,int Y_, int Z_) : x(X_), y(Y_), z(Z_) {}

  iVector3D ToPlane(PLANE_SELECT_et plane);
  iVector3D FromPlane(PLANE_SELECT_et plane);

  iVectorNorm3D Normalize();
};

class iVectorNorm
{
   public:
  int x; /*6.26 format */
  int y; /*6.26 format */
  int length;
  int angleXY;
};

class iVectorNorm3D
{
   public:
  int x; /*6.26 format */
  int y; /*6.26 format */
  int z; /*6.26 format */
  int length;
  int angleXY;
  int angleZ;  
};

class PlaneRadial
{
   public:
  int radius;
  int angle;
  int scaler;
  int z;
};

class Radial
{
  public:
  float radius;
  float angle;
  float Z;

  Vector ToVector(void);
  Vector3D ToVector3D(void);

};


class VectorMath_c
{


public :

  void Init();
  int GetLength(iPoint3D a,iPoint3D b);
  int GetLength(iVector3D* a);


  static iVector3D Wersor2Vector(iVectorNorm3D* wersor);

  PlaneRadial PlaneVector2Radial(iVector3D* vector);

  int GetArcLength(int startAngle, int endAngle, int r);
  iPoint3D GetArcPoint(iPoint3D* centrePoint, int angle, int radius, int radiusScaler);

  iVector3D GetVersorDiffence(iVectorNorm3D* a,iVectorNorm3D* b);

  uint32_t Sqrt(uint32_t in);

  #if USE_FLOAT_VECTORS == 1
  static float AngleBetween(Vector v1, Vector v2);
  //static VectorNorm3D Vector2WersorNew(Vector3D* vector);

  #endif

  
};

iVector3D operator -(const iPoint3D &a,const iPoint3D &b);
iVector3D operator +(const iPoint3D &a,const iPoint3D &b);

iVector3D operator -(const iVector3D &a,const iVector3D &b);
iVector3D operator +(const iVector3D &a,const iVector3D &b);

bool operator ==(const iPoint3D &a,const iPoint3D &b);

#if USE_FLOAT_VECTORS == 1
Vector3D operator -(const Point3D &a,const Point3D &b) ;
#endif

#endif