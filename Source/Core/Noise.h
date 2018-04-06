#pragma once

//exports.fbm_noise = function(noise, amplitudes, nx, ny) {
//	let sum = 0, sumOfAmplitudes = 0;
//	for (let octave = 0; octave < amplitudes.length; octave++) {
//		let frequency = 1 << octave;
//		sum += amplitudes[octave] * noise.noise2D(nx * frequency, ny * frequency, octave);
//		sumOfAmplitudes += amplitudes[octave];
//	}
//	return sum / sumOfAmplitudes;
//};

// returns value in -1 to 1 range
//double Noise(double x, double y);