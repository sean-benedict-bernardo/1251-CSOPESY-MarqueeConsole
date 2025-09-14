# Marquee Console
By 
- Clive Ang
- Sean Bernardo
- Kyle Gan
- Brent Uy

## To run
1. `g++ main.cpp -o main.exe`
2. `main.exe`

## GIF/MP4 to ASCII Conversion Process

1. Create `frames` and `ascii_frames` folder
2. Run `ffmpeg -i <filename> "frames/frame_%%02d.png"` to extract frames from GIF or MP4
3. Run `python convert.py` to convert frames to ASCII
4. Modify `main.cpp` to set `NUM_FRAMES` and `NUM_ROWS` with num_rows being the number of lines in each ASCII frame