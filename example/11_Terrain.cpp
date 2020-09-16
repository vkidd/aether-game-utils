//------------------------------------------------------------------------------
// 11_Terrain.cpp
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
#include "aeEditorCamera.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeRender.h"
#include "aeSpline.h"
#include "aeTerrain.h"
#include "aeWindow.h"
#include "aeVfs.h"

#include "aeImGui.h"
#include "ImGuizmo.h"

//------------------------------------------------------------------------------
// Terrain Shader
//------------------------------------------------------------------------------
const char* kTerrainVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_UNIFORM vec4 u_topColor;\
	AE_UNIFORM vec4 u_sideColor;\
	AE_IN_HIGHP vec3 a_position;\
	AE_IN_HIGHP vec3 a_normal;\
	AE_OUT_HIGHP vec4 v_color;\
	AE_OUT_HIGHP vec3 v_normal;\
	void main()\
	{\
		float top = max(0.0, a_normal.z);\
		top *= top;\
		top *= top;\
		v_color = mix(u_sideColor, u_topColor, top);\
		v_normal = a_normal;\
		gl_Position = u_worldToProj * vec4( a_position, 1.0 );\
	}";

const char* kTerrainFragShader = "\
	AE_IN_HIGHP vec4 v_color;\
	AE_IN_HIGHP vec3 v_normal;\
	void main()\
	{\
		float light = dot( normalize( v_normal ), normalize( vec3( 1.0 ) ) );\
		light = max(0.0, light);\
		light = mix( 0.8, 4.0, light );\
		AE_COLOR = vec4( AE_RGB_TO_SRGB( v_color.rgb * vec3( light ) ), v_color.a );\
	}";

//------------------------------------------------------------------------------
// Grid Shader
//------------------------------------------------------------------------------
const char* kGridVertexStr = "\
    AE_UNIFORM_HIGHP mat4 u_screenToWorld;\
    AE_IN_HIGHP vec4 a_position;\
    AE_OUT_HIGHP vec3 v_worldPos;\
    void main()\
    {\
      v_worldPos = vec3( u_screenToWorld * a_position );\
      gl_Position = a_position;\
    }";

const char* kGridFragStr = "\
    AE_IN_HIGHP vec3 v_worldPos;\
    void main()\
    {\
      int x = int( floor( v_worldPos.x ) ) % 2;\
      int y = int( floor( v_worldPos.y ) ) % 2;\
      AE_COLOR.rgb = mix( vec3( 0.3 ), vec3( 0.35 ), int( x != y ) );\
      float gridX = mod( v_worldPos.x + 16.0, 32.0 ) - 16.0;\
      float gridY = mod( v_worldPos.y + 16.0, 32.0 ) - 16.0;\
      if ( abs( gridX ) < 0.05 || abs( gridY ) < 0.05 ) { AE_COLOR.rgb = vec3( 0.25 ); } \
      AE_COLOR.a = 1.0;\
    }";

class Grid
{
public:
	void Initialize()
	{
		struct BGVertex
		{
			aeFloat4 pos;
		};
		
		BGVertex bgVertices[ aeQuadVertCount ];
		uint8_t bgIndices[ aeQuadIndexCount ];
		for ( uint32_t i = 0; i < aeQuadVertCount; i++ )
		{
			bgVertices[ i ].pos = aeFloat4( aeQuadVertPos[ i ] * 2.0f, 1.0f );
		}
		for ( uint32_t i = 0; i < aeQuadIndexCount; i++ )
		{
			bgIndices[ i ] = aeQuadIndices[ i ];
		}

		m_bgVertexData.Initialize( sizeof( BGVertex ), sizeof( uint8_t ), countof( bgVertices ), countof( bgIndices ), aeVertexPrimitive::Triangle, aeVertexUsage::Static, aeVertexUsage::Static );
		m_bgVertexData.AddAttribute( "a_position", 4, aeVertexDataType::Float, offsetof( BGVertex, pos ) );
		m_bgVertexData.SetVertices( bgVertices, countof( bgVertices ) );
		m_bgVertexData.SetIndices( bgIndices, countof( bgIndices ) );

		m_gridShader.Initialize( kGridVertexStr, kGridFragStr, nullptr, 0 );
	}

	void Render( aeFloat4x4 worldToProj )
	{
		aeUniformList uniforms;
		uniforms.Set( "u_screenToWorld", worldToProj.Inverse() );
		m_bgVertexData.Render( &m_gridShader, uniforms );
	}

private:
	aeShader m_gridShader;
	aeVertexData m_bgVertexData;
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	{
	AE_INFO( "Initialize" );

	bool headless = _AE_LINUX_;

	aeWindow window;
	aeRender render;
	aeInput input;
	aeDebugRender debug;
	aeFixedTimeStep timeStep;
	aeShader terrainShader;
	aeEditorCamera camera;
	aeTextRender textRender;
	class aeImGui* ui = nullptr;

	ui = aeAlloc::Allocate< aeImGui >();
	if ( headless )
	{
		ui->InitializeHeadless();
	}
	else
	{
		window.Initialize( 800, 600, false, true );
		window.SetTitle( "terrain" );
		render.InitializeOpenGL( &window );
		render.SetClearColor( aeColor::PicoDarkPurple() );
		debug.Initialize();
		ui->Initialize();
	}

	input.Initialize( headless ? nullptr : &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	camera.SetPosition( aeFloat3( 150.0f, 150.f, 60.0f ) );
	camera.SetFocusDistance( 100.0f );
	
	if ( !headless )
	{
		terrainShader.Initialize( kTerrainVertShader, kTerrainFragShader, nullptr, 0 );
		terrainShader.SetDepthTest( true );
		terrainShader.SetDepthWrite( true );
	}

	textRender.Initialize( "font.png", aeTextureFilter::Nearest, 8 );

	aeAlloc::Scratch< uint8_t > fileBuffer( aeVfs::GetSize( "terrain.png" ) );
	aeVfs::Read( "terrain.png", fileBuffer.Data(), fileBuffer.Length() );

	uint32_t terrainThreads = aeMath::Max( 1u, (uint32_t)( aeGetMaxConcurrentThreads() * 0.75f ) );
	aeTerrain* terrain = aeAlloc::Allocate< aeTerrain >();
	terrain->Initialize( terrainThreads, !headless );

	aeFloat4x4 worldToText = aeFloat4x4::Identity();
	auto drawWorldText = [&]( aeFloat3 p, const char* str )
	{
		p = aeFloat3::ProjectPoint( worldToText, p );
		aeFloat2 fontSize( (float)textRender.GetFontSize() );
		textRender.Add( p, fontSize, str, aeColor::White(), 0, 0 );
	};

	bool wireframe = false;
	static bool s_showTerrainDebug = false;

	ae::Sdf* currentBox = nullptr;
	{
		ae::SdfBox* box = terrain->sdf.CreateSdf< ae::SdfBox >();
		box->m_center = camera.GetFocus();
		currentBox = box;
	}

	bool gizmoClickedPrev = false;
	aeFloat4x4 gizmoPrevTransform = aeFloat4x4::Identity();

	AE_INFO( "Run" );
	while ( !input.GetState()->exit )
	{
		input.Pump();

		ui->NewFrame( &render, &input, timeStep.GetTimeStep() );

		ImGuizmo::SetOrthographic( false );
		ImGuizmo::BeginFrame();

		// New terrain objects
		if ( ImGui::Begin( "create", nullptr ) )
		{
			if ( ImGui::Button( "cube" ) )
			{
				AE_LOG( "Cube" );

				ae::SdfBox* box = terrain->sdf.CreateSdf< ae::SdfBox >();
				box->m_center = camera.GetFocus();

				terrain->Dirty( box->GetAABB() );
				currentBox = box;
			}
			else if ( ImGui::Button( "Height Map" ) )
			{
				AE_LOG( "create height map" );

				ae::SdfHeightMap* heightMap = terrain->sdf.CreateSdf< ae::SdfHeightMap >();
				heightMap->m_center = camera.GetFocus();
				heightMap->m_heightMap.LoadFile( fileBuffer.Data(), fileBuffer.Length(), ae::Image::Extension::PNG, ae::Image::Format::R );

				terrain->Dirty( heightMap->GetAABB() );
				currentBox = heightMap;
			}
			ImGui::End();
		}

		if ( !ImGuizmo::IsUsing() )
		{
			if ( s_showTerrainDebug )
			{
				terrain->SetDebugTextCallback( drawWorldText );
			}
			else
			{
				terrain->SetDebugTextCallback( nullptr );
			}

			// Wait for click release of gizmo before starting new terrain jobs
			terrain->Update( camera.GetPosition(), 1250.0f );
		}

		// Camera input
		if ( !ImGuizmo::IsUsing() )
		{
			camera.Update( &input, timeStep.GetTimeStep() );
		}
		// Camera focus
		if ( currentBox && !input.GetPrevState()->Get( aeKey::F ) && input.GetState()->Get( aeKey::F ) )
		{
			camera.Refocus( currentBox->GetAABB().GetCenter() );
		}

		// Render mode
		if ( !input.GetPrevState()->Get( aeKey::Num1 ) && input.GetState()->Get( aeKey::Num1 ) )
		{
			wireframe = true;
			s_showTerrainDebug = true;
		}
		else if ( !input.GetPrevState()->Get( aeKey::Num2 ) && input.GetState()->Get( aeKey::Num2 ) )
		{
			wireframe = false;
			s_showTerrainDebug = false;
		}
		else if ( input.GetState()->Get( aeKey::Num3 ) && !input.GetPrevState()->Get( aeKey::Num3 ) )
		{
			wireframe = false;
			s_showTerrainDebug = true;
		}

		if ( !headless )
		{
			render.StartFrame( window.GetWidth(), window.GetHeight() );

			aeFloat4x4 worldToView = aeFloat4x4::WorldToView( camera.GetPosition(), camera.GetForward(), aeFloat3( 0.0f, 0.0f, 1.0f ) );
			aeFloat4x4 viewToProj = aeFloat4x4::ViewToProjection( 0.4f, render.GetAspectRatio(), 0.5f, 1000.0f );
			aeFloat4x4 worldToProj = viewToProj * worldToView;
			// UI units in pixels, origin in bottom left
			aeFloat4x4 textToNdc = aeFloat4x4::Scaling( aeFloat3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
			textToNdc *= aeFloat4x4::Translation( aeFloat3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );
			worldToText = textToNdc.Inverse() * worldToProj;

			aeFloat4x4 identity = aeFloat4x4::Identity();
			//ImGuizmo::DrawGrid( worldToView.GetTransposeCopy().data, viewToProj.GetTransposeCopy().data, identity.data, 100.0f );

			aeColor top = aeColor::PS( 46, 65, 35 );
			aeColor side = aeColor::PS( 84, 84, 74 );
			aeUniformList uniformList;
			uniformList.Set( "u_worldToProj", worldToProj );
			if ( wireframe )
			{
				uniformList.Set( "u_topColor", top.GetLinearRGBA() );
				uniformList.Set( "u_sideColor", side.GetLinearRGBA() );
				terrainShader.SetBlending( false );
				terrainShader.SetCulling( aeShaderCulling::None );
				terrainShader.SetWireframe( true );
				terrain->Render( &terrainShader, uniformList );

				uniformList.Set( "u_topColor", top.SetA( 0.5f ).GetLinearRGBA() );
				uniformList.Set( "u_sideColor", side.SetA( 0.5f ).GetLinearRGBA() );
				terrainShader.SetBlending( true );
				terrainShader.SetCulling( aeShaderCulling::CounterclockwiseFront );
				terrainShader.SetWireframe( false );
				terrain->Render( &terrainShader, uniformList );
			}
			else
			{
				uniformList.Set( "u_topColor", top.GetLinearRGBA() );
				uniformList.Set( "u_sideColor", side.GetLinearRGBA() );
				terrainShader.SetBlending( false );
				terrainShader.SetCulling( aeShaderCulling::CounterclockwiseFront );
				terrainShader.SetWireframe( false );
				terrain->Render( &terrainShader, uniformList );
			}

			if ( s_showTerrainDebug )
			{
				terrain->RenderDebug( &debug );
			}

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );

			static ImGuizmo::OPERATION s_operation = ImGuizmo::TRANSLATE;
			if ( input.GetState()->Get( aeKey::Q ) && !input.GetPrevState()->Get( aeKey::Q ) )
			{
				currentBox = nullptr;
			}
			else if ( input.GetState()->Get( aeKey::W ) && !input.GetPrevState()->Get( aeKey::W ) )
			{
				s_operation = ImGuizmo::TRANSLATE;
			}
			else if ( input.GetState()->Get( aeKey::E ) && !input.GetPrevState()->Get( aeKey::E ) )
			{
				s_operation = ImGuizmo::ROTATE;
			}
			else if ( input.GetState()->Get( aeKey::R ) && !input.GetPrevState()->Get( aeKey::R ) )
			{
				s_operation = ImGuizmo::SCALE;
			}

			if ( currentBox )
			{
				// Use ImGuizmo::IsUsing() to only update terrain when finished dragging
				aeFloat4x4 gizmoTransform = currentBox->GetAABB().GetTransform();
				if ( !gizmoClickedPrev && ImGuizmo::IsUsing() )
				{
					gizmoPrevTransform = gizmoTransform;
					gizmoClickedPrev = true;
				}

				gizmoTransform.SetTranspose();
				ImGuizmo::Manipulate( worldToView.GetTransposeCopy().data, viewToProj.GetTransposeCopy().data, s_operation, ImGuizmo::WORLD, gizmoTransform.data );
				gizmoTransform.SetTranspose();

				debug.AddCube( gizmoTransform, aeColor::Green() );

				if ( gizmoTransform != gizmoPrevTransform )
				{
					currentBox->SetTransform( gizmoTransform );

					aeFloat3 prevPos = gizmoPrevTransform.GetTranslation();
					aeFloat3 prevHalfSize = gizmoPrevTransform.GetScale() * 0.5f;

					aeAABB prevAABB( prevPos - prevHalfSize, prevPos + prevHalfSize );
					terrain->Dirty( prevAABB );
					terrain->Dirty( currentBox->GetAABB() );
				}
			}

			debug.Render( worldToProj );
			textRender.Render( textToNdc );

			ui->Render();

			render.EndFrame();
		}

		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	ui->Terminate();
	terrain->Terminate();
	aeAlloc::Release( terrain );
	input.Terminate();
	if ( !headless )
	{
		render.Terminate();
		window.Terminate();
	}
}
	return 0;
}
