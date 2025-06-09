workspace "Enhanced_Valencian_Engine"
  projects_names = {
    "PR00_Demos",
    "PR01_Shadows",
    "PR02_Audio"
  }

  language "C++"
  cppdialect "C++20"
  configurations { 
    "ReleasePC", "RelWithDebInfoPC", "DebugPC",
    "ReleaseXbox", "RelWithDebInfoXbox", "DebugXbox"
  }
--PC
  filter "configurations:DebugPC"
    include("deps/DebugPC/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "DEBUG", "RENDER_OPENGL" }
    symbols "On"
  filter{}
  filter "configurations:RelWithDebInfoPC"
    include("deps/RelWithDebInfoPC/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "DEBUG", "RENDER_OPENGL" }
    symbols "On"
    optimize "On"
  filter{}

  filter "configurations:ReleasePC"
    include("deps/ReleasePC/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "NDEBUG", "RENDER_OPENGL" }
    optimize "On"
  filter{}
--XBOX
  filter "configurations:DebugXbox"
    include("deps/DebugXbox/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "DEBUG", "RENDER_DIRECTX11" }
    symbols "On"
  filter{}
  filter "configurations:RelWithDebInfoXbox"
    include("deps/RelWithDebInfoXbox/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "DEBUG", "RENDER_DIRECTX11" }
    symbols "On"
    optimize "On"
  filter{}

  filter "configurations:ReleaseXbox"
    include("deps/ReleaseXbox/conanbuildinfo.premake.lua")
    conan_basic_setup()
    defines { "NDEBUG", "RENDER_DIRECTX11" }
    optimize "On"
  filter{}


--Compile each project with the linked static library and its configuration
for j, prj in ipairs(projects_names) do
  project(prj)
    kind "ConsoleApp"      
    location"build/"
    targetdir( path.join("build/", name))
    linkoptions { conan_exelinkflags }
		includedirs{ conan_includedirs }
		libdirs{ conan_libdirs }
		links{ conan_libs, conan_system_libs , conan_frameworks }
		defines{ conan_defines }
    dependson{"Enhanced_Valencian_Engine"}

    links{
      "compiledlibs/%{cfg.buildcfg}/Enhanced_Valencian_Engine"
    }
    if(prj == "PR00_Demos") then files{"tests/demos.cpp"}
    elseif(prj == "PR01_Shadows") then files{"tests/test_shadows.cpp"}
    elseif(prj == "PR02_Audio") then files{"tests/test_audio.cpp"}
    end

    includedirs {"./deps/","./code/**","./tests/"}
    files { 
      -- CPPs No longer needed since we use a static library now
        "code/headers/*.hpp",
        "code/headers/*.h",
        "deps/imgui/*.hpp",
        "deps/imgui/*.h"
    }
    
end

project("Enhanced_Valencian_Engine")
  kind "StaticLib"     
  location"./build/"
  targetdir "./compiledlibs/%{cfg.buildcfg}"
  includedirs {"./deps/","./code/**"}
  files{
    "code/source/**.cpp",
    "code/source/**.cc",
    "code/headers/**.hpp",
    "code/headers/**.h",
    "deps/imgui/*.*"
  }
  --It requires the CONAN added libraries to compile
  links{
    "d3d11",
	"d3dcompiler",
    "Xinput"
  }
  linkoptions { conan_sharedlinkflags }
	includedirs{ conan_includedirs }
	defines{ conan_defines }