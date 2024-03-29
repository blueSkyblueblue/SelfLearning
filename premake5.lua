-- Solution Lines
workspace "Lines"
	architecture "x64" -- Only support x86_64 windows operating system
	startproject "Lines" -- start project will be Lines, and other project will be linked to it.

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Dependencies/GLFW/include"
IncludeDir["Glad"] = "Dependencies/Glad/include"


group "Dependencies"
	include "Dependencies/GLFW"
	include "Dependencies/Glad"
group ""

project "Lines"
	location "Lines"
	kind "ConsoleApp"
	language "C++"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	defines 
	{
		"GLFW_INCLUDE_NONE",
		"SPDLOG_COMPILED_LIB"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/spdlog/**.h",
		"%{prj.name}/vendor/spdlog/**.cpp",
		"%{prj.name}/vendor/glm/**.h",
		"%{prj.name}/vendor/glm/**.hpp",
		"%{prj.name}/vendor/glm/**.inl",
		"%{prj.name}/vendor/imgui/**.h",
		"%{prj.name}/vendor/imgui/**.cpp",
	}

	removefiles
	{
		"%{prj.name}/vendor/imgui/imgui/main.cpp",
	}

	includedirs
	{
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/glm",
		"%{prj.name}/vendor/imgui"
	}

	links 
	{
		"GLFW",
		"Glad",
		"opengl32.lib"
	}

	filter "system:Windows"
		cppdialect "C++20"
		staticruntime "Off"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Dist"
		optimize "On"
