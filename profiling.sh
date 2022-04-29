#!/bin/sh
short_file=test_short.wav
long_file=test.wav

echo "Short file: $short_file"
for reverb_length in 0.01 0.1 1.0; do
    echo "Reverb length: $reverb_length"
    echo "Time-domain:"
    bin/Release/MUSI6106Exec $short_file verb.wav $reverb_length time
    echo "Frequency-domain:"
    bin/Release/MUSI6106Exec $short_file verb.wav $reverb_length freq
    echo
done

echo "Long file: $short_file"
for reverb_length in 0.001 0.01; do
    echo "Reverb length: $reverb_length"
    echo "Time-domain:"
    bin/Release/MUSI6106Exec $long_file verb.wav $reverb_length time
    echo "Frequency-domain:"
    bin/Release/MUSI6106Exec $long_file verb.wav $reverb_length freq
    echo
done

for reverb_length in 0.1 1.0 10.0; do
    echo "Reverb length: $reverb_length"
    echo "Frequency-domain:"
    bin/Release/MUSI6106Exec $long_file verb.wav $reverb_length freq
    echo
done
