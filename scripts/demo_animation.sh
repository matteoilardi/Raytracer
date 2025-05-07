#!/bin/bash

for count in $(seq 0 359); do
    # Angle with three digits, e.g. angle="1" → angleNNN="001"
    countNNN=$(printf "%03d" $count)
    phi=$count
    if [ $count -lt 180 ]; then
        theta=$(echo "90 - 0.25 * $count" | bc)
    else
        theta=$(echo "45 + 0.25 * ($count - 180)" | bc)
    fi
    ../build/raytracer demo -d 1 --phi-deg $phi --theta-deg $theta --output-file=frame$countNNN.png
    echo "phi = $phi"
    echo "theta = $theta"
done

# -r 25: Number of frames per second
ffmpeg -r 25 -f image2 -s 640x480 -i ../build/frame%03d.png \
    -vcodec libx264 -pix_fmt yuv420p \
    demo_animation.mp4
