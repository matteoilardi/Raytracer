#!/bin/bash

if [ "$1" == "" ]; then
    echo "Usage: $(basename $0) ANGLE"
    exit 1
fi

readonly count="$1"
readonly phi=$((count+180))
readonly countNNN=$(printf "%03d" $count)
readonly pngfile=frame$countNNN


../build/raytracer render -s ../samples/demo_path_tracing.txt --mode path --antialiasing 1 --output-file=$pngfile --define-float clock=$count
echo "phi = $phi"

# TO generate the demo animation, run the following commands in the build directory

# 1) FRAME GENERATION: PARALLELIZATION FROM COMMAND LINE
# -j 6: Number of cores

# parallel -j 6 ../scripts/generate_image.sh '{}' ::: $(seq 0 359)


# MP4 FILE GENERATION FROM FRAMES
# -r 25: Number of frames per second
# -f image2: Input format (image2 is ffmpeg's demultiplexer for still image sequences)
# -s 1280x960: Image resolution
# -i ../build/frame%03d.png: Path to frames
#  demo_path_tracing_animation.mp4

# ffmpeg -r 25 -f image2 -s 1280x960 -i frame%03d.png -vcodec libx264 -pix_fmt yuv420p demo_path_tracing_animation.mp4
