#include "Common.h"

#ifdef EXAMPLE_CPP
int main() {
	// Initialize the window
	InitWindow(640, 480, "[C++] ace + lsqueezer");
	InitAudioDevice();

	{
		struct stat info;
		const char* path = EXAMPLE_DEFAULT_PATH "/" EXAMPLE_DEFAULT_NAME ".ace";
		if (stat(path, &info) != 0) {
#ifdef DEBUG
			// Prevent a crash in case no ace file was previously present.
			ace::Generate(EXAMPLE_DEFAULT_COMPRESSION, EXAMPLE_DEFAULT_RES_PATH, EXAMPLE_DEFAULT_PATH, EXAMPLE_DEFAULT_NAME);
#else
			// TODO: Add a different outcome here if you wish.
#endif
		}
	}

	// Initialize Ace
	ace::Init(EXAMPLE_DEFAULT_COMPRESSION, EXAMPLE_DEFAULT_RES_PATH, EXAMPLE_DEFAULT_PATH, EXAMPLE_DEFAULT_NAME, false);

	// Create explosion sound
	Sound explosion_sound;
	{
		ace_entry explosion = ace::LoadContent("explosion");
		Wave explosion_wave = LoadWaveFromMemory(explosion->type.c_str(), explosion->data, explosion->size);
		explosion_sound = LoadSoundFromWave(explosion_wave);
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
		const char* tags[] = { "d", "a", "c", "test", "b" }; // Tags/Content ids (out of order for testing)
		lsqueezer squeezer(EXAMPLE_ATLAS_W, EXAMPLE_ATLAS_H, true);	 // Atlas size & Verbosity

		// Generate the atlas image
		Image img = squeezer.RunTags(tags, 5);

		// Create texture from image
		atlas = LoadTextureFromImage(img);

		// Get content position inside the atlas
		a = squeezer["a"];
		b = squeezer["b"];
		c = squeezer["c"];
		d = squeezer["d"];
		test = squeezer["test"];

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
		DrawRectangleLines(0, 0, EXAMPLE_ATLAS_W, EXAMPLE_ATLAS_H, RED);

		DrawText(EXAMPLE_TEXT, (GetScreenWidth() - MeasureText(EXAMPLE_TEXT, 20)) / 2.0f, (GetScreenHeight() - 20) / 2, 20, GRAY);

		// Use atlas! :)
		EXAMPLE_DrawComponent(atlas, a, { 300, 0 }, 0, WHITE);
		EXAMPLE_DrawComponent(atlas, b, { 400, 100 }, 0, WHITE);
		EXAMPLE_DrawComponent(atlas, c, { 64, 250 }, 0, WHITE);
		EXAMPLE_DrawComponent(atlas, d, { 0, 400 }, 0, WHITE);
		EXAMPLE_DrawComponent(atlas, test, { 520, 300 }, 0, WHITE);
		EndDrawing();
	}

	// Close
	UnloadSound(explosion_sound);
	ace::Stop();
	CloseAudioDevice();
	CloseWindow();
}
#endif // EXAMPLE_CPP
