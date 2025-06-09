#ifndef __AUDIO_HPP__
#define __AUDIO_HPP__ 1

#include "AL/al.h"
#include "AL/alc.h"

#include <string>

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


struct Audio {

	Audio();

	/**
	* @brief Custom constructor
	* @param filepath Path to an audio file to be loaded. Format must be WAV
	*/
	Audio(const char* filepath);
	~Audio();

	/**
	 * @brief Waits N seconds
	 * 
	 * @param num_seconds Given seconds to wait
	 */
	static void waitSeconds(double num_seconds);

	/**
	* @brief Load WAV file function. No need for ALUT or ALURE with this
	* 
	* @param filename Path of the WAV file
	* @param buffer OpenAL Buffer to load the WAV file to
	* @param size Uncompressed sample size
	* @param frequency Sample frequency (e.g: 11025, 22050, 44100...) in hertz
	* @param format OpenAL sample format
	* 
	* @return bool Wether the file was succesfully loaded or not
	*/
	static bool loadWAVFile(const std::string filename, ALuint* buffer, ALsizei* size, ALsizei* frequency, ALenum* format);

	/** Plays the Audio */
	void play();

	/** Stops the Audio */
	void stop();

	/** Pauses the Audio */
	void pause();

	/** Sets the position in a 3D space */
	void setPosition(ALfloat pos[3]);

	/** Sets the velocity of the Audio */
	void setVelocity(ALfloat vel[3]);

	/** Sets the pitch value for the Audio */
	void setPitch(ALfloat pitch);

	/** Sets the gain (volume) for the Audio */
	void setGain(ALfloat gain);

	/** Sets wheter the Audio has to play the audio in loop or not */
	void setLooping(bool looping);

	/** Returns the length of the Audio */
	float getAudioLength();

	/** Adds a given movement vector to the position of the Audio */
	void move(ALfloat mov[3]);

	/** Moves the Audio only in X axis */
	void moveX(ALfloat movX);

	/** Moves the Audio only in Y axis */
	void moveY(ALfloat movY);

	/** Moves the Audio only in Z axis */
	void moveZ(ALfloat movZ);

	/** Links an OpenAL buffer to this Audio */
	void linkBuffer(ALuint buff);

	/** Getter for Audio position */
	ALfloat* position();

	/** Getter for Audio velocity */
	ALfloat* velocity();

	/** Getter for Audio pitch */
	ALfloat pitch();

	/** Getter for gain (volume) */
	ALfloat gain();

	/** Getter for velocity in X axis */
	ALfloat velocityX();

	/** Getter for velocity in Y axis */
	ALfloat velocityY();

	/** Getter for velocity in Z axis */
	ALfloat velocityZ();

	/** Getter for X position */
	ALfloat posX();

	/** Getter for Y position */
	ALfloat posY();

	/** Getter for Z position */
	ALfloat posZ();

	/** Getter for looping state */
	bool looping();

	/**
 	* @brief Synchronizes the position of the Audio.
 	*
 	* Updates the position of the Audio to be coherent with its velocity.
	* This is due to the fact that OpenAL does not move the sources even if they
	* have velocity, so this is done here.
	* Has to be called on every frame.
	*
	* On the other hand, updates the sourceState_ enum to get the state of the
	* audio (stopped, playing...)
	*/
	void synchronizePositionWithVelocity();

	/** OpenAL source handle */
	ALuint src_;

	/** OpenAL buffer handle */
	ALuint buffer_;

	/** Position */
	ALfloat position_[3];
	/** Velocity */

	ALfloat velocity_[3];

	/** State */
	ALint sourceState_;

	/** Pitch */
	ALfloat pitch_;

	/** Gain (volume) */
	ALfloat gain_;

	/** Looping */
	bool looping_;

};

#endif // __AUDIO_HPP__




