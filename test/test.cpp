//------------------------------------------------------------------------------
// test.cpp
// Copyright (c) John Hughes on 12/2/19. All rights reserved.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "aeArray.h"

//------------------------------------------------------------------------------
// aeArray tests
//------------------------------------------------------------------------------
TEST_CASE( "arrays elements can be removed by value", "[aeArray]" )
{
  aeArray< int > array;
  for ( uint32_t i = 0; i < 10; i++ )
  {
    array.Append( i );
    array.Append( 0 );
  }
  REQUIRE( array.Length() == 20 );
  //"<0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0>"

  SECTION( "removing all values results in empty search" )
  {
    array.RemoveAll( 0 );
    REQUIRE( array[ 0 ] == 1 );
    REQUIRE( array.Length() == 9 );
    REQUIRE( array.Find( 0 ) == -1 );
  }
  
  SECTION( "removing a value results in remaining values shifting" )
  {
    REQUIRE( array[ 6 ] == 3 );
    array.RemoveAll( 3 );
    REQUIRE( array.Find( 3 ) == -1 );
    REQUIRE( array[ 6 ] == 0 );
    REQUIRE( array[ 7 ] == 4 );
    REQUIRE( array.Length() == 19 );
  }

  SECTION( "removing a value based on function results in empty search" )
  {
    array.RemoveAllFn( []( int a ){ return a >= 6; } );
    REQUIRE( array.Find( 5 ) == 10 );
    REQUIRE( array.Find( 6 ) == -1 );
    REQUIRE( array.Find( 7 ) == -1 );
    REQUIRE( array.Find( 8 ) == -1 );
    REQUIRE( array.Find( 9 ) == -1 );
    REQUIRE( array.Length() == 16 );
  }

}

TEST_CASE( "arrays can be sized and resized", "[aeArray]" )
{
  aeArray< int > a( 5 );

  REQUIRE( a.Length() == 0 );
  REQUIRE( a.Size() == 8 );

  SECTION( "reserving bigger changes size but not length" )
  {
    a.Reserve( 10 );

    REQUIRE( a.Length() == 0 );
    REQUIRE( a.Size() == 16 );
  }
  SECTION( "reserving smaller does not change length or size" )
  {
    a.Reserve( 0 );

    REQUIRE( a.Length() == 0 );
    REQUIRE( a.Size() == 8 );
  }
  SECTION( "clearing reduces length but does not affect size" )
  {
    a.Clear();

    REQUIRE( a.Length() == 0 );
    REQUIRE( a.Size() == 8 );
  }
}

TEST_CASE( "arrays can be constructed with a specified length", "[aeArray]" )
{
  aeArray< int > a( 5, 7 );

  REQUIRE( a.Length() == 5 );
  REQUIRE( a.Size() == 8 );
}

TEST_CASE( "arrays elements can be inserted by index", "[aeArray]" )
{
  aeArray< int > empty;
  REQUIRE( empty.Length() == 0 );

  aeArray< int > array;
  for ( uint32_t i = 1; i <= 5; i++ )
  {
    array.Append( i );
  }
  REQUIRE( array.Length() == 5 );

  SECTION( "insert at beginning moves all elements back" )
  {
    REQUIRE( array.Insert( 0, 7 ) == 7 );
    REQUIRE( array.Length() == 6 );
    REQUIRE( array[ 0 ] == 7 );
    REQUIRE( array[ 1 ] == 1 );
    REQUIRE( array[ 2 ] == 2 );
    REQUIRE( array[ 3 ] == 3 );
    REQUIRE( array[ 4 ] == 4 );
    REQUIRE( array[ 5 ] == 5 );
  }

  SECTION( "insert in middle moves later elements back" )
  {
    REQUIRE( array.Insert( 3, 7 ) == 7 );
    REQUIRE( array.Length() == 6 );
    REQUIRE( array[ 0 ] == 1 );
    REQUIRE( array[ 1 ] == 2 );
    REQUIRE( array[ 2 ] == 3 );
    REQUIRE( array[ 3 ] == 7 );
    REQUIRE( array[ 4 ] == 4 );
    REQUIRE( array[ 5 ] == 5 );
  }

  SECTION( "insert at end appends value" )
  {
    REQUIRE( array.Insert( 5, 7 ) == 7 );
    REQUIRE( array.Length() == 6 );
    REQUIRE( array[ 0 ] == 1 );
    REQUIRE( array[ 1 ] == 2 );
    REQUIRE( array[ 2 ] == 3 );
    REQUIRE( array[ 3 ] == 4 );
    REQUIRE( array[ 4 ] == 5 );
    REQUIRE( array[ 5 ] == 7 );
  }

  SECTION( "insert into empty array" )
  {
    REQUIRE( empty.Insert( 0, 7 ) == 7 );
    REQUIRE( empty.Length() == 1 );
    REQUIRE( empty[ 0 ] == 7 );
  }
}
