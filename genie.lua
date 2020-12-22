-- Solution Config --
solution "VulkanTest"
	location "build"
	configurations { "Debug", "Release"	}
	platforms {"x32", "x64", "Native"}
	language "C++"
  startproject "VulkanTestProject"

	configuration "Debug"
		defines { "__DEBUG__" }
		flags { "Symbols", "NoPCH" }
		targetdir "bin/Debug"
		objdir "bin/Debug"

	configuration "Release"
    defines { "__RELEASE__" }
    flags { "Optimize", "NoPCH" }
    targetdir "bin/Release"
		objdir "bin/Release"
		
-- End Solution Config --

-- Project Config --


project "VulkanTestProject"
  location "build/projects"
  kind "ConsoleApp"

	includedirs {
		"./include/",
		"./deps/glfw/include/",
		"./deps/GLM/",
		"./deps/sokol/",
		"./deps/stb/",
    "./deps/vulkan/Include",

	}

	configuration "vs*"
		defines{
			"_GLFW_WIN32",
			"_GLFW_WGL",
		}

    libdirs{
      "./deps/vulkan/Lib32",  
    }

    links{
      "vulkan-1",
    }

		files {
			-- Project files --

			"include/**.h",
			"src/**.cpp",
			"src/**.h",


			-- GLFW files --
			"./deps/glfw/src/internal.h",
			"./deps/glfw/src/mappings.h",
			"./deps/glfw/src/context.c",
			"./deps/glfw/src/init.c",
			"./deps/glfw/src/input.c",
			"./deps/glfw/src/monitor.c",
			"./deps/glfw/src/vulkan.c",
			"./deps/glfw/src/window.c",
			"./deps/glfw/src/win32_platform.h",
			"./deps/glfw/src/win32_joystick.h",
			"./deps/glfw/src/wgl_context.h",
			"./deps/glfw/src/egl_context.h",
			"./deps/glfw/src/osmesa_context.h",
			"./deps/glfw/src/win32_init.c",
			"./deps/glfw/src/win32_joystick.c",
			"./deps/glfw/src/win32_monitor.c",
			"./deps/glfw/src/win32_time.c",
			"./deps/glfw/src/win32_thread.c",
			"./deps/glfw/src/win32_window.c",
			"./deps/glfw/src/wgl_context.c",
			"./deps/glfw/src/egl_context.c",
			"./deps/glfw/src/osmesa_context.c",
			"./deps/glfw/include/**.h",


			--GLM files --
			"./deps/GLM/glm/*.cpp",
			--"./deps/GLM/glm/*.inl",
			"./deps/GLM/glm/*.hpp",

			"./deps/GLM/glm/ext/*.cpp",
			--"./deps/GLM/glm/ext/*.inl",
			"./deps/GLM/glm/ext/*.hpp",

			"./deps/GLM/glm/detail/*.cpp",
			--"./deps/GLM/glm/detail/*.inl",
			"./deps/GLM/glm/detail/*.hpp",

			"./deps/GLM/glm/gtc/*.cpp",
			--"./deps/GLM/glm/gtc/*.inl",
			"./deps/GLM/glm/gtc/*.hpp",

			"./deps/GLM/glm/gtx/*.cpp",
			--"./deps/GLM/glm/gtx/*.inl",
			"./deps/GLM/glm/gtx/*.hpp",

			"./deps/GLM/glm/simd/*.cpp",
			--"./deps/GLM/glm/simd/*.inl",
			"./deps/GLM/glm/simd/*.h",


			-- stb files --
			"./deps/stb/stb_image.h",

		}

    -- Windows targets --
    configuration "vs2015"
		  windowstargetplatformversion "8.1"

		configuration "vs2017"
			--windowstargetplatformversion "10.0.15063.0"
			--windowstargetplatformversion "10.0.16299.0"
			windowstargetplatformversion "10.0.17134.0"
			--windowstargetplatformversion "10.0.17134.471"


-- End Project Config --