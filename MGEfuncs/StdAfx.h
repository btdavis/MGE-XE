#pragma once

#define SAFERELEASEP(a) { if(a) { delete[] a; } a = 0; }

#include <assert.h>
#include <fstream>
#include <strstream>
#include <vector>
#include <float.h>
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d9.h>
#include <d3dx9.h>

#include "niflib.h"
#include "nif_math.h"
#include "obj/NiObject.h"
#include "obj/NiAVObject.h"
#include "obj/NiNode.h"
#include "obj/NiSwitchNode.h"
#include "obj/NiProperty.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiBinaryExtraData.h"
#include "obj/NiTriStripsData.h"
#include "obj/RootCollisionNode.h"

#include "tootlelib.h"
