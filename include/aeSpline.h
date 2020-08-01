//------------------------------------------------------------------------------
// aeSpline.h
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#ifndef AESPLINE_H
#define AESPLINE_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeMath.h"

//------------------------------------------------------------------------------
// aeSpline class
//------------------------------------------------------------------------------
class aeSpline
{
public:
  aeSpline() = default;
  aeSpline( aeFloat3* controlPoints, uint32_t count );

  void AppendControlPoint( aeFloat3 p );
  void RemoveControlPoint( uint32_t index );
  void SetLooping( bool enabled );

  aeFloat3 GetControlPoint( uint32_t index ) const;
  uint32_t GetControlPointCount() const;

  aeFloat3 GetPoint( float distance ) const; // 0 <= distance <= length
  float GetLength() const;

private:
  struct Segment
  {
    void Init( aeFloat3 p0, aeFloat3 p1, aeFloat3 p2, aeFloat3 p3 );
    aeFloat3 GetPoint01( float t ) const;
    aeFloat3 GetPoint( float d ) const;

    aeFloat3 a;
    aeFloat3 b;
    aeFloat3 c;
    aeFloat3 d;
    float length;
    uint32_t resolution;
  };

  void m_RecalculateSegments();
  aeFloat3 m_GetControlPoint( int32_t index ) const;

  bool m_loop = false;
  aeArray< aeFloat3 > m_controlPoints;
  aeArray< Segment > m_segments;
  float m_length = 0.0f;
};

#endif
