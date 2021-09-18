#pragma once
#include <lsqueezer/lesser_squeezer.h>
#include <deco.h>
#include <raylib.h>
#include <AtlasComponent.h>

/* Simply comment/uncomment this macro to switch between the C/C++ examples;
   Note: Deco & Lsqueezer in C have different syntax than in C++.
*/

#define _EX_CPP

// Utility macros

#define EX_DEFAULT_PATH "."
#define EX_DEFAULT_RES_PATH "res"
#define EX_DEFAULT_NAME "resources"
#define EX_DEFAULT_COMPRESSION 10

#define EX_ATLAS_W 256
#define EX_ATLAS_H 128

#define EX_TEXT "Press LMB to play explosion.wav"

// Convenience function
static inline void EX_DrawComponent(Texture2D atlas, AtlasComponent component, Vector2 position, float rotation, Color tint) {
	const Rectangle src = { (float)component.x, (float)component.y, (float)component.width, (float)component.height };
	const Rectangle dst = { (float)position.x, (float)position.y, (float)component.width, (float)component.height };
	const Vector2 offset = { component.x_offset, component.y_offset };
	DrawTexturePro(atlas, src, dst, offset, rotation, tint);
}