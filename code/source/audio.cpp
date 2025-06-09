#include "audio.hpp"

#include <iostream>
#include <stdio.h>

#include <chrono>
#include <ctime>
#include <vector>

Audio::Audio() {
	alGenSources(1, &src_);
	if (alGetError() != AL_NO_ERROR)
	{
		std::cerr << "Failed to create OpenAL source" << std::endl;
	}

	pitch_ = 1.0f;
	gain_ = 1.0f;
	looping_ = false;
	position_[0] = 0.0f;
	position_[1] = 0.0f;
	position_[2] = 0.0f;

	velocity_[0] = 0.0f;
	velocity_[1] = 0.0f;
	velocity_[2] = 0.0f;
	buffer_ = 0;
	sourceState_ = 0;

	alSourcef(src_, AL_PITCH, pitch_);
	alSourcef(src_, AL_GAIN, gain_);
	alSourcefv(src_, AL_POSITION, position_);
	alSourcefv(src_, AL_VELOCITY, velocity_);
	alSourcei(src_, AL_LOOPING, looping_);
}

bool Audio::loadWAVFile(std::string filename) {
	
	// Load wave data into a buffer
	ALsizei siz, freq;
	ALenum form;

	bool ret = false;
	ret = Audio::initWAVFile(filename, &buffer_, &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file: " << siz << ", " << freq << ", " << form << std::endl;
	}

	filepath_ = filename;

	// Link source with buffer
	alSourcei(src_, AL_BUFFER, buffer_);

	return ret;
}

Audio::~Audio() {
	alDeleteSources(1, &src_);
	alDeleteBuffers(1, &src_);
}

bool Audio::initWAVFile(const std::string filename, ALuint* buffer, ALsizei* size, ALsizei* frequency, ALenum* format) {
	//Local Declarations
	FILE* soundFile = NULL;
	WAVE_Format wave_format;
	RIFF_Header riff_header;
	WAVE_Data wave_data;
	unsigned char* data;

	//soundFile = fopen(filename.c_str(), "rb");
	fopen_s(&soundFile, filename.c_str(), "rb");
	if (!soundFile)
		return false;

	// Read in the first chunk into the struct
	fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);

	//check for RIFF and WAVE tag in memeory
	if ((riff_header.chunkID[0] != 'R' ||
		riff_header.chunkID[1] != 'I' ||
		riff_header.chunkID[2] != 'F' ||
		riff_header.chunkID[3] != 'F') ||
		(riff_header.format[0] != 'W' ||
			riff_header.format[1] != 'A' ||
			riff_header.format[2] != 'V' ||
			riff_header.format[3] != 'E')) {
		fclose(soundFile);
		return false;
	}

	//Read in the 2nd chunk for the wave info
	fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);
	//check for fmt tag in memory
	if (wave_format.subChunkID[0] != 'f' ||
		wave_format.subChunkID[1] != 'm' ||
		wave_format.subChunkID[2] != 't' ||
		wave_format.subChunkID[3] != ' ') {
		fclose(soundFile);
		return false;
	}

	//check for extra parameters;
	if (wave_format.subChunkSize > 16)
		fseek(soundFile, sizeof(short), SEEK_CUR);

	//Read in the the last byte of data before the sound file
	fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);
	//check for data tag in memory
	if (wave_data.subChunkID[0] != 'd' ||
		wave_data.subChunkID[1] != 'a' ||
		wave_data.subChunkID[2] != 't' ||
		wave_data.subChunkID[3] != 'a') {
		fclose(soundFile);
		return false;
	}

	//Allocate memory for data
	data = new unsigned char[wave_data.subChunk2Size];

	// Read in the sound data into the soundData variable
	if (!fread(data, wave_data.subChunk2Size, 1, soundFile)) {
		fclose(soundFile);
		return false;
	}

	//Now we set the variables that we passed in with the
	//data from the structs
	*size = wave_data.subChunk2Size;
	*frequency = wave_format.sampleRate;
	//The format is worked out by looking at the number of
	//channels and the bits per sample.
	if (wave_format.numChannels == 1) {
		if (wave_format.bitsPerSample == 8)
			*format = AL_FORMAT_MONO8;
		else if (wave_format.bitsPerSample == 16)
			*format = AL_FORMAT_MONO16;
	}
	else if (wave_format.numChannels == 2) {
		if (wave_format.bitsPerSample == 8)
			*format = AL_FORMAT_STEREO8;
		else if (wave_format.bitsPerSample == 16)
			*format = AL_FORMAT_STEREO16;
	}
	//create our openAL buffer and check for success
	alGenBuffers(1, buffer);

	//now we put our data into the openAL buffer and
	//check for success
	alBufferData(*buffer, *format, (void*)data,
		*size, *frequency);

	//clean up and return true if successful
	fclose(soundFile);
	delete[] data;
	return true;
}

void Audio::play() {
	alSourcePlay(src_);
}

void Audio::stop() {
	alSourceStop(src_);
}

void Audio::pause() {
	alSourcePause(src_);
}

void Audio::destroy() {
	alDeleteSources(1, &src_);
	alDeleteBuffers(1, &src_);
}

void Audio::setPosition(ALfloat pos[3]) {
	for (int i = 0; i < 3; i++) {
		position_[i] = pos[i];
	}
	alSourcefv(src_, AL_POSITION, position_);
}

void Audio::setVelocity(ALfloat vel[3]) {
	for (int i = 0; i < 3; i++) {
		velocity_[i] = vel[i];
	}
	alSourcefv(src_, AL_VELOCITY, velocity_);
}

void Audio::setPitch(ALfloat pitch) {
	pitch_ = pitch;
	alSourcef(src_, AL_PITCH, pitch_);
}

void Audio::setGain(ALfloat gain) {
	gain_ = gain;
	alSourcef(src_, AL_GAIN, gain_);
}

void Audio::setLooping(bool looping) {
	looping_ = looping;
	alSourcei(src_, AL_LOOPING, looping_);
}

float Audio::getAudioLength() {
	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	ALint frequency;

	alGetBufferi(buffer_, AL_SIZE, &sizeInBytes);
	alGetBufferi(buffer_, AL_CHANNELS, &channels);
	alGetBufferi(buffer_, AL_BITS, &bits);

	alGetBufferi(buffer_, AL_FREQUENCY, &frequency);

	double lengthInSamples = sizeInBytes * 8 / (channels * bits);

	return (float)lengthInSamples / (float)frequency;
}

void Audio::waitSeconds(double num_seconds) {
	auto start = std::chrono::system_clock::now();
	auto end = std::chrono::system_clock::now();
	double seconds_to_nano = ((double)num_seconds) * 10000000;
	while ((end - start).count() < seconds_to_nano) {
		end = std::chrono::system_clock::now();
	}
}

void Audio::move(ALfloat mov[3]) {
	for (int i = 0; i < 3; i++) {
		position_[i] += mov[i];
	}
	alSourcefv(src_, AL_POSITION, position_);
}

void Audio::moveX(ALfloat movX) {
	position_[0] += movX;
	alSourcefv(src_, AL_POSITION, position_);
}

void Audio::moveY(ALfloat movY) {
	position_[1] += movY;
	alSourcefv(src_, AL_POSITION, position_);
}

void Audio::moveZ(ALfloat movZ) {
	position_[2] += movZ;
	alSourcefv(src_, AL_POSITION, position_);
}

void Audio::linkBuffer(ALuint buff) {
	alSourcei(src_, AL_BUFFER, buff);
}

ALfloat* Audio::position() {
	return &position_[0];
}

ALfloat* Audio::velocity() {
	return &velocity_[0];
}

ALfloat Audio::pitch() {
	return pitch_;
}

ALfloat Audio::gain() {
	return gain_;
}

ALfloat Audio::velocityX() {
	return velocity_[0];
}

ALfloat Audio::velocityY() {
	return velocity_[1];
}

ALfloat Audio::velocityZ() {
	return velocity_[2];
}

ALfloat Audio::posX() {
	return position_[0];
}

ALfloat Audio::posY() {
	return position_[1];
}

ALfloat Audio::posZ() {
	return position_[2];
}

bool Audio::looping() {
	return looping_;
}

void Audio::synchronizePositionWithVelocity() {
	//Update position to be coherent with the velocity
	position_[0] += velocity_[0];
	position_[1] += velocity_[1];
	position_[2] += velocity_[2];
	alSourcefv(src_, AL_POSITION, position_);

	//Update source state
	alGetSourcei(src_, AL_SOURCE_STATE, &sourceState_);
}
