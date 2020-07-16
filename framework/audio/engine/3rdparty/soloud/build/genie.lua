local WITH_SDL = 0
local WITH_SDL2 = 0
local WITH_SDL_STATIC = 0
local WITH_SDL2_STATIC = 0
local WITH_PORTAUDIO = 0
local WITH_OPENAL = 0
local WITH_XAUDIO2 = 0
local WITH_WINMM = 0
local WITH_WASAPI = 0
local WITH_ALSA = 0
local WITH_JACK = 0
local WITH_OSS = 0
local WITH_COREAUDIO = 0
local WITH_VITA_HOMEBREW = 0
local WITH_NOSOUND = 0
local WITH_MINIAUDIO = 0
local WITH_NULL = 1
local WITH_TOOLS = 0

if (os.is("Windows")) then
	WITH_WINMM = 1
elseif (os.is("macosx")) then
	WITH_COREAUDIO = 1
else
	WITH_ALSA = 1
	WITH_OSS = 1
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

local sdl_root       = "/libraries/sdl"
local sdl2_root      = "/libraries/sdl2"
local dxsdk_root     = os.getenv("DXSDK_DIR") and os.getenv("DXSDK_DIR") or "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)"
local portaudio_root = "/libraries/portaudio"
local openal_root    = "/libraries/openal"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

local sdl_include       = sdl_root .. "/include"
local sdl2_include      = sdl2_root .. "/include"
local sdl2_lib_x86      = sdl2_root .. "/lib/x86"
local sdl2_lib_x64      = sdl2_root .. "/lib/x64"
local dxsdk_include     = dxsdk_root .. "/include"
local portaudio_include = portaudio_root .. "/include"
local openal_include    = openal_root .. "/include"

local buildroot = ""
if _ACTION then buildroot = _ACTION end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

newoption {
    trigger       = "with-common-backends",
    description   = "Includes common backends in build"
}

newoption {
	trigger		  = "with-openal",
	description = "Include OpenAL backend in build"
}

newoption {
	trigger		  = "with-sdl",
	description = "Include SDL backend in build"
}

newoption {
	trigger		  = "with-sdl2",
	description = "Include SDL2 backend in build"
}

newoption {
	trigger		  = "with-portaudio",
	description = "Include PortAudio backend in build"
}

newoption {
	trigger		  = "with-wasapi",
	description = "Include WASAPI backend in build"
}

newoption {
	trigger		  = "with-xaudio2",
	description = "Include XAudio2 backend in build"
}

newoption {
	trigger		  = "with-native-only",
	description = "Only native backends (winmm/oss) in build (default)"
}

newoption {
	trigger		  = "with-sdl-only",
	description = "Only include sdl in build"
}

newoption {
	trigger		  = "with-sdlstatic-only",
	description = "Only include sdl that doesn't use dyndll in build"
}

newoption {
	trigger		  = "with-sdl2-only",
	description = "Only include sdl2 in build"
}

newoption {
	trigger		  = "with-sdl2static-only",
	description = "Only include sdl2 that doesn't use dyndll in build"
}

newoption {
	trigger		  = "with-coreaudio",
	description = "Include OS X CoreAudio backend in build"
}

newoption {
	trigger		  = "with-vita-homebrew-only",
	description = "Only include PS Vita homebrew backend in build"
}

newoption {
	trigger		  = "with-tools",
	description = "Include (optional) tools in build"
}

newoption {
	trigger		  = "soloud-devel",
	description = "Shorthand for options used while developing SoLoud"
}

newoption {
	trigger		  = "with-nosound",
	description = "Include nosound backend in build"
}

newoption {
	trigger		  = "with-jack",
	description = "Include JACK backend in build"
}

newoption {
	trigger		  = "with-jack-only",
	description = "Only include JACK backend in build"
}

newoption {
    trigger       = "with-miniaudio",
    description = "Include MiniAudio in build" 
}

newoption {
    trigger       = "with-miniaudio-only",
    description = "Only include MiniAudio in build"
}

if _OPTIONS["soloud-devel"] then
    WITH_SDL = 0
    WITH_SDL2 = 1
    WITH_SDL_STATIC = 0
    WITH_SDL2_STATIC = 0
    WITH_PORTAUDIO = 1
    WITH_OPENAL = 1
    WITH_XAUDIO2 = 0
    WITH_WINMM = 0
    WITH_WASAPI = 0
    WITH_MINIAUDIO = 1
    WITH_OSS = 1
    WITH_NOSOUND = 1
    if (os.is("Windows")) then
    	WITH_XAUDIO2 = 0
    	WITH_WINMM = 1
    	WITH_WASAPI = 1
    	WITH_OSS = 0
    end
    WITH_TOOLS = 1
end

if _OPTIONS["with-common-backends"] then
    WITH_SDL = 1
    WITH_SDL_STATIC = 0
    WITH_SDL2_STATIC = 0
    WITH_PORTAUDIO = 1
    WITH_OPENAL = 1
    WITH_XAUDIO2 = 0
    WITH_WINMM = 0
    WITH_WASAPI = 0
    WITH_OSS = 1
    WITH_NOSOUND = 1
    WITH_MINIAUDIO = 0

    if (os.is("Windows")) then
    	WITH_XAUDIO2 = 0
    	WITH_WINMM = 1
    	WITH_WASAPI = 1
    	WITH_OSS = 0
    end
end

if _OPTIONS["with-xaudio2"] then
	WITH_XAUDIO2 = 1
end

if _OPTIONS["with-openal"] then
	WITH_OPENAL = 1
end

if _OPTIONS["with-portaudio"] then
	WITH_PORTAUDIO = 1
end

if _OPTIONS["with-coreaudio"] then
	WITH_COREAUDIO = 1
end

if _OPTIONS["with-sdl"] then
	WITH_SDL = 1
end

if _OPTIONS["with-sdl2"] then
	WITH_SDL2 = 1
end

if _OPTIONS["with-wasapi"] then
	WITH_WASAPI = 1
end

if _OPTIONS["with-nosound"] then
    WITH_NOSOUND = 1
end

if _OPTIONS["with-sdl-only"] then
	WITH_SDL = 1
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-sdl2-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 1
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-sdlstatic-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 1
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-sdl2static-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 1
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-sdl2static-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 1
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-vita-homebrew-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_ALSA = 0
	WITH_VITA_HOMEBREW = 1
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0

	premake.gcc.cc = "arm-vita-eabi-gcc"
	premake.gcc.cxx = "arm-vita-eabi-g++"
	premake.gcc.ar = "arm-vita-eabi-ar"
end

if _OPTIONS["with-jack"] then
	WITH_JACK = 1
end

if _OPTIONS["with-jack-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_ALSA = 0
	WITH_VITA_HOMEBREW = 0
	WITH_COREAUDIO = 0
	WITH_JACK = 1
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 0
end

if _OPTIONS["with-miniaudio"] then
    WITH_MINIAUDIO = 1
end

if _OPTIONS["with-miniaudio-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_ALSA = 0
	WITH_VITA_HOMEBREW = 0
	WITH_COREAUDIO = 0
	WITH_JACK = 0
	WITH_NOSOUND = 0
	WITH_MINIAUDIO = 1
end

if _OPTIONS["with-native-only"] then
	WITH_SDL = 0
	WITH_SDL2 = 0
	WITH_SDL_STATIC = 0
	WITH_SDL2_STATIC = 0
	WITH_PORTAUDIO = 0
	WITH_OPENAL = 0
	WITH_XAUDIO2 = 0
	WITH_WINMM = 0
	WITH_WASAPI = 0
	WITH_OSS = 0
	WITH_MINIAUDIO = 0
	WITH_NOSOUND = 0
	if (os.is("Windows")) then
		WITH_WINMM = 1
	elseif (os.is("macosx")) then
		WITH_COREAUDIO = 1
	else
	  WITH_OSS = 1
	end
end

if _OPTIONS["with-tools"] then
	WITH_TOOLS = 1
end

print ("")
print ("Active options:")
print ("WITH_SDL        = ", WITH_SDL)
print ("WITH_SDL2       = ", WITH_SDL2)
print ("WITH_PORTAUDIO  = ", WITH_PORTAUDIO)
print ("WITH_OPENAL     = ", WITH_OPENAL)
print ("WITH_XAUDIO2    = ", WITH_XAUDIO2)
print ("WITH_WINMM      = ", WITH_WINMM)
print ("WITH_WASAPI     = ", WITH_WASAPI)
print ("WITH_ALSA       = ", WITH_ALSA)
print ("WITH_JACK       = ", WITH_JACK)
print ("WITH_OSS        = ", WITH_OSS)
print ("WITH_MINIAUDIO  = ", WITH_MINIAUDIO)
print ("WITH_NOSOUND    = ", WITH_NOSOUND)
print ("WITH_COREAUDIO  = ", WITH_COREAUDIO)
print ("WITH_VITA_HOMEBREW = ", WITH_VITA_HOMEBREW)
print ("WITH_TOOLS      = ", WITH_TOOLS)
print ("")


-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

solution "SoLoud"
  location(buildroot)
	configurations { "Debug", "Release" }
	startproject "simplest"	
	targetdir "../bin"
	debugdir "../bin"
	flags { "NoExceptions", "NoRTTI", "NoPCH" }
    if (os.is("Windows")) then flags {"StaticRuntime"} end
	if (os.is("Windows")) then defines { "_CRT_SECURE_NO_WARNINGS" } end
    configuration { "x32", "Debug" }
        targetsuffix "_x86_d"   
    configuration { "x32", "Release" }
		flags {	"EnableSSE2" }
        targetsuffix "_x86"
    configuration { "x64", "Debug" }
        targetsuffix "_x64_d"    
    configuration { "x64", "Release" }
        targetsuffix "_x64"
    configuration { "Release" }
    	flags { "Optimize", "OptimizeSpeed", "NoEditAndContinue", "No64BitChecks" }   
		defines { "NDEBUG" }
		objdir (buildroot .. "/release")
    configuration { "Debug" }
		flags {"Symbols" }
		defines { "DEBUG" }
		objdir (buildroot .. "/debug")
	
	-- Enable SSE4.1 when using gmake + gcc.
	-- TODO: SoLoud could do with some better platform determination. genie
	--       doesn't do this well on it's own and is recommended to setup this
	--       manually. See https://github.com/bkaradzic/bx/blob/master/scripts/toolchain.lua
if (WITH_VITA_HOMEBREW == 0) then
	configuration { "gmake" }
		buildoptions { 
			"-msse4.1", 
			"-fPIC"
		}
end

    configuration {}

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "simplest"
	kind "ConsoleApp"
	language "C++"
	files {
	  "../demos/simplest/**.c*"
	  }
	includedirs {
	  "../include"
	}
if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end

		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		end

		targetname "simplest"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "welcome"
	kind "ConsoleApp"
	language "C++"
	files {
	  "../demos/welcome/**.c*"
	  }
	includedirs {
	  "../include"
	}
if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end

		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
        end

		targetname "welcome"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "null"
	kind "ConsoleApp"
	language "C++"
	files {
	  "../demos/null/**.c*"
	  }
	includedirs {
	  "../include"
	}
if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end


		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		end

		targetname "null"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "enumerate"
	kind "ConsoleApp"
	language "C++"
	files {
	  "../demos/enumerate/**.c*"
	  }
	includedirs {
	  "../include"
	}
if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end

		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		end

		targetname "enumerate"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

if (WITH_SDL2 == 1 or WITH_SDL2STATIC) then

	project "SoloudDemoCommon"
		kind "StaticLib"
		targetdir "../lib"
		language "C++"

	files {
	  "../demos/common/**.c*",
	  "../demos/common/imgui/**.c*",
	  "../demos/common/glew/GL/**.c*"
	  }
	includedirs {
	  "../include",
	  "../demos/common",
	  "../demos/common/imgui",
	  "../demos/common/glew",
	  sdl2_include
	}
	defines { "GLEW_STATIC" }

		targetname "solouddemocommon"
end
-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

	project "SoloudStatic"
		kind "StaticLib"
		targetdir "../lib"
		language "C++"

		files
		{
		"../src/audiosource/**.c*",
		"../src/filter/**.c*",
		"../src/core/**.c*"
	  }

		includedirs
		{
		  "../src/**",
	  "../include"
		}
if (WITH_OPENAL == 1) then
	defines {"WITH_OPENAL"}
	files {
	  "../src/backend/openal/**.c*"
	  }
	includedirs {
	  "../include",
	  openal_include
	}
end

if (WITH_ALSA == 1) then
	defines {"WITH_ALSA"}
	files {
	  "../src/backend/alsa/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_OSS == 1) then
	defines {"WITH_OSS"}
	files {
	  "../src/backend/oss/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_MINIAUDIO == 1) then
	defines {"WITH_MINIAUDIO"}
	files {
	  "../src/backend/miniaudio/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_NOSOUND == 1) then
	defines {"WITH_NOSOUND"}
	files {
	  "../src/backend/nosound/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_COREAUDIO == 1) then
	defines {"WITH_COREAUDIO"}
	files {
	  "../src/backend/coreaudio/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_PORTAUDIO == 1) then
	defines {"WITH_PORTAUDIO"}

	files {
	  "../src/backend/portaudio/**.c*"
	  }
	includedirs {
	  "../include",
	  portaudio_include
	}
end

if (WITH_SDL == 1) then
		defines { "WITH_SDL" }
	files {
	  "../src/backend/sdl/**.c*"
	  }
	includedirs {
	  "../include",
	  sdl2_include
	}
end

if (WITH_SDL2 == 1) then
		defines { "WITH_SDL2" }
	files {
	  "../src/backend/sdl/**.c*"
	  }
	includedirs {
	  "../include",
	  sdl2_include
	}
end

if (WITH_SDL_STATIC == 1) then
		defines { "WITH_SDL_STATIC" }
	files {
	  "../src/backend/sdl_static/**.c*"
	  }
	includedirs {
	  "../include",
	  sdl_include
	}
end

if (WITH_SDL2_STATIC == 1) then
		defines { "WITH_SDL2_STATIC" }
	files {
	  "../src/backend/sdl2_static/**.c*"
	  }
	includedirs {
	  "../include",
	  sdl2_include
	}
end

if (WITH_WASAPI == 1) then
		defines { "WITH_WASAPI" }
	files {
	  "../src/backend/wasapi/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_XAUDIO2 == 1) then
	defines {"WITH_XAUDIO2"}
	files {
	  "../src/backend/xaudio2/**.c*"
	  }
	includedirs {
	  "../include",
	  dxsdk_include
	}
end

if (WITH_WINMM == 1) then
		defines { "WITH_WINMM" }
	files {
	  "../src/backend/winmm/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_VITA_HOMEBREW == 1) then
		defines { "WITH_VITA_HOMEBREW", "usleep=sceKernelDelayThread" }
	files {
	  "../src/backend/vita_homebrew/**.c*"
	  }
	includedirs {
	  "../include"
	}
end


if (WITH_JACK == 1) then
	defines { "WITH_JACK" }
	links { "jack" }
	files {
	  "../src/backend/jack/**.c*"
	  }
	includedirs {
	  "../include"
	}
end

if (WITH_NULL == 1) then
    defines { "WITH_NULL" }
	files {
	  "../src/backend/null/**.c*"
	  }
	includedirs {
	  "../include"
	}
end    

		targetname "soloud_static"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --
if (WITH_TOOLS == 1) then

	project "sanity"
		kind "ConsoleApp"
		language "C++"
		includedirs {
		  "../include"
		}
		files {
		  "../src/tools/sanity/**.cpp"
		}
		if (WITH_ALSA == 1) then
			links {"asound"}
		end
		if (WITH_JACK == 1) then
			links { "jack" }
		end
		if (WITH_COREAUDIO == 1) then
			links {"AudioToolbox.framework"}
		end


		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		end

		targetname "sanity"
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --
if (WITH_TOOLS == 1) then

	project "codegen"
		kind "ConsoleApp"
		language "C++"
		files {
		  "../src/tools/codegen/**.cpp"
		}
		targetname "codegen"
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

if (WITH_TOOLS == 1) then

	project "tedsid2dump"
		kind "ConsoleApp"
		language "C++"
		files {
		  "../src/tools/tedsid2dump/**.cpp"
		}
		targetname "tedsid2dump"
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

if (WITH_TOOLS == 1) then

	project "resamplerlab"
		kind "ConsoleApp"
		language "C++"
		files {
		  "../src/tools/resamplerlab/**.c*"
		}

		targetname "resamplerlab"
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

if (WITH_TOOLS == 1) then

	project "lutgen"
		kind "ConsoleApp"
		language "C++"
		files {
		  "../src/tools/lutgen/**.c*"
		}

		targetname "lutgen"
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "c_test"
	kind "ConsoleApp"
	language "C++"
	files {
	  "../demos/c_test/**.c*",
	  "../src/c_api/soloud_c.cpp"
	  }
	includedirs {
	  "../include"
	}

		links {"SoloudStatic"}
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		end
if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end


		targetname "c_test"

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

	project "SoloudDynamic"
		kind "SharedLib"
		targetdir "../lib"
		language "C++"
		files
		{
		  "../src/c_api/**.c*"
		}

		includedirs
		{
		  "../src/**",
		  "../include"
		}

		links {"SoloudStatic"}

if (os.is("Windows")) then
	linkoptions { "/DEF:\"../../src/c_api/soloud.def\"" }
end

		targetname "soloud"
		implibdir("../lib")
		implibname("soloud")

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

--
--  The rest of the projects require SDL
--

if (WITH_SDL2 == 1 or WITH_SDL2STATIC) then

function sdl2_lib()
    configuration { "x32" } 
        libdirs { sdl2_lib_x86 }
    configuration { "x64" } 
        libdirs { sdl2_lib_x64 }
    configuration {}
end

function CommonDemo(_name)
  project(_name)
	kind "WindowedApp"
	language "C++"
	files {
	  "../demos/" .. _name .. "/**.c*"
	  }
	includedirs {
	  "../include",
	  "../demos/common",
	  "../demos/common/imgui",
	  "../demos/common/glew",
	  sdl2_include
	}
	sdl2_lib()

	defines { "GLEW_STATIC" }

if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end

		links {"SoloudStatic", "SoloudDemoCommon", "SDL2main", "SDL2"}
if (os.is("Windows")) then
        links {"opengl32"}
end
		if (not os.is("windows")) then
		  links { "pthread" }
		  links { "dl" }
		  links { "GL" }
		end

		targetname (_name)
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  CommonDemo("megademo")

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

  project "piano"
	kind "WindowedApp"
	language "C++"
	files {
	  "../demos/piano/**.c*",
	  "../demos/common/**.c*",
	  "../demos/common/imgui/**.c*",
	  "../demos/common/glew/GL/**.c*"
	  }
	includedirs {
	  "../include",
	  "../demos/common",
	  "../demos/common/imgui",
	  "../demos/common/glew",
	  sdl2_include
	}
    sdl2_lib()
    
	defines { "GLEW_STATIC" }	

if (WITH_ALSA == 1) then
	links {"asound"}
end
if (WITH_JACK == 1) then
	links { "jack" }
end
if (WITH_COREAUDIO == 1) then
	links {"AudioToolbox.framework"}
end

		links {"SoloudStatic", "SDL2main", "SDL2"}
if (os.is("Windows")) then
        links {"opengl32"}
        defines {"__WINDOWS_MM__"}
end
		if (not os.is("windows")) then
		  defines { "__LINUX_ALSA__"}
		  links { "pthread" }
		  links { "dl" }
		  links { "GL" }
		end

		targetname "piano"


-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --

    CommonDemo("env")

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --
    
end

-- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< -- 8< --
