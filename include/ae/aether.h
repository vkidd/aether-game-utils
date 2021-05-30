//------------------------------------------------------------------------------
// aether.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
#ifndef AE_AETHER_H
#define AE_AETHER_H

//------------------------------------------------------------------------------
// Platform defines
//------------------------------------------------------------------------------
#define _AE_IOS_ 0
#define _AE_OSX_ 0
#define _AE_APPLE_ 0
#define _AE_WINDOWS_ 0
#define _AE_LINUX_ 0
#define _AE_EMSCRIPTEN_ 0
#if defined(__EMSCRIPTEN__)
  #undef _AE_EMSCRIPTEN_
  #define _AE_EMSCRIPTEN_ 1
#elif defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE
    #undef _AE_IOS_
    #define _AE_IOS_ 1
  #elif TARGET_OS_MAC
    #undef _AE_OSX_
    #define _AE_OSX_ 1
  #else
    #error "Platform not supported"
  #endif
  #undef _AE_APPLE_
  #define _AE_APPLE_ 1
#elif defined(_MSC_VER)
  #undef _AE_WINDOWS_
  #define _AE_WINDOWS_ 1
#elif defined(__linux__)
  #undef _AE_LINUX_
  #define _AE_LINUX_ 1
#else
  #error "Platform not supported"
#endif

//------------------------------------------------------------------------------
// Debug define
//------------------------------------------------------------------------------
#if defined(_DEBUG) || defined(DEBUG) || ( _AE_APPLE_ && !defined(NDEBUG) )
  #define _AE_DEBUG_ 1
#else
  #define _AE_DEBUG_ 0
#endif

//------------------------------------------------------------------------------
// Warnings
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif
  #pragma warning( disable : 4244 )
  #pragma warning( disable : 4800 )
#endif

#if _AE_APPLE_
  #define GL_SILENCE_DEPRECATION
#endif

//------------------------------------------------------------------------------
// ae Namespace
//------------------------------------------------------------------------------
#define AE_NAMESPACE ae

//------------------------------------------------------------------------------
// System Headers
//------------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <functional>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <utility>

//------------------------------------------------------------------------------
// SIMD headers
//------------------------------------------------------------------------------
#if _AE_APPLE_
  #ifdef __aarch64__
    #include <arm_neon.h>
  #else
    #include <x86intrin.h>
  #endif
#elif _AE_WINDOWS_
  #include <intrin.h>
#endif

//------------------------------------------------------------------------------
// Platform Utils
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define aeAssert() __debugbreak()
#elif _AE_APPLE_
  #define aeAssert() __builtin_trap()
#elif _AE_EMSCRIPTEN_
  // @TODO: Handle asserts with emscripten builds
  #define aeAssert()
#else
  #define aeAssert() asm( "int $3" )
#endif

#if _AE_WINDOWS_
  #define aeCompilationWarning( _msg ) _Pragma( message _msg )
#else
  #define aeCompilationWarning( _msg ) _Pragma( "warning #_msg" )
#endif

#if _AE_LINUX_
  #define AE_ALIGN( _x ) __attribute__ ((aligned(_x)))
//#elif _AE_WINDOWS_
  // @TODO: Windows doesn't support aligned function parameters
  //#define AE_ALIGN( _x ) __declspec(align(_x))
#else
  #define AE_ALIGN( _x )
#endif

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
template < typename T, int N > char( &countof_helper( T(&)[ N ] ) )[ N ];
#define countof( _x ) ( (uint32_t)sizeof( countof_helper( _x ) ) )

#define AE_CALL_CONST( _tx, _x, _tfn, _fn ) const_cast< _tfn* >( const_cast< const _tx* >( _x )->_fn() );

namespace AE_NAMESPACE {

//------------------------------------------------------------------------------
// Platform functions
//------------------------------------------------------------------------------
uint32_t GetPID();
uint32_t GetMaxConcurrentThreads();
bool IsDebuggerAttached();
template < typename T > const char* GetTypeName();
double GetTime();

//------------------------------------------------------------------------------
// Tags
//------------------------------------------------------------------------------
using Tag = std::string; // @TODO: Fixed length string
#define AE_ALLOC_TAG_RENDER ae::Tag( "aeGraphics" )
#define AE_ALLOC_TAG_AUDIO ae::Tag( "aeAudio" )
#define AE_ALLOC_TAG_META ae::Tag( "aeMeta" )
#define AE_ALLOC_TAG_TERRAIN ae::Tag( "aeTerrain" )
#define AE_ALLOC_TAG_NET ae::Tag( "aeNet" )
#define AE_ALLOC_TAG_HOTSPOT ae::Tag( "aeHotSpot" )
#define AE_ALLOC_TAG_MESH ae::Tag( "aeMesh" )
#define AE_ALLOC_TAG_SCRATCH ae::Tag( "aeScratch" )
#define AE_ALLOC_TAG_FIXME ae::Tag( "aeFixMe" )
#define AE_ALLOC_TAG_FILE ae::Tag( "aeFile" )

//------------------------------------------------------------------------------
// Allocator interface
// @NOTE: By default aether-game-utils uses system allocations, which may be fine
// for your use case. If not it's advised that you implement this class with
// dlmalloc or similar and then call ae::SetGlobalAllocator() with your own
// allocator when your program starts.
//------------------------------------------------------------------------------
class Allocator
{
public:
  virtual ~Allocator() {}
  virtual void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) = 0;
  virtual void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) = 0;
  virtual void Free( void* data ) = 0;
};
//------------------------------------------------------------------------------
// @NOTE: Call ae::SetGlobalAllocator() before making any allocations or else a
// default allocator will be used. You must call ae::SetGlobalAllocator() before
// any allocations are made.
//------------------------------------------------------------------------------
void SetGlobalAllocator( Allocator* alloc );
Allocator* GetGlobalAllocator();
//------------------------------------------------------------------------------
// Allocation functions
// @NOTE: All allocations are tagged, (@TODO) they can be inspected through the
// current ae::Allocator with ae::GetGlobalAllocator()
//------------------------------------------------------------------------------
// C++ style allocations
template < typename T > T* NewArray( ae::Tag tag, uint32_t count );
template < typename T, typename ... Args > T* New( ae::Tag tag, Args ... args );
template < typename T > void Delete( T* obj );
// C style allocations
void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment );
void* Reallocate( void* data, uint32_t bytes, uint32_t alignment );
void Free( void* data );

//------------------------------------------------------------------------------
// ae::Scratch< T > class
//------------------------------------------------------------------------------
// @NOTE: This class is useful for scoped allocations. (@TODO) In the future it
// should allocate from a stack. This should allow big allocations to happen
// cheaply each frame without creating any fragmentation.
template < typename T >
class Scratch
{
public:
  Scratch( uint32_t count );
  ~Scratch();
  
  T* Data();
  uint32_t Length() const;

  T& operator[] ( int32_t index );
  const T& operator[] ( int32_t index ) const;
  T& GetSafe( int32_t index );
  const T& GetSafe( int32_t index ) const;

private:
  T* m_data;
  uint32_t m_count;
};

//------------------------------------------------------------------------------
// Math defines
//------------------------------------------------------------------------------
const float PI = 3.14159265358979323846f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = 0.5f * PI;
const float QUARTER_PI = 0.25f * PI;

//------------------------------------------------------------------------------
// Standard math operations
//------------------------------------------------------------------------------
inline float Pow( float x, float e );
inline float Cos( float x );
inline float Sin( float x );
inline float Atan2( float y, float x );

inline uint32_t Mod( uint32_t i, uint32_t n );
inline int Mod( int32_t i, int32_t n );
inline float Mod( float f, float n );

inline int32_t Ceil( float f );
inline int32_t Floor( float f );
inline int32_t Round( float f );

//------------------------------------------------------------------------------
// Range functions
//------------------------------------------------------------------------------
template< typename T0, typename T1, typename... Tn > auto Min( T0&& v0, T1&& v1, Tn&&... vn );
template< typename T0, typename T1, typename... Tn > auto Max( T0&& v0, T1&& v1, Tn&&... vn );
template< typename T > inline T Abs( const T &x );

template < typename T > inline T Clip( T x, T min, T max );
inline float Clip01( float x );

//------------------------------------------------------------------------------
// Interpolation
//------------------------------------------------------------------------------
template< typename T > T Lerp( T start, T end, float t );
inline float Delerp( float start, float end, float value );
inline float Delerp01( float start, float end, float value );
template< typename T > T DtLerp( T value, float snappiness, float dt, T target );
inline float DtLerpAngle( float value, float snappiness, float dt, float target );
// @TODO: Cleanup duplicate interpolation functions
template< typename T > T CosineInterpolate( T start, T end, float t );
namespace Interpolation
{
  template< typename T > T Linear( T start, T end, float t );
  template< typename T > T Cosine( T start, T end, float t );
}

//------------------------------------------------------------------------------
// Angle functions
//------------------------------------------------------------------------------
inline float DegToRad( float degrees );
inline float RadToDeg( float radians );

//------------------------------------------------------------------------------
// Type specific limits
//------------------------------------------------------------------------------
template< typename T > constexpr T MaxValue();
template< typename T > constexpr T MinValue();

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
// @NOTE: Vec2 Vec3 and Vec4 share these functions. They act on each component
// of the vector, so in the case of Vec4 a dot product is implemented as
// (a.x*b.x)+(a.y*b.y)+(a.z*b.z)+(a.w*b.w).
template < typename T >
struct VecT
{
  VecT() {}
  VecT( bool ) = delete;

  bool operator==( const T& v ) const;
  bool operator!=( const T& v ) const;
  
  float operator[]( uint32_t idx ) const;
  float& operator[]( uint32_t idx );
  
  T operator+( const T& v ) const;
  void operator+=( const T& v );
  T operator-() const;
  T operator-( const T& v ) const;
  void operator-=( const T& v );
  T operator*( float s ) const;
  void operator*=( float s );
  T operator/( float s ) const;
  void operator/=( float s );
  
  static float Dot( const T& v0, const T& v1 );
  float Dot( const T& v ) const;
  float Length() const;
  float LengthSquared() const;
  
  float Normalize();
  float SafeNormalize( float epsilon = 0.000001f );
  T NormalizeCopy() const;
  T SafeNormalizeCopy( float epsilon = 0.000001f ) const;
  float Trim( float length );
};

#pragma warning(disable:26495) // Vecs are left uninitialized for performance

//------------------------------------------------------------------------------
// ae::Vec2 struct
//------------------------------------------------------------------------------
struct Vec2 : public VecT< Vec2 >
{
  Vec2() {} // Empty default constructor for performance of vertex arrays etc
  Vec2( const Vec2& ) = default;
  explicit Vec2( float v );
  Vec2( float x, float y );
  explicit Vec2( const float* v2 );
  static Vec2 FromAngle( float angle );
  struct
  {
    float x;
    float y;
  };
  float data[ 2 ];
};
// @HACK: For Window
using Int2 = Vec2;

//------------------------------------------------------------------------------
// ae::Vec3 struct
//------------------------------------------------------------------------------
struct Vec3 : public VecT< Vec3 >
{
  Vec3() {} // Empty default constructor for performance of vertex arrays etc
  explicit Vec3( float v );
  Vec3( float x, float y, float z );
  explicit Vec3( const float* v3 );
  Vec3( Vec2 xy, float z ); // @TODO: Support Y up
  explicit Vec3( Vec2 xy );
  explicit operator Vec2() const;
  Vec2 GetXY() const;
  Vec2 GetXZ() const;
  // Int3 NearestCopy() const; // @TODO
  // Int3 FloorCopy() const; // @TODO
  // Int3 CeilCopy() const; // @TODO
  
  float GetAngleBetween( const Vec3& v, float epsilon = 0.0001f ) const;
  void AddRotationXY( float rotation ); // @TODO: Support Y up
  Vec3 RotateCopy( Vec3 axis, float angle ) const;
  Vec3 Lerp( const Vec3& end, float t ) const;
  Vec3 Slerp( const Vec3& end, float t, float epsilon = 0.0001f ) const;
  
  static Vec3 Cross( const Vec3& v0, const Vec3& v1 );
  Vec3 Cross( const Vec3& v ) const;
  void ZeroAxis( Vec3 axis ); // Zero component along arbitrary axis (ie vec dot axis == 0)
  void ZeroDirection( Vec3 direction ); // Zero component along positive half of axis (ie vec dot dir > 0)

  // static Vec3 ProjectPoint( const class Matrix4& projection, Vec3 p ); // @TODO
  // static Vec3 ProjectVector( const class Matrix4& projection, Vec3 p ); // @TODO
  
  union
  {
    struct
    {
      float x;
      float y;
      float z;
    };
    float data[ 3 ];
  };
  float pad;
};

//------------------------------------------------------------------------------
// ae::Vec4 struct
//------------------------------------------------------------------------------
struct Vec4 : public VecT< Vec4 >
{
  Vec4() {} // Empty default constructor for performance of vertex arrays etc
  Vec4( const Vec4& ) = default;
  explicit Vec4( float f );
  explicit Vec4( float* v );
  explicit Vec4( float xyz, float w );
  Vec4( float x, float y, float z, float w );
  Vec4( Vec3 xyz, float w );
  Vec4( Vec2 xy, float z, float w );
  Vec4( Vec2 xy, Vec2 zw );
  explicit operator Vec2() const;
  explicit operator Vec3() const;
  Vec4( const float* v3, float w );
  explicit Vec4( const float* v4 );
  Vec2 GetXY() const;
  Vec2 GetZW() const;
  Vec3 GetXYZ() const;
  union
  {
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
    float data[ 4 ];
  };
};

//------------------------------------------------------------------------------
// ae::Color struct
//------------------------------------------------------------------------------
struct Color
{
  Color() {} // Empty default constructor for performance of vertex arrays etc
  Color( const Color& ) = default;
  Color( float rgb );
  Color( float r, float g, float b );
  Color( float r, float g, float b, float a );
  Color( Color c, float a );
  static Color R( float r );
  static Color RG( float r, float g );
  static Color RGB( float r, float g, float b );
  static Color RGBA( float r, float g, float b, float a );
  static Color RGBA( const float* v );
  static Color SRGB( float r, float g, float b );
  static Color SRGBA( float r, float g, float b, float a );
  static Color R8( uint8_t r );
  static Color RG8( uint8_t r, uint8_t g );
  static Color RGB8( uint8_t r, uint8_t g, uint8_t b );
  static Color RGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
  static Color SRGB8( uint8_t r, uint8_t g, uint8_t b );
  static Color SRGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a );

  Vec3 GetLinearRGB() const;
  Vec4 GetLinearRGBA() const;
  Vec3 GetSRGB() const;
  Vec4 GetSRGBA() const;

  Color Lerp( const Color& end, float t ) const;
  Color DtLerp( float snappiness, float dt, const Color& target ) const;
  Color ScaleRGB( float s ) const;
  Color ScaleA( float s ) const;
  Color SetA( float alpha ) const;

  static float SRGBToRGB( float x );
  static float RGBToSRGB( float x );

  // Grayscale
  static Color White();
  static Color Gray();
  static Color Black();
  // Rainbow
  static Color Red();
  static Color Orange();
  static Color Yellow();
  static Color Green();
  static Color Blue();
  static Color Indigo();
  static Color Violet();
  // Pico
  static Color PicoBlack();
  static Color PicoDarkBlue();
  static Color PicoDarkPurple();
  static Color PicoDarkGreen();
  static Color PicoBrown();
  static Color PicoDarkGray();
  static Color PicoLightGray();
  static Color PicoWhite();
  static Color PicoRed();
  static Color PicoOrange();
  static Color PicoYellow();
  static Color PicoGreen();
  static Color PicoBlue();
  static Color PicoIndigo();
  static Color PicoPink();
  static Color PicoPeach();
  // Misc
  static Color Magenta();

  float r;
  float g;
  float b;
  float a;

private:
  // Delete implicit conversions to try to catch color space issues
  template < typename T > Color R( T r ) = delete;
  template < typename T > Color RG( T r, T g ) = delete;
  template < typename T > Color RGB( T r, T g, T b ) = delete;
  template < typename T > Color RGBA( T r, T g, T b, T a ) = delete;
  template < typename T > Color RGBA( const T* v ) = delete;
  template < typename T > Color SRGB( T r, T g, T b ) = delete;
  template < typename T > Color SRGBA( T r, T g, T b, T a ) = delete;
};

#pragma warning(default:26495) // Re-enable uninitialized variable warning

//------------------------------------------------------------------------------
// Random values
//------------------------------------------------------------------------------
inline int32_t Random( int32_t min, int32_t max );
inline float Random( float min, float max );
inline bool RandomBool();

template < typename T >
class RandomValue
{
public:
  RandomValue() {}
  RandomValue( T min, T max );
  RandomValue( T value );
  
  void SetMin( T min );
  void SetMax( T max );
  
  T GetMin() const;
  T GetMax() const;
  
  T Get() const;
  operator T() const;
  
private:
  T m_min = T();
  T m_max = T();
};

//------------------------------------------------------------------------------
// ae::TimeStep
//------------------------------------------------------------------------------
class TimeStep
{
public:
  TimeStep();

  void SetTimeStep( float timeStep );
  float GetTimeStep() const;
  uint32_t GetStepCount() const;

  float GetDt() const;
  float SetDt( float sec ); // Useful for handling frames with high delta time, eg: timeStep.SetDt( timeStep.GetTimeStep() )

  void Wait();

private:
  uint32_t m_stepCount = 0;
  float m_timeStepSec = 0.0f;
  float m_timeStep = 0.0f;
  int64_t m_frameExcess = 0;
  float m_prevFrameTime = 0.0f;
  float m_prevFrameTimeSec = 0.0f;
  std::chrono::steady_clock::time_point m_frameStart;
};

//------------------------------------------------------------------------------
// ae::Str class
// @NOTE: A fixed length string class. The templated value is the total size of
// the string in memory.
//------------------------------------------------------------------------------
template < uint32_t N >
class Str
{
public:
  Str();
  template < uint32_t N2 > Str( const Str<N2>& str );
  Str( const char* str );
  Str( uint32_t length, const char* str );
  Str( uint32_t length, char c );
  template < typename... Args > Str( const char* format, Args... args );
  template < typename... Args > static Str< N > Format( const char* format, Args... args );
  explicit operator const char*() const;
  
  template < uint32_t N2 > void operator =( const Str<N2>& str );
  template < uint32_t N2 > Str<N> operator +( const Str<N2>& str ) const;
  template < uint32_t N2 > void operator +=( const Str<N2>& str );
  template < uint32_t N2 > bool operator ==( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator !=( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator <( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator >( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator <=( const Str<N2>& str ) const;
  template < uint32_t N2 > bool operator >=( const Str<N2>& str ) const;
  Str<N> operator +( const char* str ) const;
  void operator +=( const char* str );
  bool operator ==( const char* str ) const;
  bool operator !=( const char* str ) const;
  bool operator <( const char* str ) const;
  bool operator >( const char* str ) const;
  bool operator <=( const char* str ) const;
  bool operator >=( const char* str ) const;

  char& operator[]( uint32_t i );
  const char operator[]( uint32_t i ) const;
  const char* c_str() const;

  template < uint32_t N2 >
  void Append( const Str<N2>& str );
  void Append( const char* str );
  void Trim( uint32_t len );

  uint32_t Length() const;
  uint32_t Size() const;
  bool Empty() const;
  static constexpr uint32_t MaxLength() { return N - 3u; } // Leave room for length var and null terminator

private:
  template < uint32_t N2 > friend class Str;
  template < uint32_t N2 > friend bool operator ==( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator !=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator <( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator >( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator <=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend bool operator >=( const char*, const Str< N2 >& );
  template < uint32_t N2 > friend std::istream& operator>>( std::istream&, Str< N2 >& );
  void m_Format( const char* format );
  template < typename T, typename... Args >
  void m_Format( const char* format, T value, Args... args );
  uint16_t m_length;
  char m_str[ MaxLength() + 1u ];
};
// Predefined lengths
using Str16 = Str< 16 >;
using Str32 = Str< 32 >;
using Str64 = Str< 64 >;
using Str128 = Str< 128 >;
using Str256 = Str< 256 >;
using Str512 = Str< 512 >;

//------------------------------------------------------------------------------
// ae::Array class
//------------------------------------------------------------------------------
template < typename T, uint32_t N = 0 >
class Array
{
public:
  // Static array (N > 0)
  Array();
  Array( uint32_t length, const T& val ); // Appends 'length' number of 'val's
  // Dynamic array (N == 0)
  Array( ae::Tag tag );
  Array( ae::Tag tag, uint32_t size ); // Reserve size (with length of 0)
  Array( ae::Tag tag, uint32_t length, const T& val ); // Reserves 'length' and appends 'length' number of 'val's
  void Reserve( uint32_t total );
  // Static and dynamic arrays
  Array( const Array< T, N >& other );
  Array( Array< T, N >&& other ) noexcept; // Move operators fallback to regular operators if ae::Tags don't match
  void operator =( const Array< T, N >& other );
  void operator =( Array< T, N >&& other ) noexcept;
  ~Array();
  
  // Add elements
  T& Append( const T& value );
  void Append( const T* values, uint32_t count );
  T& Insert( uint32_t index, const T& value );

  // Find elements
  template < typename U > int32_t Find( const U& value ) const; // Returns -1 when not found
  template < typename Fn > int32_t FindFn( Fn testFn ) const; // Returns -1 when not found

  // Remove elements
  template < typename U > uint32_t RemoveAll( const U& value );
  template < typename Fn > uint32_t RemoveAllFn( Fn testFn );
  void Remove( uint32_t index );
  void Clear();

  // Access elements
  const T& operator[]( int32_t index ) const; // Performs bounds checking in debug mode. Use 'Begin()' to get raw array.
  T& operator[]( int32_t index );
  T* Begin() { return m_array; } // These functions can return null when array length is zero
  T* End() { return m_array + m_length; }
  const T* Begin() const { return m_array; }
  const T* End() const { return m_array + m_length; }

  // Array info
  uint32_t Length() const;
  uint32_t Size() const;
  
private:
  uint32_t m_GetNextSize() const;
  uint32_t m_length;
  uint32_t m_size;
  T* m_array;
  ae::Tag m_tag;
  typedef typename std::aligned_storage< sizeof(T), alignof(T) >::type AlignedStorageT;
  template < uint32_t > struct Storage { AlignedStorageT data[ N ]; };
  template <> struct Storage< 0 > {};
  Storage< N > m_storage;
public:
  // @NOTE: Ranged-based loop. Lowercase to match c++ standard ('-.-)
  T* begin() { return m_array; }
  T* end() { return m_array + m_length; }
  const T* begin() const { return m_array; }
  const T* end() const { return m_array + m_length; }
};

//------------------------------------------------------------------------------
// ae::Map class
//------------------------------------------------------------------------------
template < typename K, typename V, uint32_t N = 0 >
class Map
{
public:
  Map(); // Static map only (N > 0)
  Map( ae::Tag pool ); // Dynamic map only (N == 0)
  
  V& Set( const K& key, const V& value );
  V& Get( const K& key );
  const V& Get( const K& key ) const;
  const V& Get( const K& key, const V& defaultValue ) const;
  
  V* TryGet( const K& key );
  const V* TryGet( const K& key ) const;

  bool TryGet( const K& key, V* valueOut );
  bool TryGet( const K& key, V* valueOut ) const;
  
  bool Remove( const K& key );
  bool Remove( const K& key, V* valueOut );

  void Reserve( uint32_t total );
  void Clear();

  K& GetKey( uint32_t index );
  V& GetValue( uint32_t index );
  const K& GetKey( uint32_t index ) const;
  const V& GetValue( uint32_t index ) const;
  uint32_t Length() const;

private:
  template < typename K2, typename V2, uint32_t N2 >
  friend std::ostream& operator<<( std::ostream&, const Map< K2, V2, N2 >& );

  struct Entry
  {
    Entry() = default;
    Entry( const K& k, const V& v );

    K key;
    V value;
  };

  int32_t m_FindIndex( const K& key ) const;

  Array< Entry, N > m_entries;
};

} // AE_NAMESPACE end

//------------------------------------------------------------------------------
// Logging functions
//------------------------------------------------------------------------------
#define AE_TRACE(...) ae::LogInternal( _AE_LOG_TRACE_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_DEBUG(...) ae::LogInternal( _AE_LOG_DEBUG_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_LOG(...) ae::LogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_INFO(...) ae::LogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_WARN(...) ae::LogInternal( _AE_LOG_WARN_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_ERR(...) ae::LogInternal( _AE_LOG_ERROR_, __FILE__, __LINE__, "", __VA_ARGS__ )

//------------------------------------------------------------------------------
// Assertion functions
//------------------------------------------------------------------------------
// @TODO: Use __analysis_assume( x ); on windows to prevent warning C6011 (Dereferencing NULL pointer)
#define AE_ASSERT( _x ) do { if ( !(_x) ) { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", "" ); aeAssert(); } } while (0)
#define AE_ASSERT_MSG( _x, ... ) do { if ( !(_x) ) { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", __VA_ARGS__ ); aeAssert(); } } while (0)
#define AE_FAIL() do { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", "" ); aeAssert(); } while (0)
#define AE_FAIL_MSG( ... ) do { ae::LogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", __VA_ARGS__ ); aeAssert(); } while (0)

//------------------------------------------------------------------------------
// Static assertion functions
//------------------------------------------------------------------------------
#define AE_STATIC_ASSERT( _x ) static_assert( _x, "static assert" )
#define AE_STATIC_ASSERT_MSG( _x, _m ) static_assert( _x, _m )
#define AE_STATIC_FAIL( _m ) static_assert( 0, _m )

//------------------------------------------------------------------------------
// Handle missing 'standard' C functions
//------------------------------------------------------------------------------
#ifndef HAVE_STRLCAT
inline size_t strlcat( char* dst, const char* src, size_t size )
{
  size_t dstlen = strlen( dst );
  size -= dstlen + 1;

  if ( !size )
  {
    return dstlen;
  }

  size_t srclen = strlen( src );
  if ( srclen > size )
  {
    srclen = size;
  }

  memcpy( dst + dstlen, src, srclen );
  dst[ dstlen + srclen ] = '\0';

  return ( dstlen + srclen );
}
#endif

#ifndef HAVE_STRLCPY
inline size_t strlcpy( char* dst, const char* src, size_t size )
{
  size--;

  size_t srclen = strlen( src );
  if ( srclen > size )
  {
    srclen = size;
  }

  memcpy( dst, src, srclen );
  dst[ srclen ] = '\0';

  return srclen;
}
#endif

#ifdef _MSC_VER
# define strtok_r strtok_s
#endif

namespace AE_NAMESPACE {

// @HACK:
struct Rect
{};
struct Matrix4
{};

//------------------------------------------------------------------------------
// ae::Window class
//------------------------------------------------------------------------------
class Window
{
public:
  Window();
  bool Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor );
  bool Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor );
  void Terminate();

  void SetTitle( const char* title );
  void SetFullScreen( bool fullScreen );
  void SetPosition( Int2 pos );
  void SetSize( uint32_t width, uint32_t height );
  void SetMaximized( bool maximized );

  const char* GetTitle() const { return m_windowTitle.c_str(); }
  Int2 GetPosition() const { return m_pos; }
  int32_t GetWidth() const { return m_width; }
  int32_t GetHeight() const { return m_height; }
  bool GetFullScreen() const { return m_fullScreen; }
  bool GetMaximized() const { return m_maximized; }

private:
  void m_Initialize();
    
  Int2 m_pos;
  int32_t m_width;
  int32_t m_height;
  bool m_fullScreen;
  bool m_maximized;
  Str256 m_windowTitle;

public:
  // Internal
  void m_UpdatePos( Int2 pos ) { m_pos = pos; }
  void m_UpdateWidthHeight( int32_t width, int32_t height ) { m_width = width; m_height = height; }
  void m_UpdateMaximized( bool maximized ) { m_maximized = maximized; }

  void* window;
  class GraphicsDevice* graphicsDevice;
};

//------------------------------------------------------------------------------
// ae::Input class
//------------------------------------------------------------------------------
class Input
{
public:
  void Pump();
  bool quit = false;
};

//------------------------------------------------------------------------------
// Graphics Constants
//------------------------------------------------------------------------------
enum class TextureFilter
{
  Linear,
  Nearest
};
enum class TextureWrap
{
  Repeat,
  Clamp
};

//------------------------------------------------------------------------------
// UniformList class
//------------------------------------------------------------------------------
class UniformList
{
public:
};

//------------------------------------------------------------------------------
// Shader class
//------------------------------------------------------------------------------
class Shader
{
public:
};

//------------------------------------------------------------------------------
// VertexData class
//------------------------------------------------------------------------------
class VertexData
{
public:
};

//------------------------------------------------------------------------------
// Texture2D class
//------------------------------------------------------------------------------
class Texture2D
{
public:
};

//------------------------------------------------------------------------------
// RenderTarget class
//------------------------------------------------------------------------------
class RenderTarget
{
public:
  ~RenderTarget();
  void Initialize( uint32_t width, uint32_t height );
  void AddTexture( TextureFilter filter, TextureWrap wrap );
  void AddDepth( TextureFilter filter, TextureWrap wrap );
  void Destroy();

  void Activate();
  void Clear( Color color );
  void Render( const Shader* shader, const UniformList& uniforms );
  void Render2D( uint32_t textureIndex, Rect ndc, float z );

  const Texture2D* GetTexture( uint32_t index ) const;
  const Texture2D* GetDepth() const;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  // @NOTE: Get ndc space rect of this target within another target (fill but maintain aspect ratio)
  // GetNDCFillRectForTarget( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height() )
  // GetNDCFillRectForTarget( GraphicsDeviceTarget()::GetWidth(),  GraphicsDeviceTarget()::Height() )
  Rect GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const;

  // @NOTE: Other target to local transform (pixels->pixels)
  // Useful for transforming window/mouse pixel coordinates to local pixels
  // GetTargetPixelsToLocalTransform( GraphicsDevice::GetWindow()::GetWidth(),  GraphicsDevice::GetWindow()::Height(), GetNDCFillRectForTarget( ... ) )
  Matrix4 GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const;

  // @NOTE: Mouse/window pixel coordinates to world space
  // GetTargetPixelsToWorld( GetTargetPixelsToLocalTransform( ... ), TODO )
  Matrix4 GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const;

  // @NOTE: Creates a transform matrix from aeQuad vertex positions to ndc space
  // GraphicsDeviceTarget uses aeQuad vertices internally
  static Matrix4 GetQuadToNDCTransform( Rect ndc, float z );

private:
  struct Vertex
  {
    Vec3 pos;
    Vec2 uv;
  };

  uint32_t m_fbo = 0;

  Array< Texture2D*, 4 > m_targets;
  Texture2D m_depth;

  uint32_t m_width = 0;
  uint32_t m_height = 0;

  VertexData m_quad;
  Shader m_shader;
};

//------------------------------------------------------------------------------
// ae::GraphicsDevice class
//------------------------------------------------------------------------------
class GraphicsDevice
{
public:
  GraphicsDevice();
  ~GraphicsDevice();

  void Initialize( class Window* window );
  void Terminate();

  void Activate();
  void Clear( Color color );
  void Present();

  class Window* GetWindow() { return m_window; }
  RenderTarget* GetCanvas() { return &m_canvas; }

  uint32_t GetWidth() const { return m_canvas.GetWidth(); }
  uint32_t GetHeight() const { return m_canvas.GetHeight(); }
  float GetAspectRatio() const;

  // have to inject a barrier to readback from active render target (GL only)
  void AddTextureBarrier();

//private:
  friend class ae::Window;
  void m_HandleResize( uint32_t width, uint32_t height );

  Window* m_window;
  RenderTarget m_canvas;

  // OpenGL
  void* m_context;
  int32_t m_defaultFbo;
};

} // AE_NAMESPACE end

//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
//
//
//
//
// Internal implementation beyond this point
//
//
//
//
//------------------------------------------------------------------------------
// Platform internal implementation
//------------------------------------------------------------------------------
namespace AE_NAMESPACE {

template < typename T >
const char* GetTypeName()
{
  const char* typeName = typeid( T ).name();
#ifdef _MSC_VER
  if ( strncmp( typeName, "class ", 6 ) == 0 )
  {
    typeName += 6;
  }
  else if ( strncmp( typeName, "struct ", 7 ) == 0 )
  {
    typeName += 7;
  }
#else
  while ( *typeName && isdigit( typeName[ 0 ] ) )
  {
    typeName++;
  }
#endif
  return typeName;
}

#if defined(__aarch64__) && _AE_OSX_
  // @NOTE: Typeinfo appears to be missing for float16_t
  template <> const char* GetTypeName< float16_t >();
#endif

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
#define _AE_LOG_TRACE_ 0
#define _AE_LOG_DEBUG_ 1
#define _AE_LOG_INFO_ 2
#define _AE_LOG_WARN_ 3
#define _AE_LOG_ERROR_ 4
#define _AE_LOG_FATAL_ 5
extern const char* LogLevelNames[ 6 ];

//------------------------------------------------------------------------------
// Log colors internal implementation
//------------------------------------------------------------------------------
#if _AE_WINDOWS_ || _AE_APPLE_
#define _AE_LOG_COLORS_ false
#else
#define _AE_LOG_COLORS_ true
extern const char* LogLevelColors[ 6 ];
#endif

//------------------------------------------------------------------------------
// Internal Logging functions internal implementation
//------------------------------------------------------------------------------
void LogInternal( std::stringstream& os, const char* message );
void LogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format );

template < typename T, typename... Args >
void LogInternal( std::stringstream& os, const char* format, T value, Args... args )
{
  if ( !*format )
  {
    os << std::endl;
    return;
  }
  
  const char* head = format;
  while ( *head && *head != '#' )
  {
    head++;
  }
  if ( head > format )
  {
    os.write( format, head - format );
  }

  if ( *head == '#' )
  {
    os << value;
    head++;
  }

  LogInternal( os, head, args... );
}

template < typename... Args >
void LogInternal( uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format, Args... args )
{
  std::stringstream os;
  LogFormat( os, severity, filePath, line, assertInfo, format );
  LogInternal( os, format, args... );
}

//------------------------------------------------------------------------------
// C++ style allocation functions
//------------------------------------------------------------------------------
const uint32_t kDefaultAlignment = 16;
const uint32_t kHeaderSize = 16;
struct Header
{
  uint32_t check;
  uint32_t count;
  uint32_t size;
  uint32_t typeSize;
};

template < typename T >
T* NewArray( ae::Tag tag, uint32_t count )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );
  AE_STATIC_ASSERT( sizeof( T ) % alignof( T ) == 0 ); // All elements in array should have correct alignment

  uint32_t size = kHeaderSize + sizeof( T ) * count;
  uint8_t* base = (uint8_t*)ae::Allocate( tag, size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  AE_STATIC_ASSERT( sizeof( Header ) <= kHeaderSize );
  AE_STATIC_ASSERT( kHeaderSize % kDefaultAlignment == 0 );

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = count;
  header->size = size;
  header->typeSize = sizeof( T );

  T* result = (T*)( base + kHeaderSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    new( &result[ i ] ) T();
  }

  return result;
}

template < typename T, typename ... Args >
T* New( ae::Tag tag, Args ... args )
{
  AE_STATIC_ASSERT( alignof( T ) <= kDefaultAlignment );

  uint32_t size = kHeaderSize + sizeof( T );
  uint8_t* base = (uint8_t*)ae::Allocate( tag, size, kDefaultAlignment );
  AE_ASSERT( (intptr_t)base % kDefaultAlignment == 0 );
#if _AE_DEBUG_
  memset( (void*)base, 0xCD, size );
#endif

  Header* header = (Header*)base;
  header->check = 0xABCD;
  header->count = 1;
  header->size = size;
  header->typeSize = sizeof( T );

  return new( (T*)( base + kHeaderSize ) ) T( args ... );
}

template < typename T >
void Delete( T* obj )
{
  if ( !obj )
  {
    return;
  }

  AE_ASSERT( (intptr_t)obj % kDefaultAlignment == 0 );
  uint8_t* base = (uint8_t*)obj - kHeaderSize;

  Header* header = (Header*)( base );
  AE_ASSERT( header->check == 0xABCD );

  uint32_t count = header->count;
  AE_ASSERT_MSG( sizeof( T ) <= header->typeSize, "Released type T '#' does not match allocated type of size #", ae::GetTypeName< T >(), header->typeSize );
  for ( uint32_t i = 0; i < count; i++ )
  {
    T* o = (T*)( (uint8_t*)obj + header->typeSize * i );
    o->~T();
  }

#if _AE_DEBUG_
  memset( (void*)base, 0xDD, header->size );
#endif

  ae::Free( base );
}

//------------------------------------------------------------------------------
// C style allocations
//------------------------------------------------------------------------------
inline void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment )
{
  return ae::GetGlobalAllocator()->Allocate( tag, bytes, alignment );
}

inline void* Reallocate( void* data, uint32_t bytes, uint32_t alignment )
{
  return ae::GetGlobalAllocator()->Reallocate( data, bytes, alignment );
}

inline void Free( void* data )
{
  ae::GetGlobalAllocator()->Free( data );
}

//------------------------------------------------------------------------------
// ae::Scratch< T > member functions
//------------------------------------------------------------------------------
template < typename T >
Scratch< T >::Scratch( uint32_t count )
{
  m_count = count;
  m_data = ae::NewArray< T >( AE_ALLOC_TAG_SCRATCH, count );
}

template < typename T >
Scratch< T >::~Scratch()
{
  ae::Delete( m_data );
}

template < typename T >
T* Scratch< T >::Data()
{
  return m_data;
}

template < typename T >
uint32_t Scratch< T >::Length() const
{
  return m_count;
}

template < typename T >
T& Scratch< T >::operator[] ( int32_t index )
{
  return m_data[ index ];
}

template < typename T >
const T& Scratch< T >::operator[] ( int32_t index ) const
{
  return m_data[ index ];
}

template < typename T >
T& Scratch< T >::GetSafe( int32_t index )
{
  AE_ASSERT( index < (int32_t)m_count );
  return m_data[ index ];
}

template < typename T >
const T& Scratch< T >::GetSafe( int32_t index ) const
{
  AE_ASSERT( index < (int32_t)m_count );
  return m_data[ index ];
}

//------------------------------------------------------------------------------
// Math function implementations
//------------------------------------------------------------------------------
template< typename T >
T&& Min( T&& v )
{
  return std::forward< T >( v );
}
template< typename T0, typename T1, typename... Tn >
auto Min( T0&& v0, T1&& v1, Tn&&... vn )
{
  return ( v0 < v1 ) ? Min( v0, std::forward< Tn >( vn )... ) : Min( v1, std::forward< Tn >( vn )... );
}

template< typename T >
T&& Max( T&& v )
{
  return std::forward< T >( v );
}
template< typename T0, typename T1, typename... Tn >
auto Max( T0&& v0, T1&& v1, Tn&&... vn )
{
  return ( v0 > v1 ) ? Max( v0, std::forward< Tn >( vn )... ) : Max( v1, std::forward< Tn >( vn )... );
}

template<typename T>
inline T Abs(const T &x)
{
  if(x < static_cast<T>(0))
  {
    return x * static_cast<T>(-1);
  }
  return x;
}

template < typename T >
inline T Clip( T x, T min, T max )
{
  return Min( Max( x, min ), max );
}

inline float Clip01( float x )
{
  return Clip( x, 0.0f, 1.0f );
}

inline float DegToRad( float degrees )
{
  return degrees * PI / 180.0f;
}

inline float RadToDeg( float radians )
{
  return radians * 180.0f / PI;
}

inline int32_t Ceil( float f )
{
  bool positive = f >= 0.0f;
  if(positive)
  {
    int i = static_cast<int>(f);
    if( f > static_cast<float>(i) ) return i + 1;
    else return i;
  }
  else return static_cast<int>(f);
}

inline int32_t Floor( float f )
{
  bool negative = f < 0.0f;
  if(negative)
  {
    int i = static_cast<int>(f);
    if( f < static_cast<float>(i) ) return i - 1;
    else return i;
  }
  else return static_cast<int>(f);
}

inline int32_t Round( float f )
{
  if( f >= 0.0f ) return (int32_t)( f + 0.5f );
  else return (int32_t)( f - 0.5f );
}

inline uint32_t Mod( uint32_t i, uint32_t n )
{
  return i % n;
}

inline int Mod( int32_t i, int32_t n )
{
  if( i < 0 )
  {
    return ( ( i % n ) + n ) % n;
  }
  else
  {
    return i % n;
  }
}

inline float Mod( float f, float n )
{
  return fmodf( fmodf( f, n ) + n, n );
}

inline float Pow( float x, float e )
{
  return powf( x, e );
}

inline float Cos( float x )
{
  return cosf( x );
}

inline float Sin( float x )
{
  return sinf( x );
}

inline float Atan2( float y, float x )
{
  return atan2( y, x );
}

template< typename T >
constexpr T MaxValue()
{
  return std::numeric_limits< T >::max();
}

template< typename T >
constexpr T MinValue()
{
  return std::numeric_limits< T >::min();
}

template<>
constexpr float MaxValue< float >()
{
  return std::numeric_limits< float >::infinity();
}

template<>
constexpr float MinValue< float >()
{
  return -1 * std::numeric_limits< float >::infinity();
}

template<>
constexpr double MaxValue< double >()
{
  return std::numeric_limits< double >::infinity();
}

template<>
constexpr double MinValue< double >()
{
  return -1 * std::numeric_limits< double >::infinity();
}

template< typename T >
T Lerp( T start, T end, float t )
{
  return start + ( end - start ) * t;
}

inline float Delerp( float start, float end, float value )
{
  return ( value - start ) / ( end - start );
}

inline float Delerp01( float start, float end, float value )
{
  return Clip01( ( value - start ) / ( end - start ) );
}

template< typename T >
T DtLerp( T value, float snappiness, float dt, T target )
{
  return ae::Lerp( target, value, exp2( -exp2( snappiness ) * dt ) );
}

inline float DtLerpAngle( float value, float snappiness, float dt, float target )
{
  target = ae::Mod( target, ae::TWO_PI );
  float innerDist = ae::Abs( target - value );
  float preDist = ae::Abs( ( target - ae::TWO_PI ) - value );
  float postDist = ae::Abs( ( target + ae::TWO_PI ) - value );
  if ( innerDist >= preDist || innerDist >= postDist )
  {
    if ( preDist < postDist )
    {
      target -= ae::TWO_PI;
    }
    else
    {
      target += ae::TWO_PI;
    }
  }
  value = ae::DtLerp( value, snappiness, dt, target );
  return ae::Mod( value, ae::TWO_PI );
}

template< typename T >
T CosineInterpolate( T start, T end, float t )
{
  float angle = ( t * PI ) + PI;
  t = cosf(angle);
  t = ( t + 1 ) / 2.0f;
  return start + ( ( end - start ) * t );
}

namespace Interpolation
{
  template< typename T >
  T Linear( T start, T end, float t )
  {
    return start + ( ( end - start ) * t );
  }

  template< typename T >
  T Cosine( T start, T end, float t )
  {
    float angle = ( t * ae::PI );// + ae::PI;
    t = ( 1.0f - ae::Cos( angle ) ) / 2;
    // @TODO: Needed for Color, support types without lerp
    return start.Lerp( end, t ); //return start + ( ( end - start ) * t );
  }
}

inline int32_t Random( int32_t min, int32_t max )
{
  if ( min >= max )
  {
    return min;
  }
  return min + ( rand() % ( max - min ) );
}

inline float Random( float min, float max )
{
  if ( min >= max )
  {
    return min;
  }
  return min + ( ( rand() / (float)RAND_MAX ) * ( max - min ) );
}

inline bool RandomBool()
{
  return Random( 0, 2 );
}

//------------------------------------------------------------------------------
// RandomValue member functions
//------------------------------------------------------------------------------
template < typename T >
inline ae::RandomValue< T >::RandomValue( T min, T max ) : m_min(min), m_max(max) {}

template < typename T >
inline ae::RandomValue< T >::RandomValue( T value ) : m_min(value), m_max(value) {}

template < typename T >
inline void ae::RandomValue< T >::SetMin( T min )
{
  m_min = min;
}

template < typename T >
inline void ae::RandomValue< T >::SetMax( T max )
{
  m_max = max;
}

template < typename T >
inline T ae::RandomValue< T >::GetMin() const
{
  return m_min;
}

template < typename T >
inline T ae::RandomValue< T >::GetMax() const
{
  return m_max;
}

template < typename T >
inline T ae::RandomValue< T >::Get() const
{
  return Random( m_min, m_max );
}

template < typename T >
inline ae::RandomValue< T >::operator T() const
{
  return Get();
}

//------------------------------------------------------------------------------
// ae::Vec2 shared member functions
// ae::Vec3 shared member functions
// ae::Vec4 shared member functions
//------------------------------------------------------------------------------
template < typename T >
bool VecT< T >::operator==( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) == 0;
}

template < typename T >
bool VecT< T >::operator!=( const T& v ) const
{
  auto&& self = *(T*)this;
  return memcmp( self.data, v.data, sizeof(T::data) ) != 0;
}

template < typename T >
float VecT< T >::operator[]( uint32_t idx ) const
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
float& VecT< T >::operator[]( uint32_t idx )
{
  auto&& self = *(T*)this;
#if _AE_DEBUG_
  AE_ASSERT( idx < countof(self.data) );
#endif
  return self.data[ idx ];
}

template < typename T >
T VecT< T >::operator+( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    result.data[ i ] = self.data[ i ] + v.data[ i ];
  }
  return result;
}

template < typename T >
void VecT< T >::operator+=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(T::data); i++ )
  {
    self.data[ i ] += v.data[ i ];
  }
}

template < typename T >
T VecT< T >::operator-() const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = -self.data[ i ];
  }
  return result;
}

template < typename T >
T VecT< T >::operator-( const T& v ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] - v.data[ i ];
  }
  return result;
}

template < typename T >
void VecT< T >::operator-=( const T& v )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] -= v.data[ i ];
  }
}

template < typename T >
T VecT< T >::operator*( float s ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] * s;
  }
  return result;
}

template < typename T >
void VecT< T >::operator*=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] *= s;
  }
}

template < typename T >
T VecT< T >::operator/( float s ) const
{
  auto&& self = *(T*)this;
  T result;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    result.data[ i ] = self.data[ i ] / s;
  }
  return result;
}

template < typename T >
void VecT< T >::operator/=( float s )
{
  auto&& self = *(T*)this;
  for ( uint32_t i = 0; i < countof(self.data); i++ )
  {
    self.data[ i ] /= s;
  }
}

template < typename T >
float VecT< T >::Dot( const T& v0, const T& v1 )
{
  float result = 0.0f;
  for ( uint32_t i = 0; i < countof(v0.data); i++ )
  {
    result += v0.data[ i ] * v1.data[ i ];
  }
  return result;
}

template < typename T >
float VecT< T >::Dot( const T& v ) const
{
  return Dot( *(T*)this, v );
}

template < typename T >
float VecT< T >::Length() const
{
  return sqrt( LengthSquared() );
}

template < typename T >
float VecT< T >::LengthSquared() const
{
  return Dot( *(T*)this );
}

template < typename T >
float VecT< T >::Normalize()
{
  float length = Length();
  *(T*)this /= length;
  return length;
}

template < typename T >
float VecT< T >::SafeNormalize( float epsilon )
{
  auto&& self = *(T*)this;
  float length = Length();
  if ( length < epsilon )
  {
    self = T( 0.0f );
    return 0.0f;
  }
  self /= length;
  return length;
}

template < typename T >
T VecT< T >::NormalizeCopy() const
{
  T result = *(T*)this;
  result.Normalize();
  return result;
}

template < typename T >
T VecT< T >::SafeNormalizeCopy( float epsilon ) const
{
  T result = *(T*)this;
  result.SafeNormalize( epsilon );
  return result;
}

template < typename T >
float VecT< T >::Trim( float trimLength )
{
  float length = Length();
  if ( trimLength < length )
  {
    *(T*)this *= ( trimLength / length );
    return trimLength;
  }
  return length;
}

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const VecT< T >& v )
{
  constexpr uint32_t count = countof( T::data );
  for ( uint32_t i = 0; i < count - 1; i++ )
  {
    os << v[ i ] << " ";
  }
  return os << v[ count - 1 ];
}

#pragma warning(disable:26495) // Hide incorrect Vec2 initialization warning due to union

//------------------------------------------------------------------------------
// ae::Vec2 member functions
//------------------------------------------------------------------------------
inline Vec2::Vec2( float v ) : x( v ), y( v ) {}
inline Vec2::Vec2( float x, float y ) : x( x ), y( y ) {}
inline Vec2::Vec2( const float* v2 ) : x( v2[ 0 ] ), y( v2[ 1 ] ) {}
inline Vec2 Vec2::FromAngle( float angle ) { return Vec2( ae::Cos( angle ), ae::Sin( angle ) ); }

//------------------------------------------------------------------------------
// ae::Vec3 member functions
//------------------------------------------------------------------------------
inline Vec3::Vec3( float v ) : x( v ), y( v ), z( v ), pad( 0.0f ) {}
inline Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( const float* v3 ) : x( v3[ 0 ] ), y( v3[ 1 ] ), z( v3[ 2 ] ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy, float z ) : x( xy.x ), y( xy.y ), z( z ), pad( 0.0f ) {}
inline Vec3::Vec3( Vec2 xy ) : x( xy.x ), y( xy.y ), z( 0.0f ), pad( 0.0f ) {}
inline Vec3::operator Vec2() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec3::GetXZ() const { return Vec2( x, z ); }
inline Vec3 Vec3::Lerp( const Vec3& end, float t ) const
{
  t = ae::Clip01( t );
  float minT = ( 1.0f - t );
  return Vec3( x * minT + end.x * t, y * minT + end.y * t, z * minT + end.z * t );
}
inline Vec3 Vec3::Cross( const Vec3& v0, const Vec3& v1 )
{
  return Vec3( v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x );
}
inline Vec3 Vec3::Cross( const Vec3& v ) const { return Cross( *this, v ); }
inline void Vec3::ZeroAxis( Vec3 axis )
{
  axis.SafeNormalize();
  *this -= axis * Dot( axis );
}
inline void Vec3::ZeroDirection( Vec3 direction )
{
  float d = Dot( direction );
  if ( d > 0.0f )
  {
    direction.SafeNormalize();
    *this -= direction * d;
  }
}

//------------------------------------------------------------------------------
// ae::Vec4 member functions
//------------------------------------------------------------------------------
inline Vec4::Vec4( float f ) : x( f ), y( f ), z( f ), w( f ) {}
inline Vec4::Vec4( float xyz, float w ) : x( xyz ), y( xyz ), z( xyz ), w( w ) {}
inline Vec4::Vec4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
inline Vec4::Vec4( Vec3 xyz, float w ) : x( xyz.x ), y( xyz.y ), z( xyz.z ), w( w ) {}
inline Vec4::Vec4( Vec2 xy, float z, float w ) : x( xy.x ), y( xy.y ), z( z ), w( w ) {}
inline Vec4::Vec4( Vec2 xy, Vec2 zw ) : x( xy.x ), y( xy.y ), z( zw.x ), w( zw.y ) {}
inline Vec4::operator Vec2() const { return Vec2( x, y ); }
inline Vec4::operator Vec3() const { return Vec3( x, y, z ); }
inline Vec4::Vec4( const float* v3, float w ) : x( v3[ 0 ] ), y( v3[ 1 ] ), z( v3[ 2 ] ), w( w ) {}
inline Vec4::Vec4( const float* v4 ) : x( v4[ 0 ] ), y( v4[ 1 ] ), z( v4[ 2 ] ), w( v4[ 3 ] ) {}
inline Vec2 Vec4::GetXY() const { return Vec2( x, y ); }
inline Vec2 Vec4::GetZW() const { return Vec2( z, w ); }
inline Vec3 Vec4::GetXYZ() const { return Vec3( x, y, z ); }

//------------------------------------------------------------------------------
// ae::Colors
// It's expensive to do the srgb conversion everytime these are constructed so
// do it once and then return a copy each time static Color functions are called.
//------------------------------------------------------------------------------
// Grayscale
inline Color Color::White() { static Color c = Color::SRGB8( 255, 255, 255 ); return c; }
inline Color Color::Gray() { static Color c = Color::SRGB8( 127, 127, 127 ); return c; }
inline Color Color::Black() { static Color c = Color::SRGB8( 0, 0, 0 ); return c; }
// Rainbow
inline Color Color::Red() { static Color c = Color::SRGB8( 255, 0, 0 ); return c; }
inline Color Color::Orange() { static Color c = Color::SRGB8( 255, 127, 0 ); return c; }
inline Color Color::Yellow() { static Color c = Color::SRGB8( 255, 255, 0 ); return c; }
inline Color Color::Green() { static Color c = Color::SRGB8( 0, 255, 0 ); return c; }
inline Color Color::Blue() { static Color c = Color::SRGB8( 0, 0, 255 ); return c; }
inline Color Color::Indigo() { static Color c = Color::SRGB8( 75, 0, 130 ); return c; }
inline Color Color::Violet() { static Color c = Color::SRGB8( 148, 0, 211 ); return c; }
// Pico
inline Color Color::PicoBlack() { static Color c = Color::SRGB8( 0, 0, 0 ); return c; }
inline Color Color::PicoDarkBlue() { static Color c = Color::SRGB8( 29, 43, 83 ); return c; }
inline Color Color::PicoDarkPurple() { static Color c = Color::SRGB8( 126, 37, 83 ); return c; }
inline Color Color::PicoDarkGreen() { static Color c = Color::SRGB8( 0, 135, 81 ); return c; }
inline Color Color::PicoBrown() { static Color c = Color::SRGB8( 171, 82, 54 ); return c; }
inline Color Color::PicoDarkGray() { static Color c = Color::SRGB8( 95, 87, 79 ); return c; }
inline Color Color::PicoLightGray() { static Color c = Color::SRGB8( 194, 195, 199 ); return c; }
inline Color Color::PicoWhite() { static Color c = Color::SRGB8( 255, 241, 232 ); return c; }
inline Color Color::PicoRed() { static Color c = Color::SRGB8( 255, 0, 77 ); return c; }
inline Color Color::PicoOrange() { static Color c = Color::SRGB8( 255, 163, 0 ); return c; }
inline Color Color::PicoYellow() { static Color c = Color::SRGB8( 255, 236, 39 ); return c; }
inline Color Color::PicoGreen() { static Color c = Color::SRGB8( 0, 228, 54 ); return c; }
inline Color Color::PicoBlue() { static Color c = Color::SRGB8( 41, 173, 255 ); return c; }
inline Color Color::PicoIndigo() { static Color c = Color::SRGB8( 131, 118, 156 ); return c; }
inline Color Color::PicoPink() { static Color c = Color::SRGB8( 255, 119, 168 ); return c; }
inline Color Color::PicoPeach() { static Color c = Color::SRGB8( 255, 204, 170 ); return c; }
// Misc
inline Color Color::Magenta() { return Color( 1.0f, 0.0f, 1.0f ); }

//------------------------------------------------------------------------------
// ae::Color functions
//------------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& os, Color c )
{
  return os << "<" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ">";
}
inline Color::Color( float rgb ) : r( rgb ), g( rgb ), b( rgb ), a( 1.0f ) {}
inline Color::Color( float r, float g, float b ) : r( r ), g( g ), b( b ), a( 1.0f ) {}
inline Color::Color( float r, float g, float b, float a )
  : r( r ), g( g ), b( b ), a( a )
{}
inline Color::Color( Color c, float a ) : r( c.r ), g( c.g ), b( c.b ), a( a ) {}
inline Color Color::R( float r ) { return Color( r, 0.0f, 0.0f, 1.0f ); }
inline Color Color::RG( float r, float g ) { return Color( r, g, 0.0f, 1.0f ); }
inline Color Color::RGB( float r, float g, float b ) { return Color( r, g, b, 1.0f ); }
inline Color Color::RGBA( float r, float g, float b, float a ) { return Color( r, g, b, a ); }
inline Color Color::RGBA( const float* v ) { return Color( v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ] ); }
inline Color Color::SRGB( float r, float g, float b ) { return Color( SRGBToRGB( r ), SRGBToRGB( g ), SRGBToRGB( b ), 1.0f ); }
inline Color Color::SRGBA( float r, float g, float b, float a ) { return Color( SRGBToRGB( r ), SRGBToRGB( g ), SRGBToRGB( b ), a ); }
inline Color Color::R8( uint8_t r ) { return Color( r / 255.0f, 0.0f, 0.0f, 1.0f ); }
inline Color Color::RG8( uint8_t r, uint8_t g ) { return Color( r / 255.0f, g / 255.0f, 0.0f, 1.0f ); }
inline Color Color::RGB8( uint8_t r, uint8_t g, uint8_t b ) { return Color( r / 255.0f, g / 255.0f, b / 255.0f, 1.0f ); }
inline Color Color::RGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return Color( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
}
inline Color Color::SRGB8( uint8_t r, uint8_t g, uint8_t b )
{
  return Color( SRGBToRGB( r / 255.0f ), SRGBToRGB( g / 255.0f ), SRGBToRGB( b / 255.0f ), 1.0f );
}
inline Color Color::SRGBA8( uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
  return Color( SRGBToRGB( r / 255.0f ), SRGBToRGB( g / 255.0f ), SRGBToRGB( b / 255.0f ), a / 255.0f );
}
inline Vec3 Color::GetLinearRGB() const { return Vec3( r, g, b ); }
inline Vec4 Color::GetLinearRGBA() const { return Vec4( r, g, b, a ); }
inline Vec3 Color::GetSRGB() const { return Vec3( RGBToSRGB( r ), RGBToSRGB( g ), RGBToSRGB( b ) ); }
inline Vec4 Color::GetSRGBA() const { return Vec4( GetSRGB(), a ); }
inline Color Color::Lerp( const Color& end, float t ) const
{
  return Color(
    ae::Lerp( r, end.r, t ),
    ae::Lerp( g, end.g, t ),
    ae::Lerp( b, end.b, t ),
    ae::Lerp( a, end.a, t )
  );
}
inline Color Color::DtLerp( float snappiness, float dt, const Color& target ) const
{
  return Lerp( target, exp2( -exp2( snappiness ) * dt ) );
}
inline Color Color::ScaleRGB( float s ) const { return Color( r * s, g * s, b * s, a ); }
inline Color Color::ScaleA( float s ) const { return Color( r, g, b, a * s ); }
inline Color Color::SetA( float alpha ) const { return Color( r, g, b, alpha ); }
inline float Color::SRGBToRGB( float x ) { return pow( x , 2.2 ); }
inline float Color::RGBToSRGB( float x ) { return pow( x, 1.0 / 2.2 ); }

#pragma warning(default:26495) // Re-enable uninitialized variable warning

//------------------------------------------------------------------------------
// ae::Str functions
//------------------------------------------------------------------------------
template < uint32_t N >
std::ostream& operator<<( std::ostream& out, const Str< N >& str )
{
  return out << str.c_str();
}

template < uint32_t N >
std::istream& operator>>( std::istream& in, Str< N >& str )
{
  in.getline( str.m_str, Str< N >::MaxLength() );
  str.m_length = in.gcount();
  str.m_str[ str.m_length ] = 0;
  return in;
}

template < uint32_t N >
const Str< N >& ToString( const Str< N >& value )
{
  return value;
}

inline const char* ToString( const char* value )
{
  return value;
}

inline Str16 ToString( int32_t value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%d", value );
  return Str16( length, str );
}

inline Str16 ToString( uint32_t value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%u", value );
  return Str16( length, str );
}

inline Str16 ToString( float value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return Str16( length, str );
}

inline Str16 ToString( double value )
{
  char str[ Str16::MaxLength() + 1u ];
  uint32_t length = snprintf( str, sizeof( str ) - 1, "%.2f", value );
  return Str16( length, str );
}

template < typename T >
Str64 ToString( const T& v )
{
  std::stringstream os;
  os << v;
  return os.str().c_str();
}

template < uint32_t N >
Str< N >::Str()
{
  AE_STATIC_ASSERT_MSG( sizeof( *this ) == N, "Incorrect Str size" );
  m_length = 0;
  m_str[ 0 ] = 0;
}

template < uint32_t N >
template < uint32_t N2 >
Str< N >::Str( const Str<N2>& str )
{
  AE_ASSERT( str.m_length <= (uint16_t)MaxLength() );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, m_length + 1u );
}

template < uint32_t N >
Str< N >::Str( const char* str )
{
  m_length = (uint16_t)strlen( str );
  AE_ASSERT_MSG( m_length <= (uint16_t)MaxLength(), "Length:# Max:#", m_length, MaxLength() );
  memcpy( m_str, str, m_length + 1u );
}

template < uint32_t N >
Str< N >::Str( uint32_t length, const char* str )
{
  AE_ASSERT( length <= (uint16_t)MaxLength() );
  m_length = length;
  memcpy( m_str, str, m_length );
  m_str[ length ] = 0;
}

template < uint32_t N >
Str< N >::Str( uint32_t length, char c )
{
  AE_ASSERT( length <= (uint16_t)MaxLength() );
  m_length = length;
  memset( m_str, c, m_length );
  m_str[ length ] = 0;
}

template < uint32_t N >
template < typename... Args >
Str< N >::Str( const char* format, Args... args )
{
  m_length = 0;
  m_str[ 0 ] = 0;
  m_Format( format, args... );
}

template < uint32_t N >
Str< N >::operator const char*() const
{
  return m_str;
}

template < uint32_t N >
const char* Str< N >::c_str() const
{
  return m_str;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::operator =( const Str<N2>& str )
{
  AE_ASSERT( str.m_length <= (uint16_t)MaxLength() );
  m_length = str.m_length;
  memcpy( m_str, str.m_str, str.m_length + 1u );
}

template < uint32_t N >
Str<N> Str< N >::operator +( const char* str ) const
{
  Str<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
template < uint32_t N2 >
Str<N> Str< N >::operator +( const Str<N2>& str ) const
{
  Str<N> out( *this );
  out += str;
  return out;
}

template < uint32_t N >
void Str< N >::operator +=( const char* str )
{
  uint32_t len = (uint32_t)strlen( str );
  AE_ASSERT( m_length + len <= (uint16_t)MaxLength() );
  memcpy( m_str + m_length, str, len + 1u );
  m_length += len;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::operator +=( const Str<N2>& str )
{
  AE_ASSERT( m_length + str.m_length <= (uint16_t)MaxLength() );
  memcpy( m_str + m_length, str.c_str(), str.m_length + 1u );
  m_length += str.m_length;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator ==( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) == 0;
}

template < uint32_t N >
bool Str< N >::operator ==( const char* str ) const
{
  return strcmp( m_str, str ) == 0;
}

template < uint32_t N >
bool operator ==( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) == 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator !=( const Str<N2>& str ) const
{
  return !operator==( str );
}

template < uint32_t N >
bool Str< N >::operator !=( const char* str ) const
{
  return !operator==( str );
}

template < uint32_t N >
bool operator !=( const char* str0, const Str<N>& str1 )
{
  return !operator==( str0, str1 );
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator <( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) < 0;
}

template < uint32_t N >
bool Str< N >::operator <( const char* str ) const
{
  return strcmp( m_str, str ) < 0;
}

template < uint32_t N >
bool operator <( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) < 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator >( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) > 0;
}

template < uint32_t N >
bool Str< N >::operator >( const char* str ) const
{
  return strcmp( m_str, str ) > 0;
}

template < uint32_t N >
bool operator >( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) > 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator <=( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) <= 0;
}

template < uint32_t N >
bool Str< N >::operator <=( const char* str ) const
{
  return strcmp( m_str, str ) <= 0;
}

template < uint32_t N >
bool operator <=( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) <= 0;
}

template < uint32_t N >
template < uint32_t N2 >
bool Str< N >::operator >=( const Str<N2>& str ) const
{
  return strcmp( m_str, str.c_str() ) >= 0;
}

template < uint32_t N >
bool Str< N >::operator >=( const char* str ) const
{
  return strcmp( m_str, str ) >= 0;
}

template < uint32_t N >
bool operator >=( const char* str0, const Str<N>& str1 )
{
  return strcmp( str0, str1.m_str ) >= 0;
}

template < uint32_t N >
char& Str< N >::operator[]( uint32_t i )
{
  AE_ASSERT( i <= m_length ); return m_str[ i ]; // @NOTE: Allow indexing null, one past length
}

template < uint32_t N >
const char Str< N >::operator[]( uint32_t i ) const
{
  AE_ASSERT( i <= m_length ); return m_str[ i ]; // @NOTE: Allow indexing null, one past length
}

template < uint32_t N >
uint32_t Str< N >::Length() const
{
  return m_length;
}

template < uint32_t N >
uint32_t Str< N >::Size() const
{
  return MaxLength();
}

template < uint32_t N >
bool Str< N >::Empty() const
{
  return m_length == 0;
}

template < uint32_t N >
template < uint32_t N2 >
void Str< N >::Append( const Str<N2>& str )
{
  *this += str;
}

template < uint32_t N >
void Str< N >::Append( const char* str )
{
  *this += str;
}

template < uint32_t N >
void Str< N >::Trim( uint32_t len )
{
  if ( len == m_length )
  {
    return;
  }

  AE_ASSERT( len < m_length );
  m_length = len;
  m_str[ m_length ] = 0;
}

template < uint32_t N >
template < typename... Args >
Str< N > Str< N >::Format( const char* format, Args... args )
{
  Str< N > result( "" );
  result.m_Format( format, args... );
  return result;
}

template < uint32_t N >
void Str< N >::m_Format( const char* format )
{
  *this += format;
}

template < uint32_t N >
template < typename T, typename... Args >
void Str< N >::m_Format( const char* format, T value, Args... args )
{
  if ( !*format )
  {
    return;
  }

  const char* head = format;
  while ( *head && *head != '#' )
  {
    head++;
  }
  if ( head > format )
  {
    *this += Str< N >( head - format, format );
  }

  if ( *head == '#' )
  {
    // @TODO: Replace with ToString()?
    std::ostringstream stream;
    stream << value;
    *this += stream.str().c_str();
    head++;
  }
  m_Format( head, args... );
}

//------------------------------------------------------------------------------
// Array functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
inline std::ostream& operator<<( std::ostream& os, const Array< T, N >& array )
{
  os << "<";
  for ( uint32_t i = 0; i < array.Length(); i++ )
  {
    os << array[ i ];
    if ( i != array.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << ">";
}

template < typename T, uint32_t N >
Array< T, N >::Array()
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = 0;
  m_size = N;
  m_array = (T*)&m_storage;
}

template < typename T, uint32_t N >
Array< T, N >::Array( uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static arrays" );
  
  m_length = length;
  m_size = N;
  m_array = (T*)&m_storage;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  AE_ASSERT( tag != ae::Tag() );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t size )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( size );
}

template < typename T, uint32_t N >
Array< T, N >::Array( ae::Tag tag, uint32_t length, const T& value )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static arrays" );
  
  m_length = 0;
  m_size = 0;
  m_array = nullptr;
  m_tag = tag;

  Reserve( length );

  m_length = length;
  for ( uint32_t i = 0; i < length; i++ )
  {
    new ( &m_array[ i ] ) T ( value );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( const Array< T, N >& other )
{
  m_length = 0;
  m_size = N;
  m_array = N ? (T*)&m_storage : nullptr;
  m_tag = other.m_tag;
  
  // Array must be initialized above before calling Reserve
  Reserve( other.m_length );

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T, uint32_t N >
Array< T, N >::Array( Array< T, N >&& other ) noexcept
{
  m_tag = other.m_tag;
  if ( N )
  {
    m_length = 0;
    m_size = N;
    m_array = N ? (T*)&m_storage : nullptr;
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
    // @NOTE: Don't reset tag. 'other' must remain in a valid state.
  }
}

template < typename T, uint32_t N >
Array< T, N >::~Array()
{
  Clear();
  
  if ( N == 0 )
  {
    ae::Free( m_array );
  }
  m_size = 0;
  m_array = nullptr;
}

template < typename T, uint32_t N >
void Array< T, N >::operator =( const Array< T, N >& other )
{
  if ( m_array == other.m_array )
  {
    return;
  }
  
  Clear();
  
  if ( m_size < other.m_length )
  {
    Reserve( other.m_length );
  }

  m_length = other.m_length;
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &m_array[ i ] ) T ( other.m_array[ i ] );
  }
}

template < typename T, uint32_t N >
void Array< T, N >::operator =( Array< T, N >&& other ) noexcept
{
  if ( N || m_tag != other.m_tag )
  {
    *this = other; // Regular assignment (without std::move)
  }
  else
  {
    if ( m_array )
    {
      Clear();
      ae::Free( m_array );
    }
    
    m_length = other.m_length;
    m_size = other.m_size;
    m_array = other.m_array;
    
    other.m_length = 0;
    other.m_size = 0;
    other.m_array = nullptr;
  }
}

template < typename T, uint32_t N >
T& Array< T, N >::Append( const T& value )
{
  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  new ( &m_array[ m_length ] ) T ( value );
  m_length++;

  return m_array[ m_length - 1 ];
}

template < typename T, uint32_t N >
void Array< T, N >::Append( const T* values, uint32_t count )
{
  Reserve( m_length + count );

#if _AE_DEBUG_
  AE_ASSERT( m_size >= m_length + count );
#endif
  for ( uint32_t i = 0; i < count; i++ )
  {
    new ( &m_array[ m_length ] ) T ( values[ i ] );
    m_length++;
  }
}

template < typename T, uint32_t N >
T& Array< T, N >::Insert( uint32_t index, const T& value )
{
#if _AE_DEBUG_
  AE_ASSERT( index <= m_length );
#endif

  if ( m_length == m_size )
  {
    Reserve( m_GetNextSize() );
  }

  if ( index == m_length )
  {
    new ( &m_array[ index ] ) T ( value );
  }
  else
  {
    new ( &m_array[ m_length ] ) T ( std::move( m_array[ m_length - 1 ] ) );
    for ( int32_t i = m_length - 1; i > index; i-- )
    {
      m_array[ i ] = std::move( m_array[ i - 1 ] );
    }
    m_array[ index ] = value;
  }
  
  m_length++;

  return m_array[ index ];
}

template < typename T, uint32_t N >
void Array< T, N >::Remove( uint32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index < m_length );
#endif

  m_length--;
  for ( uint32_t i = index; i < m_length; i++ )
  {
    m_array[ i ] = std::move( m_array[ i + 1 ] );
  }
  m_array[ m_length ].~T();
}

template < typename T, uint32_t N >
template < typename U >
uint32_t Array< T, N >::RemoveAll( const U& value )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = Find( value ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T, uint32_t N >
template < typename Fn >
uint32_t Array< T, N >::RemoveAllFn( Fn testFn )
{
  uint32_t count = 0;
  int32_t index = 0;
  while ( ( index = FindFn( testFn ) ) >= 0 )
  {
    // @TODO: Update this to be single loop, so array is only compacted once
    Remove( index );
    count++;
  }
  return count;
}

template < typename T, uint32_t N >
template < typename U >
int32_t Array< T, N >::Find( const U& value ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( m_array[ i ] == value )
    {
      return i;
    }
  }
  return -1;
}

template < typename T, uint32_t N >
template < typename Fn >
int32_t Array< T, N >::FindFn( Fn testFn ) const
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    if ( testFn( m_array[ i ] ) )
    {
      return i;
    }
  }
  return -1;
}

template < typename T, uint32_t N >
void Array< T, N >::Reserve( uint32_t size )
{
  if ( N > 0 )
  {
    AE_ASSERT( N >= size );
    return;
  }
  else if ( size <= m_size )
  {
    return;
  }
  
#if _AE_DEBUG_
  AE_ASSERT( m_tag != ae::Tag() );
#endif
  
  // Next power of two
  size--;
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  size++;
  
#if _AE_DEBUG_
  AE_ASSERT( size );
#endif
  m_size = size;
  
  T* arr = (T*)ae::Allocate( m_tag, m_size * sizeof(T), alignof(T) );
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    new ( &arr[ i ] ) T ( std::move( m_array[ i ] ) );
    m_array[ i ].~T();
  }
  
  ae::Free( m_array );
  m_array = arr;
}

template < typename T, uint32_t N >
void Array< T, N >::Clear()
{
  for ( uint32_t i = 0; i < m_length; i++ )
  {
    m_array[ i ].~T();
  }
  m_length = 0;
}

template < typename T, uint32_t N >
const T& Array< T, N >::operator[]( int32_t index ) const
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT( index < (int32_t)m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
T& Array< T, N >::operator[]( int32_t index )
{
#if _AE_DEBUG_
  AE_ASSERT( index >= 0 );
  AE_ASSERT_MSG( index < (int32_t)m_length, "index: # length: #", index, m_length );
#endif
  return m_array[ index ];
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Length() const
{
  return m_length;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::Size() const
{
  return m_size;
}

template < typename T, uint32_t N >
uint32_t Array< T, N >::m_GetNextSize() const
{
  if ( m_size == 0 )
  {
    return ae::Max( 1, 32 / sizeof(T) ); // @NOTE: Initially allocate 32 bytes (rounded down) of type
  }
  else
  {
    return m_size * 2;
  }
}

//------------------------------------------------------------------------------
// Map functions
//------------------------------------------------------------------------------
template < typename K >
bool Map_IsEqual( const K& k0, const K& k1 );

template <>
inline bool Map_IsEqual( const char* const & k0, const char* const & k1 )
{
  return strcmp( k0, k1 ) == 0;
}

template < typename K >
bool Map_IsEqual( const K& k0, const K& k1 )
{
  return k0 == k1;
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Map()
{
  AE_STATIC_ASSERT_MSG( N != 0, "Must provide allocator for non-static maps" );
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Map( ae::Tag pool ) :
  m_entries( pool )
{
  AE_STATIC_ASSERT_MSG( N == 0, "Do not provide allocator for static maps" );
}

template < typename K, typename V, uint32_t N >
Map< K, V, N >::Entry::Entry( const K& k, const V& v ) :
  key( k ),
  value( v )
{}

template < typename K, typename V, uint32_t N >
int32_t Map< K, V, N >::m_FindIndex( const K& key ) const
{
  for ( uint32_t i = 0; i < m_entries.Length(); i++ )
  {
    if ( Map_IsEqual( m_entries[ i ].key, key ) )
    {
      return i;
    }
  }

  return -1;
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::Set( const K& key, const V& value )
{
  int32_t index = m_FindIndex( key );
  Entry* entry = ( index >= 0 ) ? &m_entries[ index ] : nullptr;
  if ( entry )
  {
    entry->value = value;
    return entry->value;
  }
  else
  {
    return m_entries.Append( Entry( key, value ) ).value;
  }
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::Get( const K& key )
{
  return m_entries[ m_FindIndex( key ) ].value;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::Get( const K& key ) const
{
  return m_entries[ m_FindIndex( key ) ].value;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::Get( const K& key, const V& defaultValue ) const
{
  int32_t index = m_FindIndex( key );
  return ( index >= 0 ) ? m_entries[ index ].value : defaultValue;
}

template < typename K, typename V, uint32_t N >
V* Map< K, V, N >::TryGet( const K& key )
{
  return const_cast< V* >( const_cast< const Map< K, V, N >* >( this )->TryGet( key ) );
}

template < typename K, typename V, uint32_t N >
const V* Map< K, V, N >::TryGet( const K& key ) const
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    return &m_entries[ index ].value;
  }
  else
  {
    return nullptr;
  }
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::TryGet( const K& key, V* valueOut )
{
  return const_cast< const Map< K, V, N >* >( this )->TryGet( key, valueOut );
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::TryGet( const K& key, V* valueOut ) const
{
  const V* val = TryGet( key );
  if ( val )
  {
    if ( valueOut )
    {
      *valueOut = *val;
    }
    return true;
  }
  return false;
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::Remove( const K& key )
{
  return Remove( key, nullptr );
}

template < typename K, typename V, uint32_t N >
bool Map< K, V, N >::Remove( const K& key, V* valueOut )
{
  int32_t index = m_FindIndex( key );
  if ( index >= 0 )
  {
    if ( valueOut )
    {
      *valueOut = m_entries[ index ].value;
    }
    m_entries.Remove( index );
    return true;
  }
  else
  {
    return false;
  }
}

template < typename K, typename V, uint32_t N >
void Map< K, V, N >::Reserve( uint32_t total )
{
  m_entries.Reserve( total );
}

template < typename K, typename V, uint32_t N >
void Map< K, V, N >::Clear()
{
  m_entries.Clear();
}

template < typename K, typename V, uint32_t N >
K& Map< K, V, N >::GetKey( uint32_t index )
{
  return m_entries[ index ].key;
}

template < typename K, typename V, uint32_t N >
V& Map< K, V, N >::GetValue( uint32_t index )
{
  return m_entries[ index ].value;
}

template < typename K, typename V, uint32_t N >
const K& Map< K, V, N >::GetKey( uint32_t index ) const
{
  return m_entries[ index ].key;
}

template < typename K, typename V, uint32_t N >
const V& Map< K, V, N >::GetValue( uint32_t index ) const
{
  return m_entries[ index ].value;
}

template < typename K, typename V, uint32_t N >
uint32_t Map< K, V, N >::Length() const
{
  return m_entries.Length();
}

template < typename K, typename V, uint32_t N >
std::ostream& operator<<( std::ostream& os, const Map< K, V, N >& map )
{
  os << "{";
  for ( uint32_t i = 0; i < map.m_entries.Length(); i++ )
  {
    os << "(" << map.m_entries[ i ].key << ", " << map.m_entries[ i ].value << ")";
    if ( i != map.m_entries.Length() - 1 )
    {
      os << ", ";
    }
  }
  return os << "}";
}

} // AE_NAMESPACE end

//------------------------------------------------------------------------------
// The following should be compiled into a single module and linked with the
// application. It's worth putting this in it's own module to limit the
// number of dependencies brought into your own code. For instance 'Windows.h'
// is included and this can easily cause naming conflicts with gameplay/engine
// code.
// Usage inside a cpp/mm file is:
//
// // ae.cpp/mm EXAMPLE START
//
// #define AE_MAIN
// #include "aether.h"
//
// // ae.cpp/mm EXAMPLE END
//------------------------------------------------------------------------------
#ifdef AE_MAIN

//------------------------------------------------------------------------------
// Platform includes, required for logging, windowing, file io
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
  #define WIN32_LEAN_AND_MEAN
  #include "Windows.h"
  #include "processthreadsapi.h" // For GetCurrentProcessId()
#elif _AE_APPLE_
  #include <sys/sysctl.h>
  #include <unistd.h>
#else
  #include <unistd.h>
#endif
#include <thread>

//------------------------------------------------------------------------------
// Platform functions internal implementation
//------------------------------------------------------------------------------
namespace AE_NAMESPACE {

uint32_t GetPID()
{
#if _AE_WINDOWS_
  return GetCurrentProcessId();
#else
  return getpid();
#endif
}

uint32_t GetMaxConcurrentThreads()
{
  return std::thread::hardware_concurrency();
}

#if _AE_APPLE_
bool IsDebuggerAttached()
{
  struct kinfo_proc info;
  info.kp_proc.p_flag = 0;

  // Initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process ID.
  int mib[ 4 ];
  mib[ 0 ] = CTL_KERN;
  mib[ 1 ] = KERN_PROC;
  mib[ 2 ] = KERN_PROC_PID;
  mib[ 3 ] = getpid();

  // Call sysctl
  size_t size = sizeof( info );
  int result = sysctl( mib, sizeof( mib ) / sizeof( *mib ), &info, &size, NULL, 0 );
  AE_ASSERT( result == 0 );

  // Application is being debugged if the P_TRACED flag is set
  return ( ( info.kp_proc.p_flag & P_TRACED ) != 0 );
}
#elif _AE_WINDOWS_
bool IsDebuggerAttached()
{
  return IsDebuggerPresent();
}
#else
bool IsDebuggerAttached()
{
  return false;
}
#endif

double GetTime()
{
#if _AE_WINDOWS_
  static LARGE_INTEGER counterFrequency = { 0 };
  if ( !counterFrequency.QuadPart )
  {
    bool success = QueryPerformanceFrequency( &counterFrequency ) != 0;
    AE_ASSERT( success );
  }

  LARGE_INTEGER performanceCount = { 0 };
  bool success = QueryPerformanceCounter( &performanceCount ) != 0;
  AE_ASSERT( success );
  return performanceCount.QuadPart / (double)counterFrequency.QuadPart;
#else
  return std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count() / 1000000.0;
#endif
}

//------------------------------------------------------------------------------
// ae::Vec3 functions
//------------------------------------------------------------------------------
float Vec3::GetAngleBetween( const Vec3& v, float epsilon ) const
{
  const Vec3 crossProduct = Cross( v );
  const float dotProduct = Dot( v );
  if ( crossProduct.LengthSquared() < epsilon && dotProduct > 0.0f )
  {
    return 0.0f;
  }
  else if ( crossProduct.LengthSquared() < epsilon && dotProduct < 0.0f )
  {
    return ae::PI;
  }
  float angle = dotProduct;
  angle /= Length() * v.Length();
  angle = std::acos( angle );
  angle = std::abs( angle );
  return angle;
}

void Vec3::AddRotationXY( float rotation )
{
  float sinTheta = std::sin( rotation );
  float cosTheta = std::cos( rotation );
  float newX = x * cosTheta - y * sinTheta;
  float newY = x * sinTheta + y * cosTheta;
  x = newX;
  y = newY;
}

Vec3 Vec3::RotateCopy( Vec3 axis, float angle ) const
{
  // http://stackoverflow.com/questions/6721544/circular-rotation-around-an-arbitrary-axis
  axis.Normalize();
  float cosA = cosf( angle );
  float mCosA = 1.0f - cosA;
  float sinA = sinf( angle );
  Vec3 r0(
    cosA + axis.x * axis.x * mCosA,
    axis.x * axis.y * mCosA - axis.z * sinA,
    axis.x * axis.z * mCosA + axis.y * sinA );
  Vec3 r1(
    axis.y * axis.x * mCosA + axis.z * sinA,
    cosA + axis.y * axis.y * mCosA,
    axis.y * axis.z * mCosA - axis.x * sinA );
  Vec3 r2(
    axis.z * axis.x * mCosA - axis.y * sinA,
    axis.z * axis.y * mCosA + axis.x * sinA,
    cosA + axis.z * axis.z * mCosA );
  return Vec3( r0.Dot( *this ), r1.Dot( *this ), r2.Dot( *this ) );
}

Vec3 Vec3::Slerp( const Vec3& end, float t, float epsilon ) const
{
  if ( Length() < epsilon || end.Length() < epsilon )
  {
    return Vec3( 0.0f );
  }
  Vec3 v0 = NormalizeCopy();
  Vec3 v1 = end.NormalizeCopy();
  float d = ae::Clip( v0.Dot( v1 ), -1.0f, 1.0f );
  if ( d > ( 1.0f - epsilon ) )
  {
    return v1;
  }
  if ( d < -( 1.0f - epsilon ) )
  {
    return v0;
  }
  float angle = std::acos( d ) * t;
  Vec3 v2 = v1 - v0 * d;
  v2.Normalize();
  return ( ( v0 * std::cos( angle ) ) + ( v2 * std::sin( angle ) ) );
}

//------------------------------------------------------------------------------
// Log levels internal implementation
//------------------------------------------------------------------------------
const char* LogLevelNames[] =
{
  "TRACE",
  "DEBUG",
  "INFO ",
  "WARN ",
  "ERROR",
  "FATAL"
};

//------------------------------------------------------------------------------
// Log colors internal implementation
//------------------------------------------------------------------------------
#if _AE_LOG_COLORS_
const char* LogLevelColors[] =
{
  "\x1b[94m",
  "\x1b[36m",
  "\x1b[32m",
  "\x1b[33m",
  "\x1b[31m",
  "\x1b[35m"
};
#endif

//------------------------------------------------------------------------------
// Logging functions internal implementation
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
void LogInternal( std::stringstream& os, const char* message )
{
  os << message << std::endl;
  printf( os.str().c_str() ); // std out
  OutputDebugStringA( os.str().c_str() ); // visual studio debug output
}
#else
void LogInternal( std::stringstream& os, const char* message )
{
  std::cout << os.str() << message << std::endl;
}
#endif

void LogFormat( std::stringstream& os, uint32_t severity, const char* filePath, uint32_t line, const char* assertInfo, const char* format )
{
  char timeBuf[ 16 ];
  time_t t = time( nullptr );
  tm* lt = localtime( &t );
  timeBuf[ strftime( timeBuf, sizeof( timeBuf ), "%H:%M:%S", lt ) ] = '\0';

  const char* fileName = strrchr( filePath, '/' );
  if ( fileName )
  {
    fileName++; // Remove end forward slash
  }
  else if ( ( fileName = strrchr( filePath, '\\' ) ) )
  {
    fileName++; // Remove end backslash
  }
  else
  {
    fileName = filePath;
  }

#if _AE_LOG_COLORS_
  os << "\x1b[90m" << timeBuf;
  os << " [" << ae::GetPID() << "] ";
  os << LogLevelColors[ severity ] << LogLevelNames[ severity ];
  os << " \x1b[90m" << fileName << ":" << line;
#else
  os << timeBuf;
  os << " [" << ae::GetPID() << "] ";
  os << LogLevelNames[ severity ];
  os << " " << fileName << ":" << line;
#endif

  bool hasAssertInfo = ( assertInfo && assertInfo[ 0 ] );
  bool hasFormat = ( format && format[ 0 ] );
  if ( hasAssertInfo || hasFormat )
  {
    os << ": ";
  }
#if _AE_LOG_COLORS_
  os << "\x1b[0m";
#endif
  if ( hasAssertInfo )
  {
    os << assertInfo;
    if ( hasFormat )
    {
      os << " ";
    }
  }
}

//------------------------------------------------------------------------------
// DefaultAllocator class
//------------------------------------------------------------------------------
class DefaultAllocator final : public Allocator
{
public:
  void* Allocate( ae::Tag tag, uint32_t bytes, uint32_t alignment ) override
  {
#if _AE_WINDOWS_
    return _aligned_malloc( bytes, alignment );
#elif _AE_LINUX_
    return aligned_alloc( alignment, bytes );
#else
    // @HACK: macosx clang c++11 does not have aligned alloc
    return malloc( bytes );
#endif
  }

  void* Reallocate( void* data, uint32_t bytes, uint32_t alignment ) override
  {
#if _AE_WINDOWS_
    return _aligned_realloc( data, bytes, alignment );
#else
    aeCompilationWarning( "Aligned realloc() not determined on this platform" )
      return nullptr;
#endif
  }

  void Free( void* data ) override
  {
#if _AE_WINDOWS_
    _aligned_free( data );
#elif _AE_LINUX_
    free( data );
#else
    free( data );
#endif
  }
};

//------------------------------------------------------------------------------
// Allocator functions
//------------------------------------------------------------------------------
static Allocator* g_allocator = nullptr;

void SetGlobalAllocator( Allocator* alloc )
{
  AE_ASSERT_MSG( alloc, "No allocator provided to ae::SetGlobalAllocator()" );
  AE_ASSERT_MSG( !g_allocator, "Call ae::SetGlobalAllocator() before making any allocations to use your own allocator" );
  g_allocator = alloc;
}

Allocator* GetGlobalAllocator()
{
  if ( !g_allocator )
  {
    // @TODO: Allocating this statically here won't work for hotloading
    static DefaultAllocator s_allocator;
	  g_allocator = &s_allocator;
  }
  return g_allocator;
}

//------------------------------------------------------------------------------
// ae::TimeStep member functions
//------------------------------------------------------------------------------
TimeStep::TimeStep()
{
  m_stepCount = 0;
  m_timeStep = 0.0f;
  m_frameExcess = 0;
  m_prevFrameTime = 0.0f;

  SetTimeStep( 1.0f / 60.0f );
}

void TimeStep::SetTimeStep( float timeStep )
{
  m_timeStepSec = timeStep; m_timeStep = timeStep * 1000000.0f;
}

float TimeStep::GetTimeStep() const
{
  return m_timeStepSec;
}

uint32_t TimeStep::GetStepCount() const
{
  return m_stepCount;
}

float TimeStep::GetDt() const
{
  return m_prevFrameTimeSec;
}

float TimeStep::SetDt( float sec )
{
  m_prevFrameTimeSec = sec; // Useful for handling frames with high delta time, eg: timeStep.SetDT( timeStep.GetTimeStep()
}

void TimeStep::Wait()
{
  if ( m_timeStep == 0.0f )
  {
    return;
  }
  
  // @TODO: Maybe this should use the same time source as GetTime()
  
  if ( m_stepCount == 0 )
  {
    m_prevFrameTime = m_timeStep;
    m_frameStart = std::chrono::steady_clock::now();
  }
  else
  {
    std::chrono::steady_clock::time_point execFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds execDuration = std::chrono::duration_cast< std::chrono::microseconds >( execFinish - m_frameStart );
    
    int64_t prevFrameExcess = m_prevFrameTime - m_timeStep;
    m_frameExcess = ( m_frameExcess * 0.5f + prevFrameExcess * 0.5f ) + 0.5f;

    int64_t wait = m_timeStep - execDuration.count();
    wait -= ( m_frameExcess > 0 ) ? m_frameExcess : 0;
    if ( 1000 < wait && wait < m_timeStep )
    {
      std::this_thread::sleep_for( std::chrono::microseconds( wait ) );
    }
    std::chrono::steady_clock::time_point frameFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds frameDuration = std::chrono::duration_cast< std::chrono::microseconds >( frameFinish - m_frameStart );

    m_prevFrameTime = frameDuration.count();
    m_frameStart = std::chrono::steady_clock::now();
  }
  
  m_prevFrameTimeSec = m_prevFrameTime / 1000000.0f;
  
  m_stepCount++;
}

//------------------------------------------------------------------------------
// Window member functions
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
// @TODO: Cleanup namespace
LRESULT CALLBACK WinProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  ae::Window* window = (ae::Window*)GetWindowLongPtr( hWnd, GWLP_USERDATA );
  switch ( msg )
  {
    case WM_CREATE:
    {
      // Store ae window pointer in window state. Retrievable with GetWindowLongPtr()
      // https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-?redirectedfrom=MSDN
      // @TODO: Handle these error cases gracefully
      CREATESTRUCT* createMsg = (CREATESTRUCT*)lParam;
      AE_ASSERT( createMsg );
      ae::Window* window = (ae::Window*)createMsg->lpCreateParams;
      AE_ASSERT( window );
      SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)window );
      AE_ASSERT( window == (ae::Window*)GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
      window->window = hWnd;
      break;
    }
    case WM_SIZE:
    {
      if ( window->graphicsDevice )
      {
        uint32_t width = LOWORD( lParam );
        uint32_t height = HIWORD( lParam );
        window->m_UpdateWidthHeight( width, height );
        window->graphicsDevice->m_HandleResize( width, height );
      }
      break;
    }
    case WM_CLOSE:
    {
      PostQuitMessage( 0 );
      break;
    }
  }
  return DefWindowProc( hWnd, msg, wParam, lParam );
}
#endif

Window::Window()
{
  window = nullptr;
  graphicsDevice = nullptr;
  m_pos = Int2( 0.0f ); // @TODO: int
  m_width = 0;
  m_height = 0;
  m_fullScreen = false;
  m_maximized = false;
}

bool Window::Initialize( uint32_t width, uint32_t height, bool fullScreen, bool showCursor )
{
  AE_ASSERT( !window );

  //m_pos = Int2( fullScreen ? 0 : (int)SDL_WINDOWPOS_CENTERED );
  m_width = width;
  m_height = height;
  m_fullScreen = fullScreen;

  m_Initialize();

  //SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );
  //SDL_GetWindowPosition( (SDL_Window*)window, &m_pos.x, &m_pos.y );

  return false;
}

bool Window::Initialize( Int2 pos, uint32_t width, uint32_t height, bool showCursor )
{
  AE_ASSERT( !window );

  m_pos = pos;
  m_width = width;
  m_height = height;
  m_fullScreen = false;

  m_Initialize();

  //SDL_ShowCursor( showCursor ? SDL_ENABLE : SDL_DISABLE );

  return false;
}

void Window::m_Initialize()
{
//  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER ) < 0 )
//  {
//    AE_FAIL_MSG( "SDL could not initialize: #", SDL_GetError() );
//  }
//
//#if _AE_IOS_
//  m_pos = Int2( 0 );
//  m_fullScreen = true;
//
//  SDL_DisplayMode displayMode;
//  if ( SDL_GetDesktopDisplayMode( 0, &displayMode ) == 0 )
//  {
//    m_width = displayMode.w;
//    m_height = displayMode.h;
//  }
//
//  window = SDL_CreateWindow( "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, SDL_WINDOW_SHOWN );
//#else
//  Rect windowRect( m_pos.x, m_pos.y, m_width, m_height );
//  bool overlapsAny = false;
//  uint32_t displayCount = SDL_GetNumVideoDisplays();
//  for ( uint32_t i = 0; i < displayCount; i++ )
//  {
//    SDL_Rect rect;
//    int result = SDL_GetDisplayBounds( i, &rect );
//    if ( result == 0 )
//    {
//      Rect screenRect( rect.x, rect.y, rect.w, rect.h );
//      Rect intersection;
//      if ( windowRect.GetIntersection( screenRect, &intersection ) )
//      {
//        // Check how much window overlaps. This prevent windows that are barely overlapping from appearing offscreen.
//        float intersectionArea = intersection.w * intersection.h;
//        float screenArea = screenRect.w * screenRect.h;
//        float windowArea = windowRect.w * windowRect.h;
//        float screenOverlap = intersectionArea / screenArea;
//        float windowOverlap = intersectionArea / windowArea;
//        if ( screenOverlap > 0.1f || windowOverlap > 0.1f )
//        {
//          overlapsAny = true;
//          break;
//        }
//      }
//    }
//  }
//
//  if ( !overlapsAny && displayCount )
//  {
//    SDL_Rect screenRect;
//    if ( SDL_GetDisplayBounds( 0, &screenRect ) == 0 )
//    {
//      int32_t border = screenRect.w / 16;
//
//      m_width = screenRect.w - border * 2;
//      int32_t h0 = screenRect.h - border * 2;
//      int32_t h1 = m_width * ( 10.0f / 16.0f );
//      m_height = aeMath::Min( h0, h1 );
//
//      m_pos.x = border;
//      m_pos.y = ( screenRect.h - m_height ) / 2;
//      m_pos.x += screenRect.x;
//      m_pos.y += screenRect.y;
//
//      m_fullScreen = false;
//    }
//  }
//
//  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
//  flags |= m_fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE;
//  window = SDL_CreateWindow( "", m_pos.x, m_pos.y, m_width, m_height, flags );
//#endif
//  AE_ASSERT( window );
//
//  SDL_SetWindowTitle( (SDL_Window*)window, "" );
//  m_windowTitle = "";

#if _AE_WINDOWS_
#define WNDCLASSNAME L"wndclass"
  HINSTANCE hinstance = GetModuleHandle( NULL );

  WNDCLASSEX ex;
  memset( &ex, 0, sizeof( ex ) );
  ex.cbSize = sizeof( ex );
  ex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  ex.lpfnWndProc = WinProc;
  ex.hInstance = hinstance;
  ex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
  ex.hCursor = LoadCursor( NULL, IDC_ARROW );
  ex.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
  ex.lpszClassName = WNDCLASSNAME;
  if ( !RegisterClassEx( &ex ) ) // Create the window
  {
    AE_FAIL_MSG( "Failed to register window. Error: #", GetLastError() );
  }

  // WS_POPUP for full screen
  uint32_t windowStyle = WS_OVERLAPPEDWINDOW;
  windowStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  HWND hwnd = CreateWindowEx( NULL, WNDCLASSNAME, L"Window", WS_OVERLAPPEDWINDOW, 0, 0, GetWidth(), GetHeight(), NULL, NULL, hinstance, this );
  AE_ASSERT_MSG( hwnd, "Failed to create window. Error: #", GetLastError() );

  HDC hdc = GetDC( hwnd );
  AE_ASSERT_MSG( hdc, "Failed to Get the Window Device Context" );

  // Choose the best pixel format for the curent environment
  PIXELFORMATDESCRIPTOR pfd;
  memset( &pfd, 0, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int indexPixelFormat = ChoosePixelFormat( hdc, &pfd );
  AE_ASSERT_MSG( indexPixelFormat, "Failed to choose pixel format. Error: #", GetLastError() );
  if ( !DescribePixelFormat( hdc, indexPixelFormat, sizeof( pfd ), &pfd ) )
  {
    AE_FAIL_MSG( "Failed to read chosen pixel format. Error: #", GetLastError() );
  }
  AE_INFO( "Chosen Pixel format: #bit RGB #bit Depth",
    (int)pfd.cColorBits,
    (int)pfd.cDepthBits
  );
  if ( !SetPixelFormat( hdc, indexPixelFormat, &pfd ) )
  {
    AE_FAIL_MSG( "Could not set window pixel format. Error: #", GetLastError() );
  }

  // Create OpenGL context
  HGLRC hglrc = wglCreateContext( hdc );
  AE_ASSERT_MSG( hglrc, "Failed to create the OpenGL Rendering Context" );
  if ( !wglMakeCurrent( hdc, hglrc ) )
  {
    AE_FAIL_MSG( "Failed to make OpenGL Rendering Context current" );
  }

  // Finish window setup
  ShowWindow( hwnd, SW_SHOW );
  SetForegroundWindow( hwnd ); // Slightly Higher Priority
  SetFocus( hwnd ); // Sets Keyboard Focus To The Window
  if ( !UpdateWindow( hwnd ) )
  {
    AE_FAIL_MSG( "Failed on first window update. Error: #", GetLastError() );
  }
#endif
}

void Window::Terminate()
{
  //SDL_DestroyWindow( (SDL_Window*)window );
}

void Window::SetTitle( const char* title )
{
  if ( window && m_windowTitle != title )
  {
    //SDL_SetWindowTitle( (SDL_Window*)window, title );
    m_windowTitle = title;
  }
}

void Window::SetFullScreen( bool fullScreen )
{
  //if ( window )
  //{
  //  uint32_t oldFlags = SDL_GetWindowFlags( (SDL_Window*)window );

  //  uint32_t newFlags = oldFlags;
  //  if ( fullScreen )
  //  {
  //    newFlags |= SDL_WINDOW_FULLSCREEN;
  //  }
  //  else
  //  {
  //    newFlags &= ~SDL_WINDOW_FULLSCREEN;
  //  }

  //  if ( newFlags != oldFlags )
  //  {
  //    SDL_SetWindowFullscreen( (SDL_Window*)window, newFlags );
  //  }

  //  m_fullScreen = fullScreen;
  //}
}

void Window::SetPosition( Int2 pos )
{
  //if ( window )
  //{
  //  SDL_SetWindowPosition( (SDL_Window*)window, pos.x, pos.y );
  //  m_pos = pos;
  //}
}

void Window::SetSize( uint32_t width, uint32_t height )
{
  //if ( window )
  //{
  //  SDL_SetWindowSize( (SDL_Window*)window, width, height );
  //  m_width = width;
  //  m_height = height;
  //}
}

void Window::SetMaximized( bool maximized )
{
  //if ( maximized )
  //{
  //  SDL_MaximizeWindow( (SDL_Window*)window );
  //}
  //else
  //{
  //  SDL_RestoreWindow( (SDL_Window*)window );
  //}
  //m_maximized = maximized;
}

//------------------------------------------------------------------------------
// ae::Input member functions
//------------------------------------------------------------------------------
void Input::Pump()
{
#if _AE_WINDOWS_
  MSG msg;
  // Get messages for current thread
  while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
  {
    switch ( msg.message )
    {
      case WM_QUIT:
        quit = true;
        break;
    }
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
#endif
}

//------------------------------------------------------------------------------
// OpenGL includes
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
	#pragma comment (lib, "opengl32.lib")
	#pragma comment (lib, "glu32.lib")
	#include <gl/GL.h>
	#include <gl/GLU.h>
#endif

//------------------------------------------------------------------------------
// ae::RenderTarget member functions
//------------------------------------------------------------------------------
RenderTarget::~RenderTarget()
{
  Destroy();
}

void RenderTarget::Initialize( uint32_t width, uint32_t height )
{
  Destroy();

  AE_ASSERT( m_fbo == 0 );

  AE_ASSERT( width != 0 );
  AE_ASSERT( height != 0 );

  m_width = width;
  m_height = height;

  //glGenFramebuffers( 1, &m_fbo );
  //AE_ASSERT( m_fbo );
  //glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

  //AE_CHECK_GL_ERROR();
  //Vertex quadVerts[] =
  //{
  //  { aeQuadVertPos[ 0 ], aeQuadVertUvs[ 0 ] },
  //  { aeQuadVertPos[ 1 ], aeQuadVertUvs[ 1 ] },
  //  { aeQuadVertPos[ 2 ], aeQuadVertUvs[ 2 ] },
  //  { aeQuadVertPos[ 3 ], aeQuadVertUvs[ 3 ] }
  //};
  //AE_STATIC_ASSERT( countof( quadVerts ) == aeQuadVertCount );
  //m_quad.Initialize( sizeof( Vertex ), sizeof( aeQuadIndex ), aeQuadVertCount, aeQuadIndexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
  //m_quad.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( Vertex, pos ) );
  //m_quad.AddAttribute( "a_uv", 2, aeVertexDataType::Float, offsetof( Vertex, uv ) );
  //m_quad.SetVertices( quadVerts, aeQuadVertCount );
  //m_quad.SetIndices( aeQuadIndices, aeQuadIndexCount );
  //AE_CHECK_GL_ERROR();

  //const char* vertexStr = "\
  //  AE_UNIFORM_HIGHP mat4 u_localToNdc;\
  //  AE_IN_HIGHP vec3 a_position;\
  //  AE_IN_HIGHP vec2 a_uv;\
  //  AE_OUT_HIGHP vec2 v_uv;\
  //  void main()\
  //  {\
  //    v_uv = a_uv;\
  //    gl_Position = u_localToNdc * vec4( a_position, 1.0 );\
  //  }";
  //const char* fragStr = "\
  //  uniform sampler2D u_tex;\
  //  AE_IN_HIGHP vec2 v_uv;\
  //  void main()\
  //  {\
  //    AE_COLOR = AE_TEXTURE2D( u_tex, v_uv );\
  //  }";
  //m_shader.Initialize( vertexStr, fragStr, nullptr, 0 );

  //AE_CHECK_GL_ERROR();
}

void RenderTarget::Destroy()
{
  //m_shader.Destroy();
  //m_quad.Destroy();

  //for ( uint32_t i = 0; i < m_targets.Length(); i++ )
  //{
  //  m_targets[ i ]->Destroy();
  //  ae::Delete( m_targets[ i ] );
  //}
  //m_targets.Clear();

  //m_depth.Destroy();

  //if ( m_fbo )
  //{
  //  glDeleteFramebuffers( 1, &m_fbo );
  //  m_fbo = 0;
  //}

  m_width = 0;
  m_height = 0;
}

void RenderTarget::AddTexture( TextureFilter filter, TextureWrap wrap )
{
  //AE_ASSERT( m_targets.Length() < kMaxFrameBufferAttachments );

  //aeTexture2D* tex = ae::New< aeTexture2D >( AE_ALLOC_TAG_RENDER );
  //tex->Initialize( nullptr, m_width, m_height, aeTextureFormat::RGBA16F, aeTextureType::HalfFloat, filter, wrap );

  //GLenum attachement = GL_COLOR_ATTACHMENT0 + m_targets.Length();
  //glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  //glFramebufferTexture2D( GL_FRAMEBUFFER, attachement, tex->GetTarget(), tex->GetTexture(), 0 );

  //m_targets.Append( tex );

  //AE_CHECK_GL_ERROR();
}

void RenderTarget::AddDepth( TextureFilter filter, TextureWrap wrap )
{
  //AE_ASSERT_MSG( m_depth.GetTexture() == 0, "Render target already has a depth texture" );

  //m_depth.Initialize( nullptr, m_width, m_height, aeTextureFormat::Depth32F, aeTextureType::Float, filter, wrap );
  //glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  //glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth.GetTarget(), m_depth.GetTexture(), 0 );

  //AE_CHECK_GL_ERROR();
}

void RenderTarget::Activate()
{
  //CheckFramebufferComplete( m_fbo );
  //glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_fbo );
  //
  //GLenum buffers[] =
  //{
  //  GL_COLOR_ATTACHMENT0,
  //  GL_COLOR_ATTACHMENT1,
  //  GL_COLOR_ATTACHMENT2,
  //  GL_COLOR_ATTACHMENT3,
  //  GL_COLOR_ATTACHMENT4,
  //  GL_COLOR_ATTACHMENT5,
  //  GL_COLOR_ATTACHMENT6,
  //  GL_COLOR_ATTACHMENT7,
  //  GL_COLOR_ATTACHMENT8,
  //  GL_COLOR_ATTACHMENT9,
  //  GL_COLOR_ATTACHMENT10,
  //  GL_COLOR_ATTACHMENT11,
  //  GL_COLOR_ATTACHMENT12,
  //  GL_COLOR_ATTACHMENT13,
  //  GL_COLOR_ATTACHMENT14,
  //  GL_COLOR_ATTACHMENT15
  //};
  //AE_STATIC_ASSERT( countof( buffers ) == kMaxFrameBufferAttachments );
  //glDrawBuffers( m_targets.Length(), buffers );

  //glViewport( 0, 0, GetWidth(), GetHeight() );
}

void RenderTarget::Clear( Color color )
{
  Activate();

  //AE_CHECK_GL_ERROR();

  //Vec3 clearColor = color.GetLinearRGB();
  //glClearColor( clearColor.x, clearColor.y, clearColor.z, 1.0f );
  //glClearDepth( gReverseZ ? 0.0f : 1.0f );

  //glDepthMask( GL_TRUE );
  //glDisable( GL_DEPTH_TEST );
  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //AE_CHECK_GL_ERROR();
}

void RenderTarget::Render( const Shader* shader, const UniformList& uniforms )
{
  //glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );
  //m_quad.Render( shader, uniforms );
}

void RenderTarget::Render2D( uint32_t textureIndex, Rect ndc, float z )
{
  //glBindFramebuffer( GL_READ_FRAMEBUFFER, m_fbo );

  //aeUniformList uniforms;
  //uniforms.Set( "u_localToNdc", RenderTarget::GetQuadToNDCTransform( ndc, z ) );
  //uniforms.Set( "u_tex", GetTexture( textureIndex ) );
  //m_quad.Render( &m_shader, uniforms );
}

const Texture2D* RenderTarget::GetTexture( uint32_t index ) const
{
  return m_targets[ index ];
}

const Texture2D* RenderTarget::GetDepth() const
{
  //return m_depth.GetTexture() ? &m_depth : nullptr;
  return nullptr;
}

uint32_t RenderTarget::GetWidth() const
{
  return m_width;
}

uint32_t RenderTarget::GetHeight() const
{
  return m_height;
}

Matrix4 RenderTarget::GetTargetPixelsToLocalTransform( uint32_t otherPixelWidth, uint32_t otherPixelHeight, Rect ndc ) const
{
  //Matrix4 windowToNDC = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) );
  //windowToNDC.Scale( Vec3( 2.0f / otherPixelWidth, 2.0f / otherPixelHeight, 1.0f ) );

  //Matrix4 ndcToQuad = RenderTarget::GetQuadToNDCTransform( ndc, 0.0f );
  //ndcToQuad.Invert();

  //Matrix4 quadToRender = Matrix4::Scaling( Vec3( m_width, m_height, 1.0f ) );
  //quadToRender.Translate( Vec3( 0.5f, 0.5f, 0.0f ) );

  //return ( quadToRender * ndcToQuad * windowToNDC );
  return {};
}

Rect RenderTarget::GetNDCFillRectForTarget( uint32_t otherWidth, uint32_t otherHeight ) const
{
  //float canvasAspect = m_width / (float)m_height;
  //float targetAspect = otherWidth / (float)otherHeight;
  //if ( canvasAspect >= targetAspect )
  //{
  //  // Fit width
  //  float height = targetAspect / canvasAspect;
  //  return Rect( -1.0f, -height, 2.0f, height * 2.0f );
  //}
  //else
  //{
  //  // Fit height
  //  float width = canvasAspect / targetAspect;
  //  return Rect( -width, -1.0f, width * 2.0f, 2.0f );
  //}
  return {};
}

Matrix4 RenderTarget::GetTargetPixelsToWorld( const Matrix4& otherTargetToLocal, const Matrix4& worldToNdc ) const
{
  //Matrix4 canvasToNdc = Matrix4::Translation( Vec3( -1.0f, -1.0f, 0.0f ) ) * Matrix4::Scaling( Vec3( 2.0f / GetWidth(), 2.0f / GetHeight(), 1.0f ) );
  //return ( worldToNdc.Inverse() * canvasToNdc * otherTargetToLocal );
  return {};
}

Matrix4 RenderTarget::GetQuadToNDCTransform( Rect ndc, float z )
{
  //Matrix4 localToNdc = Matrix4::Translation( Vec3( ndc.x, ndc.y, z ) );
  //localToNdc.Scale( Vec3( ndc.w, ndc.h, 1.0f ) );
  //localToNdc.Translate( Vec3( 0.5f, 0.5f, 0.0f ) );
  //return localToNdc;
  return {};
}

//------------------------------------------------------------------------------
// GraphicsDevice member functions
//------------------------------------------------------------------------------
GraphicsDevice::GraphicsDevice()
{
  m_window = nullptr;

  // OpenGL
  m_context = nullptr;
  m_defaultFbo = 0;
}

GraphicsDevice::~GraphicsDevice()
{
  Terminate();
}

void GraphicsDevice::Initialize( class Window* window )
{
  AE_ASSERT( window );
  //AE_ASSERT_MSG( window->window, "Window must be initialized prior to GraphicsDevice initialization." );
  AE_ASSERT_MSG( !m_context, "GraphicsDevice already initialized" );

  m_window = window;
  window->graphicsDevice = this;

//  // TODO: needed on ES2/GL/WebGL1, but not on ES3/WebGL2
//#if !_AE_IOS_
//  AE_STATIC_ASSERT( GL_ARB_framebuffer_sRGB );
//#endif
//
//#if _AE_IOS_
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
//#else
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
//  if ( gGL41 )
//  {
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
//  }
//  else
//  {
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
//    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
//  }
//#endif
//
//  SDL_GL_SetAttribute( SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1 );
//
//#if AE_GL_DEBUG_MODE
//  SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );
//#endif
//  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
//  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
//
//  m_context = SDL_GL_CreateContext( (SDL_Window*)m_window->window );
//  AE_ASSERT( m_context );
//  SDL_GL_MakeCurrent( (SDL_Window*)m_window->window, m_context );
//
//  SDL_GL_SetSwapInterval( 1 );
//
//#if _AE_WINDOWS_
//  glewExperimental = GL_TRUE;
//  GLenum err = glewInit();
//  glGetError(); // Glew currently has an issue which causes a GL_INVALID_ENUM on init
//  AE_ASSERT_MSG( err == GLEW_OK, "Could not initialize glew" );
//#endif
//
//#if AE_GL_DEBUG_MODE
//  glDebugMessageCallback( aeOpenGLDebugCallback, nullptr );
//#endif
//
//  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFbo );
//
//  AE_CHECK_GL_ERROR();

#if _AE_WINDOWS_
  // @TODO: Remove start
  glShadeModel( GL_SMOOTH );							// Enable Smooth Shading
  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );				// Black Background
  glClearDepth( 1.0f );									// Depth Buffer Setup
  glEnable( GL_DEPTH_TEST );							// Enables Depth Testing
  glDepthFunc( GL_LEQUAL );								// The Type Of Depth Testing To Do
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ); // Really Nice Perspective Calculations
  // @TODO: Remove end
#endif

  m_HandleResize( m_window->GetWidth(), m_window->GetHeight() );

}

void GraphicsDevice::Terminate()
{
  if ( m_context )
  {
    //SDL_GL_DeleteContext( m_context );
    m_context = 0;
  }
}

void GraphicsDevice::Activate()
{
  //AE_ASSERT( m_context );

  if ( m_window->GetWidth() != m_canvas.GetWidth() || m_window->GetHeight() != m_canvas.GetHeight() )
  {
    m_HandleResize( m_window->GetWidth(), m_window->GetHeight() );
  }
  m_canvas.Activate();

//#if !_AE_IOS_
//  // This is automatically enabled on opengl es3 and can't be turned off
//  glEnable( GL_FRAMEBUFFER_SRGB );
//#endif

#if _AE_WINDOWS_
  // @TODO: Remove start
  glViewport( 0, 0, m_canvas.GetWidth(), m_canvas.GetHeight() ); // Reset The Current Viewport

  glMatrixMode( GL_PROJECTION ); // Select The Projection Matrix
  glLoadIdentity(); // Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  float aspectRatio = m_canvas.GetWidth() / (float)m_canvas.GetHeight();
  gluPerspective( 45.0f, aspectRatio, 0.1f, 100.0f );

  glMatrixMode( GL_MODELVIEW ); // Select The Modelview Matrix
  glLoadIdentity();

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// Clear Screen And Depth Buffer
  glLoadIdentity();									// Reset The Current Modelview Matrix
  glTranslatef( -1.5f, 0.0f, -6.0f );						// Move Left 1.5 Units And Into The Screen 6.0
  glBegin( GL_TRIANGLES );								// Drawing Using Triangles
  glVertex3f( 0.0f, 1.0f, 0.0f );					// Top
  glVertex3f( -1.0f, -1.0f, 0.0f );					// Bottom Left
  glVertex3f( 1.0f, -1.0f, 0.0f );					// Bottom Right
  glEnd();											// Finished Drawing The Triangle
  glTranslatef( 3.0f, 0.0f, 0.0f );						// Move Right 3 Units
  glBegin( GL_QUADS );									// Draw A Quad
  glVertex3f( -1.0f, 1.0f, 0.0f );					// Top Left
  glVertex3f( 1.0f, 1.0f, 0.0f );					// Top Right
  glVertex3f( 1.0f, -1.0f, 0.0f );					// Bottom Right
  glVertex3f( -1.0f, -1.0f, 0.0f );					// Bottom Left
  glEnd();											// Done Drawing The Quad
  // @TODO: Remove end
#endif
}

void GraphicsDevice::Clear( Color color )
{
  Activate();
  m_canvas.Clear( color );
}

void GraphicsDevice::Present()
{
  //AE_ASSERT( m_context );
  //AE_CHECK_GL_ERROR();

  //glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_defaultFbo );
  //glViewport( 0, 0, m_window->GetWidth(), m_window->GetHeight() );

  //// Clear window target in case canvas doesn't fit exactly
  //glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  //glClearDepth( 1.0f );

  //glDepthMask( GL_TRUE );

  //glDisable( GL_DEPTH_TEST );
  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  //AE_CHECK_GL_ERROR();

  //m_canvas.Render2D( 0, Rect( Vec2( -1.0f ), Vec2( 1.0f ) ), 0.5f );

//#if !_AE_EMSCRIPTEN_
//  SDL_GL_SwapWindow( (SDL_Window*)m_window->window );
//#endif
//
//  AE_CHECK_GL_ERROR();

#if _AE_WINDOWS_
  AE_ASSERT( m_window );
  HWND hWnd = (HWND)m_window->window;
  AE_ASSERT( hWnd );
  HDC hdc = GetDC( hWnd );
  SwapBuffers( hdc ); // Swap Buffers
#endif
}

float GraphicsDevice::GetAspectRatio() const
{
  if ( m_canvas.GetWidth() + m_canvas.GetHeight() == 0 )
  {
    return 0.0f;
  }
  else
  {
    return m_canvas.GetWidth() / (float)m_canvas.GetHeight();
  }
}

void GraphicsDevice::AddTextureBarrier()
{
//  // only GL has texture barrier for reading from previously written textures
//  // There are less draconian ways in desktop ES, and nothing in WebGL.
//#if _AE_WINDOWS_ || _AE_OSX_
//  glTextureBarrierNV();
//#endif
}

void GraphicsDevice::m_HandleResize( uint32_t width, uint32_t height )
{
  // @TODO: Allow user to pass in a canvas scaling factor / aspect ratio parameter
  m_canvas.Initialize( width, height );
  m_canvas.AddTexture( TextureFilter::Nearest, TextureWrap::Clamp );
  m_canvas.AddDepth( TextureFilter::Nearest, TextureWrap::Clamp );
}

} // AE_NAMESPACE end

#endif // AE_MAIN
#endif // AE_AETHER_H
