workspace "avx"
  configurations { "dbg", "dist" }

  toolset "clang"
  language "C++"
  compileas "C++"

  flags { "fatalwarnings", "linktimeoptimization" }

  vectorextensions "avx2"
  vectorextensions "sse4.1"

  filter { "action:gmake*" }
    buildoptions {
      "-Wall", "-Wextra", "-Wfloat-equal", "-Winline", "-Wundef", "-Werror",
      "-fverbose-asm", "-Wint-to-pointer-cast", "-Wshadow", "-Wpointer-arith",
      "-Wcast-align", "-Wcast-qual", "-Wunreachable-code", "-Wstrict-overflow=5",
      "-Wwrite-strings", "-Wconversion", "--pedantic-errors",
      "-Wredundant-decls", "-Werror=uninitialized", "-Wextra",
      "-Wmissing-declarations", "-ftree-vectorize", -- "-fopt-info-vec-all",
      "-march=native"
    }

  --filter { "toolset:gcc" }
  --  buildoptions {
  --    "-Wall", "-Wextra", "-Wfloat-equal", "-Winline", "-Wundef", "-Werror",
  --    "-fverbose-asm", "-Wint-to-pointer-cast", "-Wshadow", "-Wpointer-arith",
  --    "-Wcast-align", "-Wcast-qual", "-Wunreachable-code", "-Wstrict-overflow=5",
  --    "-Wwrite-strings", "-Wconversion", "--pedantic-errors",
  --    "-Wredundant-decls", "-Werror=maybe-uninitialized",
  --    "-Wmissing-declarations",
  --  }

  --filter { "language:c" }
  --  buildoptions {
  --    "-xc", "-std=c11", "-Wmissing-parameter-type",
  --    "-Wmissing-prototypes",
  --    "-Wnested-externs", "-Wold-style-declaration",
  --    "-Wold-style-definition", "-Wstrict-prototypes", "-Wpointer-sign"
  --  }

  filter { "language:c++" }
    buildoptions { "-xc++", "-std=c++14" }

  filter "configurations:dbg"
    optimize "off"
    symbols "on"
    buildoptions { "-ggdb3", "-O0", "-DDEBUG" }

  filter "configurations:dist"
    optimize "full"
    symbols "off"

  project "mains"
    kind "consoleapp"
    files { "src/main.cpp" }
    targetdir  "bin/%{cfg.buildcfg}"
    targetname "%{wks.name}"

  project "clobber"
    kind "makefile"

    local dirs = " bin obj "

    -- on windows, clean like this
    filter "system:not windows"
      cleancommands {
        "({RMDIR}" .. dirs .."*.make Makefile *.o -r 2>/dev/null; echo)"
      }

    -- otherwise, clean like this
    filter "system:windows"
      cleancommands {
        "{DELETE} *.make Makefile *.o",
        "{RMDIR}" .. dirs
      }
