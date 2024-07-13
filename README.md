# Screen Recorder
Basic screen recorder with WINAPI.

## Compilation
- Using [myke](https://github.com/bogdanmaciuca/myke): `py myke.py makefile.myke`
- Using `cl`: `cl main.c stb_image_write_impl.c /link /out:scrrec.exe user32.lib gdi32.lib`

## Features:
- Screenshots of the full screen (supports cropping)
