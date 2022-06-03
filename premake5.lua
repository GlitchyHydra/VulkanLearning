require "os"
local os = package.loaded['os']

workspace "Vulkan3dRenderer"
    architecture "x64"
    startproject "Vulkan3dRenderer"
    configurations {"Debug", "Release"}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

sdkpath = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "Vulkan3dRenderer/vendor/GLFW/include"
IncludeDir["glm"] = "Vulkan3dRenderer/vendor/glm"
IncludeDir["Vulkan"] = "%{sdkpath}/include"

group "Dependecies"
    include "Vulkan3dRenderer/vendor/GLFW"

group ""

project "Vulkan3dRenderer"
    location "Vulkan3dRenderer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp"
    }
    
    defines
	{
        "GLFW_INCLUDE_NONE"
	}

    libdirs
    {
        --"%{prj.name}/vendor/assimp"
        "%{sdkpath}/Lib",
        "%{prj.name}/vendor/glfw_precomp/lib-vc2022"
        --"%{sdkpath}/Lib/lib-vc2022"
	}

    includedirs
	{
		"%{prj.name}/src",
        --"%{IncludeDir.GLFW}",
        "%{prj.name}/vendor/glfw_precomp/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.Vulkan}"
    }
    
    links 
	{ 
        --"GLFW",
        "glfw3_mt",
        "vulkan-1",
        "VkLayer_utils"
    }
    
    filter "system:windows"
		systemversion "latest"


    filter "configurations:Debug"
        runtime "Debug"
		symbols "on"

        defines
        {
            "DEBUG_ALLOCATOR"
        }
       
        links
        {
        }

	filter "configurations:Release"
        runtime "Release"
		optimize "on"

        defines
        {

        }

        
        links
        {
        }

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"