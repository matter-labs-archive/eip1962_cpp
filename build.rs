extern crate cc;

fn main() {
    #[cfg(all(not(feature = "fuzz"), not(feature = "gcc"), not(feature = "clang")))]
    {
        cc::Build::new()
            .cpp(true) // Switch to C++ library compilation.
            .flag("-std=c++1z")
            // .flag("-static-libstdc++")
            .include("eip1962cpp/include")
            .file("eip1962cpp/src/api.cpp")
            .file("eip1962cpp/src/common.cpp")
            .file("eip1962cpp/src/wrapper.cpp")
            .file("eip1962cpp/src/repr.cpp")
            .file("eip1962cpp/src/gas_meter.cpp")
            .warnings(false)
            // .static_flag(true)
            // .opt_level_str("3")
            .compile("eip1962cpp.a");
    }

    #[cfg(all(not(feature = "fuzz"), not(feature = "gcc"), feature = "clang"))]
    {
        let compiler = cc::Build::new().cpp(true).get_compiler();
        if !compiler.is_like_clang() {
            panic!("Target compiler is not Clang like. To use GCC use feature `gcc`");
        }
        cc::Build::new()
            .cpp(true) // Switch to C++ library compilation.
            .flag("-std=c++1z")
            // .flag("-static-libstdc++")
            .include("eip1962cpp/include")
            .file("eip1962cpp/src/api.cpp")
            .file("eip1962cpp/src/common.cpp")
            .file("eip1962cpp/src/wrapper.cpp")
            .file("eip1962cpp/src/repr.cpp")
            .file("eip1962cpp/src/gas_meter.cpp")
            .warnings(false)
            // .static_flag(true)
            .opt_level_str("3")
            .compile("eip1962cpp.a");
    }

    #[cfg(all(not(feature = "fuzz"), feature = "gcc"))]
    {
        let compiler = cc::Build::new().cpp(true).get_compiler();
        if !compiler.is_like_gnu() {
            panic!("Target compiler is not GCC like. To use Clang use default features");
        }
        cc::Build::new()
            .cpp(true) // Switch to C++ library compilation.
            .flag("-std=c++1z")
            // .flag("-static-libstdc++")
            .include("eip1962cpp/include")
            .file("eip1962cpp/src/api.cpp")
            .file("eip1962cpp/src/common.cpp")
            .file("eip1962cpp/src/wrapper.cpp")
            .file("eip1962cpp/src/repr.cpp")
            .file("eip1962cpp/src/gas_meter.cpp")
            .warnings(false)
            // .static_flag(true)
            .opt_level_str("3")
            .compile("eip1962cpp.a");
    }

    #[cfg(all(feature = "fuzz", not(target_os = "macos")))]
    {
        let compiler = cc::Build::new().cpp(true).get_compiler();
        if !compiler.is_like_clang() {
            panic!("Fuzzing target should be compiled by Clang");
        }
        cc::Build::new()
            .cpp(true) 
            .flag("-std=c++1z")
            // .flag("-static-libstdc++")
            // .flag("-fuse-ld=lld")
            .flag("-fsanitize=fuzzer,undefined,address") // Additional flags for fuzzing
            .include("eip1962cpp/include")
            .file("eip1962cpp/src/api.cpp")
            .file("eip1962cpp/src/common.cpp")
            .file("eip1962cpp/src/wrapper.cpp")
            .file("eip1962cpp/src/repr.cpp")
            .file("eip1962cpp/src/gas_meter.cpp")
            .warnings(false)
            // .static_flag(true)
            // .opt_level_str("3")
            .compile("eip1962cpp_fuzz.a");
    }

    #[cfg(all(feature = "fuzz", target_os = "macos"))]
    {
        let compiler = cc::Build::new().cpp(true).get_compiler();
        if !compiler.is_like_clang() {
            panic!("Fuzzing target should be compiled by Clang");
        }

        cc::Build::new()
            .cpp(true) 
            .flag("-std=c++1z")
            // .flag("-static-libstdc++")
            // .flag("-fuse-ld=lld")
            .flag("-fsanitize=fuzzer,undefined") // Additional flags for fuzzing
            .include("eip1962cpp/include")
            .file("eip1962cpp/src/api.cpp")
            .file("eip1962cpp/src/common.cpp")
            .file("eip1962cpp/src/wrapper.cpp")
            .file("eip1962cpp/src/repr.cpp")
            .file("eip1962cpp/src/gas_meter.cpp")
            .warnings(false)
            // .static_flag(true)
            // .opt_level_str("3")
            .compile("eip1962cpp_fuzz.a");
    }
}