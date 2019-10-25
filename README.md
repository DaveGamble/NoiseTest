# NoiseTest

Quick utility for dividing two wav files spectrally, when measuring with white noise.


Use NoiseTest <noise_file_to_generate.wav> to make some Gaussian distributed white noise.

Use NoiseTest <the_noise_file.wav> <what_you_measured.wav> <output_ir.wav> to get an IR from what you recorded.

Compile with: g++ NoiseTest.cpp WavFile.cpp fftsg_h.cpp -o NoiseTest

should be fine on anything posix.

