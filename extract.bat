@echo off
REM Save the first argument after the batch script name to a variable
set "filename=%~1"

@REM check if file exists
if not exist "%filename%" (
    echo File "%filename%" not found!
    exit /b 1
)

@REM delete everything stored in frames folder
if exist frames (rd /s /q frames)
mkdir frames

@REM remove everything in ascii_frames folder
if exist ascii_frames (rd /s /q ascii_frames)
mkdir ascii_frames

ffmpeg -i "%filename%" "frames/frame_%%02d.png"
python convert.py