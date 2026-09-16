# CMAKE_ARGV contains every argument passed after `cmake -P`, including the
# `--` separator below. Preserve the resolved link order and quote paths that
# contain whitespace so the result can be included in an R Makevars file.
set(record_arguments OFF)
set(link_flags "")
math(EXPR last_argument "${CMAKE_ARGC} - 1")

foreach(index RANGE 0 ${last_argument})
  set(argument_name "CMAKE_ARGV${index}")
  set(argument "${${argument_name}}")

  if(record_arguments)
    if(argument MATCHES "[ \t]")
      string(REPLACE "\"" "\\\"" argument "${argument}")
      set(argument "\"${argument}\"")
    endif()
    list(APPEND link_flags "${argument}")
  elseif(argument STREQUAL "--")
    set(record_arguments ON)
  endif()
endforeach()

string(JOIN " " link_flags_line ${link_flags})
file(WRITE "${OUTPUT}" "${link_flags_line}\n")
