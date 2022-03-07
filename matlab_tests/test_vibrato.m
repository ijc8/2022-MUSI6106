[x, Fs] = audioread("input_sine.wav");
y1 = vibrato(x, Fs, 5, 0.01);
audiowrite("output_sine_matlab.wav", y1, Fs);
c1 = audioread("output_sine.wav");

[x, Fs] = audioread("input_whistle.wav");
y2 = vibrato(x, Fs, 5, 0.01);
audiowrite("output_whistle_matlab.wav", y2, Fs);
c2 = audioread("output_whistle.wav");

[x, Fs] = audioread("input_music.wav");
y3 = vibrato(x, Fs, 5, 0.01);
audiowrite("output_music_matlab.wav", y3, Fs);
c3 = audioread("output_music.wav");

figure
% hold on;
% s = 1;
% e = 1000;
% plot(y1(s:e))
% plot(c1(s:e))
% plot(y1(s:e) - c1(s:e))
% hold off
plot(y1 - c1)
title('Difference between MATLAB & C++ output (sine)')
shg

figure
plot(y2 - c2)
title('Difference between MATLAB & C++ output (whistle)')
shg

figure
plot(y3 - c3)
title('Difference between MATLAB & C++ output (music)')
shg
