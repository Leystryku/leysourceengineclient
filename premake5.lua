solution "leysourceengineclient"
   language "C++"
   location "project"
   targetdir "build/release"

   flags { "Optimize", "NoMinimalRebuild", "NoFramePointer", "EnableSSE2", "FloatFast", "NoBufferSecurityCheck"}

   if os.is("linux") or os.is("macosx") then
      buildoptions {"-m32 -fPIC -ldl -lstdc++"}
      linkoptions  {"-m32 -fPIC -ldl -lstdc++"}
   else
      --buildoptions({"/Qpar", "/Qfast_transcendentals", "/GL", "/Ox", "/Gm", "/MP", "/MD", "/Gy", "/Gw"})
      --linkoptions { "/OPT:REF", "/OPT:ICF", "/LTCG"}
   end

   vpaths {
      ["Header Files/*"] = "src/**.h",
      ["Source Files/*"] = "src/**.cpp",
   }

   kind "ConsoleApp"

   configurations { "Debug", "Release" }

   files { "src/**.h", "src/**.cpp" }
   
   includedirs { }
   libdirs {"libs/"}
   links { "ws2_32", "winmm" }
   
   
   -- A project defines one build target
   project "leysourceengineclient"
      targetname "leysourceengineclient"

      configuration "Release"
         defines { "NDEBUG", "_GENERIC" }
         
      configuration "Debug"
         defines { "DEBUG", "_GENERIC" }
         flags { "Symbols", "EnableSSE2" }
         targetdir "build/debug"