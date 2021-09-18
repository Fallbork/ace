workspace "deco"
	configurations { "Debug", "Release" }
	platforms { "x64", "x86" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		
	filter { "platforms:x86" }
		architecture "x86"

	filter { "platforms:x64" }
		architecture "x86_64"
	
	filter{}
		defines{ "PLATFORM_DESKTOP", "GRAPHICS_API_OPENGL_21" }
		
project "raylib"
		filter "configurations:Debug OR Release"
			kind "StaticLib"
			
		filter "action:vs*"
			defines{ "_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "_WIN32" }
			links { "winmm" }
			
		filter "action:gmake*"
			links { "pthread", "GL", "m", "dl", "rt", "X11" }
			
		filter{}
		
		language "C++"
		targetdir "Addons/raylib/build/%{cfg.buildcfg}/%{cfg.platform}"
		cppdialect "C++17"
		
		files { "Addons/raylib/src/*.h", "Addons/raylib/src/*.c" }
		vpaths {
			["Header Files"] = { "Addons/raylib/src/**.h" },
			["Source Files/*"] = { "Addons/raylib/src/**.c" },
		}

		includedirs { "Addons/raylib/src", "Addons/raylib/src/external/glfw/include" }

project "deco"
		filter "configurations:Debug OR Release"
			kind "StaticLib"
			
		filter "action:gmake*"
			links { "pthread", "GL", "m", "dl", "rt", "X11" }
			
		filter{}
		
		language "C++"
		targetdir "src/build/%{cfg.buildcfg}/%{cfg.platform}"
		cppdialect "C++17"
		
		files { "src/**.h", "src/**.c", "src/**.cpp", "addons/zstd/lib/**.h", "addons/zstd/lib/**.c",
				"addons/zstd/programs/util.*", "addons/zstd/programs/timefn.*",
				"addons/zstd/programs/platform.*", "addons/zstd/lib/common/*.*" }
		removefiles { "addons/zstd/programs/dibio.*" }
		vpaths {
			["Header Files"] = { "**.h" },
			["Source Files/*"] = { "**.c", "**.cpp" },
		}

		includedirs { "addons/zstd/lib", "addons/zstd/programs", "addons/raylib/src" }

project "example-app"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/%{cfg.platform}"
	cppdialect "C++17"

	-- Files included in the solution
	files { "example/**.c", "example/**.cpp", "example/**.h" }
	vpaths {
		["Header Files"] = { "**.h" },
		["Source Files"] = { "**.c", "**.cpp" },
	}

	links { "raylib", "deco" }
	
	-- Additional include >>directories<< - easier access!
	includedirs { "addons/zstd/lib", "addons/zstd/programs", "addons/raylib/src", "src" }
	defines{ "PLATFORM_DESKTOP", "GRAPHICS_API_OPENGL_21" }

	filter "action:vs*"
		defines{ "_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS", "_WIN32" }
		dependson { "raylib", "deco" }
		links { "raylib.lib", "winmm", "kernel32", "deco.lib" }
		libdirs { "addons/raylib/build/%{cfg.buildcfg}/%{cfg.platform}", "src/build/%{cfg.buildcfg}/%{cfg.platform}" }

	filter "action:gmake*"
		links { "pthread", "GL", "m", "dl", "rt", "X11" }		