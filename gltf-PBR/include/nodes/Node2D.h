/*
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "NodeGL.h"

namespace nodes {

// Basic support for 2D nodes
typedef std::shared_ptr<class Node2D> Node2DRef;

class Node2D : public NodeGL {
  public:
	Node2D( void );
	virtual ~Node2D( void );

	// getters and setters
	virtual ci::vec2 getPosition() const { return mPosition; }
	virtual void setPosition( float x, float y )
	{
		mPosition = ci::vec2( x, y );
		invalidateTransform();
	}
	virtual void setPosition( const ci::vec2 &pt )
	{
		mPosition = pt;
		invalidateTransform();
	}

	virtual ci::quat getRotation() const { return mRotation; }
	virtual void setRotation( float radians )
	{
		mRotation = glm::angleAxis( radians, ci::vec3( 0, 0, 1 ) );
		invalidateTransform();
	}
	virtual void setRotation( const ci::quat &rot )
	{
		mRotation = rot;
		invalidateTransform();
	}

	virtual ci::vec2 getScale() const { return mScale; }
	virtual void setScale( float scale )
	{
		mScale = ci::vec2( scale, scale );
		invalidateTransform();
	}
	virtual void setScale( float x, float y )
	{
		mScale = ci::vec2( x, y );
		invalidateTransform();
	}
	virtual void setScale( const ci::vec2 &scale )
	{
		mScale = scale;
		invalidateTransform();
	}

	virtual ci::vec2 getAnchor() const { return mAnchorIsPercentage ? mAnchor * getSize() : mAnchor; }
	virtual void setAnchor( float x, float y )
	{
		mAnchor = ci::vec2( x, y );
		mAnchorIsPercentage = false;
		invalidateTransform();
	}
	virtual void setAnchor( const ci::vec2 &pt )
	{
		mAnchor = pt;
		mAnchorIsPercentage = false;
		invalidateTransform();
	}

	virtual ci::vec2 getAnchorPercentage() const { return mAnchorIsPercentage ? mAnchor : mAnchor / getSize(); }
	virtual void setAnchorPercentage( float px, float py )
	{
		mAnchor = ci::vec2( px, py );
		mAnchorIsPercentage = true;
		invalidateTransform();
	}
	virtual void setAnchorPercentage( const ci::vec2 &pt )
	{
		mAnchor = pt;
		mAnchorIsPercentage = true;
		invalidateTransform();
	}

	//
	virtual float     getWidth() const { return mWidth; }
	virtual float     getScaledWidth() const { return mWidth * mScale.x; }
	virtual float     getHeight() const { return mHeight; }
	virtual float     getScaledHeight() const { return mHeight * mScale.y; }
	virtual ci::vec2  getSize() const { return ci::vec2( mWidth, mHeight ); }
	virtual ci::vec2  getScaledSize() const { return getSize() * mScale; }
	virtual ci::Rectf getBounds() const { return ci::Rectf( ci::vec2( 0 ), getSize() ); }
	virtual ci::Rectf getScaledBounds() const { return ci::Rectf( ci::vec2( 0 ), getScaledSize() ); }

	virtual void setWidth( float w ) { mWidth = w; }
	virtual void setHeight( float h ) { mHeight = h; }
	virtual void setSize( float w, float h )
	{
		mWidth = w;
		mHeight = h;
	}
	virtual void setSize( const ci::ivec2 &size )
	{
		mWidth = (float)size.x;
		mHeight = (float)size.y;
	}
	virtual void setBounds( const ci::Rectf &bounds )
	{
		mWidth = bounds.getWidth();
		mHeight = bounds.getHeight();
	}

	// conversions from screen to world to object coordinates and vice versa
	virtual ci::vec2 screenToParent( const ci::vec2 &pt ) const;
	virtual ci::vec2 screenToObject( const ci::vec2 &pt, float z = 0 ) const;
	virtual ci::vec2 parentToScreen( const ci::vec2 &pt ) const;
	virtual ci::vec2 parentToObject( const ci::vec2 &pt ) const;
	virtual ci::vec2 objectToParent( const ci::vec2 &pt ) const;
	virtual ci::vec2 objectToScreen( const ci::vec2 &pt ) const;

	// stream support
	virtual inline std::string toString() const { return "Node2D"; }
  protected:
	ci::vec2 mPosition;
	ci::quat mRotation;
	ci::vec2 mScale;
	ci::vec2 mAnchor;

	bool mAnchorIsPercentage;

	float mWidth;
	float mHeight;

	// required function (see: class Node)
	virtual void transform() const
	{
		// construct transformation matrix
		ci::mat4 transform = glm::translate( ci::vec3( mPosition, 0 ) );
		transform *= glm::toMat4( mRotation );
		transform *= glm::scale( ci::vec3( mScale, 1 ) );

		if( mAnchorIsPercentage )
			transform *= glm::translate( ci::vec3( -mAnchor * getSize(), 0 ) );
		else
			transform *= glm::translate( ci::vec3( -mAnchor, 0 ) );

		setTransform( transform );
	}
};

} // namespace nodes