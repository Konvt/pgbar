add_rules("mode.debug", "mode.release")

set_languages("cxx11") -- modify it to change the global language standard
add_includedirs("include/")

for _, file in ipairs(os.files("tests/*.cpp")) do
  local name = path.basename(file)

  if not string.find(name, 'performance') then
    target(name)
      set_kind("binary")
      set_default(false)
      add_files("tests/" .. name .. ".cpp")
      add_tests(name, {run_timeout = 20000}) -- 20s
  end
end

target("demo")
  set_kind("binary")
  add_files("demo/demo.cpp")
  set_default(true)

target("format")
  set_kind("phony")
  on_run(function (target)
    os.run(format("clang-format -i include/pgbar/pgbar.hpp"))
    os.run(format("clang-format -i demo/demo.cpp"))

    for _, filepath in ipairs(os.files("tests/*.*pp")) do
      os.run(format("clang-format -i %s", filepath))
    end
  end)
