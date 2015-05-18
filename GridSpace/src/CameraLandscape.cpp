//
//  CameraLandscape.cpp
//
//  Created by Soso Limited on 5/18/15.
//
//

#include "CameraLandscape.h"

using namespace soso;
using namespace cinder;
using namespace std;

const double TAU = 6.28318530718;

void CameraLandscape::setup()
{
	struct Vertex {
		vec3 pos;
		vec2 tc;
	};

	auto shader = gl::getStockShader( gl::ShaderDef().texture( GL_TEXTURE_2D ).color() );
	auto mat = glm::translate( vec3( 20, 0, 20 ) ) * glm::scale( vec3( 20 ) ) * glm::rotate<float>( M_PI / 4, vec3( 0, 1, 0 ) );
	batch = gl::Batch::create( geom::Teapot().subdivisions( 8 ) >> geom::Transform( mat ), shader );

	vector<Vertex> vertices;
	const auto r_min = 1;
	const auto r_max = 21;
	const auto r_step = 1;
	const auto rings = r_max - r_min;
	const auto segments = 64;

	// vertices from inside to outside edge
	for( auto r = r_min; r <= r_max; r += r_step )
	{
		for( auto s = 0; s < segments; s += 1 )
		{
			auto t = (float) s / segments;
			auto x = cos( t * TAU ) * r;
			auto y = sin( t * TAU ) * r;
			auto pos = vec3( x, -5.0f, y );
			auto tc = vec2( 0.5 );

			// Mirror texture at halfway point
			tc.y = abs( t - 0.5f ) * 2.0f;
			tc.x = lmap<float>( r, r_min, r_max, 1.0f, 0.0f );

			vertices.push_back( { pos, tc } );
		}
	}

	vector<uint32_t> indices;
	// return a wrapped index in a row and segment
	auto index = [segments, rings] (int s, int r) {
		r *= segments;
		s %= segments;
		return r + s;
	};
	for( auto r = 0; r < rings; r += 1 )
	{
		for( auto s = 0; s < segments; s += 1 )
		{
			indices.push_back( index( s, r ) );
			indices.push_back( index( s + 1, r ) );
			indices.push_back( index( s + 1, r + 1 ) );

			indices.push_back( index( s, r ) );
			indices.push_back( index( s, r + 1 ) );
			indices.push_back( index( s + 1, r + 1 ) );
		}
	}

	auto vertex_vbo = gl::Vbo::create( GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW );
	auto index_vbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW );

	auto vertex_layout = geom::BufferLayout();
	vertex_layout.append( geom::Attrib::POSITION, 3, sizeof(Vertex), offsetof(Vertex, pos) );
	vertex_layout.append( geom::Attrib::TEX_COORD_0, 2, sizeof(Vertex), offsetof(Vertex, tc) );

	auto mesh = gl::VboMesh::create( vertices.size(), GL_TRIANGLES, {{ vertex_layout, vertex_vbo }}, indices.size(), GL_UNSIGNED_INT, index_vbo );
	batch = gl::Batch::create( mesh, shader );
}

void CameraLandscape::updateTexture( const ci::Surface &iImage )
{
	if( ! texture ) {
		texture = gl::Texture::create( iImage, gl::Texture::Format().loadTopDown() );
	}
	else {
		texture->update( iImage );
	}
}

void CameraLandscape::draw() const
{
	if( texture && batch )
	{
		gl::ScopedTextureBind tex0( texture, 0 );
		batch->draw();
	}
}