#include "Common.h"
#include <sys/stat.h>

#ifndef _EX_CPP
int main(void) {
	// Initialize the window
	InitWindow(640, 480, "[C] deco + lsqueezer");
	InitAudioDevice();

	{
		struct stat info;
		const char* path = EX_DEFAULT_PATH "/" EX_DEFAULT_NAME ".deco";
		if (stat(path, &info) != 0) {
#ifdef DEBUG
			// Prevent a crash in case no deco file was previously present.
			Deco_Generate(EX_DEFAULT_COMPRESSION, EX_DEFAULT_RES_PATH, EX_DEFAULT_PATH, EX_DEFAULT_NAME);
#else
			// TODO: Add a different outcome here if you wish.
#endif
		}
	}

	// Initialize Deco
	Deco_Init(EX_DEFAULT_COMPRESSION, EX_DEFAULT_PATH, EX_DEFAULT_NAME);

	// Create explosion sound
	Sound explosion_sound;
	{
		deco_entry* explosion = Deco_LoadContent("explosion");
		Wave explosion_wave = LoadWaveFromMemory(explosion->type, explosion->data, explosion->size);
		explosion_sound = LoadSoundFromWave(explosion_wave);
		Deco_FreeEntry(explosion);
		UnloadWave(explosion_wave);
	}

	/* Run lsqueezer;
	   Optional: Create AtlasComponent variables outside lsqueezer so the
	   object can be destroyed. Not necessary, but not having to use the
	   [] operator makes life a lot easier in my opinion.
	*/
	Texture2D atlas;

	AtlasComponent a;	 // Optional
	AtlasComponent b;	 // Optional
	AtlasComponent c;	 // Optional
	AtlasComponent d;	 // Optional
	AtlasComponent test; // Optional

	{
		const char* tags[] = { "a", "b", "c", "d", "test" }; // Tags/Content ids
		LS_Init(EX_ATLAS_W, EX_ATLAS_H, true);		 // Atlas size & Verbosity

		// Generate the atlas image
		Image img = LS_RunTags(tags, 5);

		// Create texture from image
		atlas = LoadTextureFromImage(img);

		// Get content position inside the atlas
		a = LS_GetComponent("a");
		b = LS_GetComponent("b");
		c = LS_GetComponent("c");
		d = LS_GetComponent("d");
		test = LS_GetComponent("test");

		// Unloads the atlas image (can be done as soon as the rexture is created)
		UnloadImage(img);
	}

	// Main loop
	while (!WindowShouldClose()) {
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			PlaySound(explosion_sound);
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawTexture(atlas, 0, 0, WHITE);
		DrawRectangleLines(0, 0, EX_ATLAS_W, EX_ATLAS_H, RED);

		DrawText(EX_TEXT, (GetScreenWidth() - MeasureText(EX_TEXT, 20)) / 2.0f, (GetScreenHeight() - 20) / 2, 20, GRAY);

		// Use atlas! :)
		EX_DrawComponent(atlas, a, (Vector2){ 300, 0 }, 0, WHITE);
		EX_DrawComponent(atlas, b, (Vector2){ 400, 100 }, 0, WHITE);
		EX_DrawComponent(atlas, c, (Vector2){ 64, 250 }, 0, WHITE);
		EX_DrawComponent(atlas, d, (Vector2){ 0, 400 }, 0, WHITE);
		EX_DrawComponent(atlas, test, (Vector2){ 520, 300 }, 0, WHITE);
		EndDrawing();
	}

	// Close
	UnloadSound(explosion_sound);
	LS_Stop();
	Deco_Stop();
	CloseAudioDevice();
	CloseWindow();
}
#endif // _EX_CPP
