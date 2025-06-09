#include "engine.hpp"

#include "AL/al.h"
#include "AL/alc.h"

/*
* Struct that holds the RIFF data of the Wave file.
* The RIFF data is the meta data information that holds,
* the ID, size and format of the wave file
*/
struct RIFF_Header {
	char chunkID[4];
	long chunkSize;  //size not including chunkSize or chunkID
	char format[4];
};

/*
* Struct to hold fmt subchunk data for WAVE files.
*/
struct WAVE_Format {
	char subChunkID[4];
	long subChunkSize;
	short audioFormat;
	short numChannels;
	long sampleRate;
	long byteRate;
	short blockAlign;
	short bitsPerSample;
};

/*
* Struct to hold the data of the wave file
*/
struct WAVE_Data {
	char subChunkID[4];  //should contain the word data
	long subChunk2Size;  //Stores the size of the data block
};

/**
* Load wave file function. No need for ALUT or ALURE with this
* @return Wether the file was succesfully loaded
* @param filename Path of the WAV file
* @param buffer OpenAL Buffer to load the WAV file to
* @param size Uncompressed sample size
* @param frequency Sample frequency (e.g: 11025, 22050, 44100...) in hertz
* @param format OpenAL sample format
*/
bool LoadWavFile(const std::string filename, ALuint* buffer,
	ALsizei* size, ALsizei* frequency,
	ALenum* format) {
	//Local Declarations
	FILE* soundFile = NULL;
	WAVE_Format wave_format;
	RIFF_Header riff_header;
	WAVE_Data wave_data;
	unsigned char* data;

	soundFile = fopen(filename.c_str(), "rb");
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

int main(int, char**) {
	//*/
	unsigned int const window_w = 1280, window_h = 720;
	/*/
	unsigned int const window_w = 1920, window_h = 1080;
	/**/
	Engine engine = Engine(window_w, window_h);
	Window* window = engine.getWindow();

	window->set_title("Test Audio");

	// #### CAMERA ####
	engine.getComponentManager()->NewPerspectiveCamera(45.0f, (float)window_w / (float)window_h, 1.0f, 2000.0f);
	engine.getComponentManager()->get_principal_camera()->position_ = glm::vec3(80.0f, 80.0f, -80.0f);
	engine.getComponentManager()->get_principal_camera()->pitch_ = -40.0f;
	engine.getComponentManager()->get_principal_camera()->yaw_ = 130.0f;


	
	// Buffers hold sound data
	ALuint buf;
	// Sources are points emitting sound
	ALuint src;

	ALCdevice* device;
	ALCcontext* ctx;

	ALenum alert = alGetError();
	if (alert == AL_NO_ERROR) printf("Alert 1 | Everyting ok\n");
	else printf("Error -> %d\n", alert);

	// Open device and create context
	device = alcOpenDevice("openal-soft");
	ctx = alcCreateContext(device, NULL);
	alcMakeContextCurrent(ctx);

	// Create a source
	alGenSources(1, &src);
	if (alGetError() != AL_NO_ERROR) {
		std::cerr << "Failed to create OpenAL source!" << std::endl;
		return 1;
	}

	// Position of the source sound.
	ALfloat source_pos[] = { 0.0, 0.0, 0.0 };
	// Velocity of the source sound.
	ALfloat source_vel[] = { 0.0, 0.0, 0.0 };
	alSourcef(src, AL_PITCH, 1.0f);
	alSourcef(src, AL_GAIN, 1.0f);
	alSourcefv(src, AL_POSITION, source_pos);
	alSourcefv(src, AL_VELOCITY, source_vel);
	alSourcei(src, AL_LOOPING, 0);

	// Load wave data into a buffer
	ALsizei siz, freq;
	ALenum form;
	bool ret = LoadWavFile("../data/audio/combat.wav", &buf, &siz, &freq, &form);

	if (ret == false) {
		std::cerr << "Could not load Wave file" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file: " << siz << ", " << freq << ", " << form << std::endl;
	}

	// Link source with buffer
	alSourcei(src, AL_BUFFER, buf);
	// Play source
	alSourcePlay(src);

	alert = alGetError();
	if (alert == AL_NO_ERROR) printf("Alert 2 | Everyting ok\n");
	else printf("Error -> %d\n", alert);

	if (window) {
	  while (window->is_open() && !window->return_last_input().escape) {
			engine.Update();

			//printf("Hola\n");

			engine.Render();

	  }
	}
	else {std::cout << "Something went wrong...";}

	//esat::WindowDestroy();

	alert = alGetError();

	//bool play = true;

	//esat::WindowInit(720, 640);
	//esat::WindowSetMouseVisibility(true);

	//while (esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
	//  esat::DrawBegin();
	//  esat::DrawClear(0, 0, 0);

	//  /*if (play) {
	//    alSourcePlay(src); 
	//    play = false;
	//  }*/

	//  esat::DrawEnd();
	//  esat::WindowFrame();
	//}

	//esat::WindowDestroy();

	return 0;
}




