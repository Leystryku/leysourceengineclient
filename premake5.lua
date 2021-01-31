solution "leysourceengineclient"
   language "C++"
   location "project"
   targetdir "build/release"

   flags { "NoMinimalRebuild", "NoBufferSecurityCheck"}

   symbols "On"
   optimize "On"
   omitframepointer "On"
   vectorextensions "SSE2"
   floatingpoint "Fast"

   if os.istarget("linux") or os.istarget("macosx") then
      buildoptions {"-m32 -fPIC -ldl -lstdc++"}
      linkoptions  {"-m32 -fPIC -ldl -lstdc++"}
   end

   vpaths {
      ["Header Files/*"] = "src/**.h",
      ["Source Files/*"] = "src/**.cpp",
   }

   kind "ConsoleApp"

   configurations { "Debug", "Release" }

   files { "src/**.h", "src/**.cpp" }

   libdirs {"libs/"}
   links { "Shlwapi", "ws2_32", "winmm" }

   -- A project defines one build target
   project "leysourceengineclient"
      targetname "leysourceengineclient"

      configuration "Release"
         defines { "_CRT_SECURE_NO_WARNINGS", "NDEBUG", "_GENERIC" }
         symbols "Off"
         targetdir "build/release"

      configuration "Debug"
         defines { "_CRT_SECURE_NO_WARNINGS", "DEBUG", "_GENERIC" }
         optimize "Off"
         targetdir "build/debug"