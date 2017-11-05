#!/usr/bin/env python3

from setuptools import setup, find_packages, Extension
import subprocess

def pkgconfig(option, name):
    return subprocess.check_output(["pkg-config", option, name]).decode('UTF-8').strip()

ext_native = Extension(
    "ppmpeg._native",
    [
        "ppmpeg/_native/util/buffer.c",
        "ppmpeg/_native/util/codec.c",
        "ppmpeg/_native/util/frame.c",
        "ppmpeg/_native/util/instance.c",
        "ppmpeg/_native/util/resamplers.c",
        "ppmpeg/_native/util/scalars.c",
        "ppmpeg/_native/_native.c",
        "ppmpeg/_native/ffmpeg.c",
        "ppmpeg/_native/media_reader.c",
        "ppmpeg/_native/media_util.c",
        "ppmpeg/_native/media_writer.c",
    ],
    include_dirs = [
        pkgconfig("--cflags-only-I", "libavcodec").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libavdevice").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libavfilter").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libavformat").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libavutil").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libswresample").lstrip("-I"),
        pkgconfig("--cflags-only-I", "libswscale").lstrip("-I")
    ],
    extra_compile_args = ["-std=c99"],
    library_dirs = [
        pkgconfig("--libs-only-L", "libavcodec").lstrip("-L"),
        pkgconfig("--libs-only-L", "libavdevice").lstrip("-L"),
        pkgconfig("--libs-only-L", "libavfilter").lstrip("-L"),
        pkgconfig("--libs-only-L", "libavformat").lstrip("-L"),
        pkgconfig("--libs-only-L", "libavutil").lstrip("-L"),
        pkgconfig("--libs-only-L", "libswresample").lstrip("-L"),
        pkgconfig("--libs-only-L", "libswscale").lstrip("-L")
    ],
    libraries = ["avcodec",
        "avdevice",
        "avfilter",
        "avformat",
        "avutil",
        "swresample",
        "swscale"
     ],
    extra_link_args = []
)

setup(name = "ppmpeg",
      version = "0.1",
      description = "​Python Library Powered by FFmpeg",
      long_description = "",
      author = "OVENWORKS",
      author_email = "ppmpeg@ovenworks.jp",
      url = "http://www.ovenworks.jp",
      packages = ["ppmpeg"],
      ext_modules = [ ext_native ],
      test_suite = "test"
)


