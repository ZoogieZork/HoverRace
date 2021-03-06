// Shapes.h
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//

//
// This files contains the 2.5D shapes used in the game
// Theses shapes are used to calculate contacts between objects
//
// Most of the members of theses class are public
//
// Serialisation will not and shound not be added to theses class
// Theses objects should be build at run time for collision detection only
//

#pragma once

#include "../Util/WorldCoordinates.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace Model {

class MR_DllDeclare ShapeInterface
{
	public:
		virtual ~ShapeInterface() { }

		enum eShape { eCylinder = 0, eLineSegment = 1, ePolygon = 2 };

		// Shape position
		virtual MR_Int32 XPos() const = 0;
		virtual MR_Int32 YPos() const = 0;

		// Basic bonding attributes (Bonding box)
		virtual MR_Int32 XMin() const = 0;
		virtual MR_Int32 XMax() const = 0;
		virtual MR_Int32 YMin() const = 0;
		virtual MR_Int32 YMax() const = 0;

		virtual MR_Int32 ZMin() const = 0;
		virtual MR_Int32 ZMax() const = 0;

		virtual eShape ShapeType() const = 0;
};

// List of availlable shapes

class MR_DllDeclare CylinderShape : public ShapeInterface
{
	public:
		virtual MR_Int32 AxisX() const = 0;
		virtual MR_Int32 AxisY() const = 0;
		virtual MR_Int32 RayLen() const = 0;

		// Pre overloaded
		MR_Int32 XPos() const;
		MR_Int32 YPos() const;

		MR_Int32 XMin() const;
		MR_Int32 XMax() const;
		MR_Int32 YMin() const;
		MR_Int32 YMax() const;

		eShape ShapeType() const;

};

class MR_DllDeclare LineSegmentShape : public ShapeInterface
{
	public:

		virtual MR_Int32 X0() const = 0;
		virtual MR_Int32 Y0() const = 0;
		virtual MR_Int32 Z0() const = 0;

		virtual MR_Int32 X1() const = 0;
		virtual MR_Int32 Y1() const = 0;
		virtual MR_Int32 Z1() const = 0;

		virtual MR_Int32 HorizontalLen() const = 0;

		// Pre overloaded
		MR_Int32 XPos() const;
		MR_Int32 YPos() const;

		MR_Int32 XMin() const;
		MR_Int32 XMax() const;
		MR_Int32 YMin() const;
		MR_Int32 YMax() const;

		eShape ShapeType() const;

};

class MR_DllDeclare PolygonShape : public ShapeInterface
{
	public:
		virtual int VertexCount() const = 0;
		virtual MR_Int32 X(int pIndex) const = 0;
		virtual MR_Int32 Y(int pIndex) const = 0;
		virtual MR_Int32 SideLen(int pIndex) const = 0;

		// Pre overloaded
		MR_Int32 XPos() const;
		MR_Int32 YPos() const;

		eShape ShapeType() const;

};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
