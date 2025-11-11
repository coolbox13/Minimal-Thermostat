Import("env")
import platform

# Detect operating system
system = platform.system()

if system == "Darwin":  # macOS
    # Force C++ compilation with clang++ and libc++
    env.Replace(
        CC="clang++",
        CXX="clang++",
    )
    env.Append(
        CCFLAGS=["-stdlib=libc++"],
        LINKFLAGS=["-stdlib=libc++", "-lc++"]
    )
    print("Configured native platform for macOS: clang++ with libc++")
elif system == "Linux":
    # Use g++ with libstdc++ (default on Linux)
    env.Replace(
        CC="g++",
        CXX="g++",
    )
    env.Append(
        LINKFLAGS=["-lstdc++"]
    )
    print("Configured native platform for Linux: g++ with libstdc++")
else:
    # Windows or other platforms - use defaults
    print(f"Configured native platform for {system}: using default compiler")
