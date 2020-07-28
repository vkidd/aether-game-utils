//------------------------------------------------------------------------------
// 09_Platformer2D.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "aeClock.h"
#include "aeHotSpot.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeSignal.h"
#include "aeSparseGrid.h"
#include "aeWindow.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kTileMask_Open = 0;
const uint32_t kTileMask_Collision = 1;

const uint32_t kTile_Air = 0;
const uint32_t kTile_Wall = 1;
const uint32_t kTile_Water = 2;

const float kJumpMaxAirTime = 0.35f;
const float kJumpHoldTimeMax = 0.4f;

//------------------------------------------------------------------------------
// Player class
//------------------------------------------------------------------------------
class Player
{
public:
  Player( HotSpotWorld* world, aeFloat2 startPos );
  void OnCollision( const HotSpotObject::CollisionInfo* info );
  void Update( HotSpotWorld* world, aeInput* input, float dt );

  void Render( aeSpriteRender* spriteRender, aeTexture2D* tex );
  aeFloat2 Player::GetPosition() const { return m_body->GetPosition(); }
  bool Player::CanJump() const { return m_canJumpTimer > 0.0f; }

private:
  HotSpotObject* m_body = nullptr;
  aeFloat2 m_startPos = aeFloat2( 0.0f );
  float m_canJumpTimer = 0.0f;
  float m_jumpHoldTimer = 0.0f;
};

//------------------------------------------------------------------------------
// Player member functions
//------------------------------------------------------------------------------
Player::Player( HotSpotWorld* world, aeFloat2 startPos )
{
  m_body = world->CreateObject();
  m_body->SetMass( 70.0f );
  m_body->SetVolume( m_body->GetMass() / 1050.0f );
  m_startPos = startPos;
  m_body->onCollision.Add( this, &Player::OnCollision );
  m_body->Warp( startPos );
}

void Player::OnCollision( const HotSpotObject::CollisionInfo* info )
{
  if ( info->normal.y > 0 )
  {
    m_canJumpTimer = kJumpMaxAirTime;
    m_jumpHoldTimer = 0.0f;
  }
}

void Player::Update( HotSpotWorld* world, aeInput* input, float dt )
{
  uint32_t tile = world->GetTile( HotSpotWorld::_GetTilePos( m_body->GetPosition() ) );
  bool jumpButton = ( input->GetState()->space || input->GetState()->up );

  m_canJumpTimer -= dt;

  // Water
  if ( tile == kTile_Water )
  {
    if ( jumpButton ) { m_body->AddForce( aeFloat2( 0.0f, 1000.0f ) ); }
    if ( input->GetState()->down ) { m_body->AddForce( aeFloat2( 0.0f, -700.0f ) ); }

    if ( input->GetState()->left ) { m_body->AddForce( aeFloat2( -70.0f * 6.5f, 0.0f ) ); }
    if ( input->GetState()->right ) { m_body->AddForce( aeFloat2( 70.0f * 6.5f, 0.0f ) );  }

    // Always reset jump so a jump is possible immediately after leaving water
    m_canJumpTimer = kJumpMaxAirTime;
    m_jumpHoldTimer = 0.0f;
  }
  else // Air / ground
  {
    if ( input->GetState()->left ) { m_body->AddForce( aeFloat2( -70.0f * 4.5f, 0.0f ) ); }
    if ( input->GetState()->right ) { m_body->AddForce( aeFloat2( 70.0f * 4.5f, 0.0f ) ); }

    if ( CanJump() && jumpButton )
    {
      // Cancel previous downward velocity for last kJumpMaxAirTime seconds
      // to get full jump height
      aeFloat2 vel = m_body->GetVelocity();
      vel.y = 0.0f;
      m_body->SetVelocity( vel );

      m_canJumpTimer = 0.0f;
      m_jumpHoldTimer = kJumpHoldTimeMax;
      m_body->AddImpulse( aeFloat2( 0.0f, 400.0f ) );
    }

    if ( m_jumpHoldTimer > 0.0f && jumpButton )
    {
      m_jumpHoldTimer -= dt;
      m_body->AddForce( aeFloat2( 0.0f, 400.0f ) );
    }
  }

  m_body->AddGravity( aeFloat2( 0.0f, -10.0f ) );
}

void Player::Render( aeSpriteRender* spriteRender, aeTexture2D* tex )
{
  aeFloat4x4 transform = aeFloat4x4::Translation( aeFloat3( GetPosition(), -0.5f ) );
  transform.Scale( aeFloat3( 1.0f, 1.0f, 1.0f ) );
  spriteRender->AddSprite( tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), CanJump() ? aeColor::Red() : aeColor::Blue() );
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  aeWindow window;
  aeRender render;
  aeInput input;
  aeSpriteRender spriteRender;
  
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "Platformer 2D" );  render.InitializeOpenGL( &window, 400, 300 );
  render.SetClearColor( aeColor::PicoDarkBlue() );
  input.Initialize( &window, &render );
  spriteRender.Initialize( 512 );
  spriteRender.SetBlending( true );
  spriteRender.SetDepthTest( true );
  spriteRender.SetDepthWrite( true );
  spriteRender.SetSorting( true );

  aeFixedTimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );

  HotSpotWorld world;
  world.Initialize();

  // Resources
  world.SetCollisionMask( kTileMask_Collision );
  world.SetTileProperties( kTile_Air, kTileMask_Open );
  world.SetTileFluidDensity( kTile_Air, 12.5f );
  world.SetTileProperties( kTile_Wall, kTileMask_Collision );
  world.SetTileProperties( kTile_Water, kTileMask_Open );
  world.SetTileFluidDensity( kTile_Water, 1000.0f );
#define O kTile_Air
#define B kTile_Wall
#define W kTile_Water
  const uint32_t kMapData[] =
  {
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,B,B,B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,B,B,B,B,B,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,B,B,B,
    B,O,O,O,O,O,O,O,B,B,B,B,B,O,O,O,O,O,O,O,O,O,O,O,O,O,O,B,
    B,O,O,O,O,O,O,O,O,O,B,B,W,W,W,W,W,W,B,B,B,B,O,O,O,O,O,B,
    B,O,O,O,O,O,B,B,O,O,B,B,W,W,W,W,W,W,B,B,B,B,B,W,W,W,W,B,
    B,O,O,O,O,O,O,O,O,O,B,B,W,W,W,W,W,W,B,B,B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,B,
  };
#undef O
#undef B
#undef W
  const uint32_t kMapWidth = 28;
  const uint32_t kMapHeight = 12;
  AE_STATIC_ASSERT( countof(kMapData) == kMapWidth * kMapHeight );
  world.LoadTiles( kMapData, kMapWidth, kMapHeight, true );
  
  aeTexture2D tex;
  uint8_t texData[] = { 255, 255, 255 };
  tex.Initialize( texData, 1, 1, aeTextureFormat::RGB, aeTextureType::Uint8, aeTextureFilter::Nearest, aeTextureWrap::Clamp );

  // Camera
  float scale = 10.0f;
  aeFloat2 camera( 0.0f );

  // Player
  Player player( &world, aeFloat2( 2.0f, 2.0f ) );

  // Game loop
  while ( !input.GetState()->exit )
  {
    input.Pump();
    render.Resize( window.GetWidth(), window.GetHeight() );
    render.StartFrame();
    spriteRender.Clear();

    player.Update( &world, &input, timeStep.GetTimeStep() );
    world.Update( timeStep.GetTimeStep() );

    camera = player.GetPosition();

    player.Render( &spriteRender, &tex );

    for ( uint32_t y = 0; y < kMapHeight; y++ )
    {
      for ( uint32_t x = 0; x < kMapWidth; x++ )
      {
        aeColor color;
        switch ( world.GetTile( aeInt2( x, y ) ) )
        {
          case kTile_Air: color = aeColor::PicoPeach(); break;
          case kTile_Water: color = aeColor::PicoPink(); break;
          default: color = aeColor::PicoOrange(); break;
        }
        aeFloat4x4 transform = aeFloat4x4::Translation( aeFloat3( x, y, 0.0f ) );
        transform.Scale( aeFloat3( 1.0f, 1.0f, 0.0f ) );
        spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), color );
      }
    }

    aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / scale, render.GetAspectRatio() / scale, 1.0f ) );
    screenTransform.Translate( aeFloat3( -camera.x, -camera.y, 0.0f ) );
    spriteRender.Render( screenTransform );
    render.EndFrame();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
