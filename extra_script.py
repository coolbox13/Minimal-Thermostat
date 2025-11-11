Import("env")

# Force C++ compilation and add standard library
env.Replace(
    CC="clang++",
    CXX="clang++",
)

env.Append(
    CCFLAGS=["-stdlib=libc++"],
    LINKFLAGS=["-stdlib=libc++", "-lc++"]
)

print("Configured native platform to use clang++ with libc++")
