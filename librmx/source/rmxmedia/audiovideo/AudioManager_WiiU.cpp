/*
 * Wii U audio backend shim for AudioManager
 * Provides a mixing thread and TODO hooks for WHB/WUT audio output.
 * This implementation mixes audio using the existing mixer code and
 * calls a platform-specific output path (currently a TODO).
 */

#include "rmxmedia.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace rmx
{
	AudioManager::AudioManager() :
		mRootMixer(*new AudioMixer(0)),
		mAudioThreadRunning(false),
		mAudioThread(nullptr)
	{
		mAudioMixers[0] = &mRootMixer;
	}

	AudioManager::~AudioManager()
	{
		// Stop thread if running
		exit();

		// Delete all audio mixers (including the root mixer)
		for (const auto& [key, audioMixer] : mAudioMixers)
		{
			delete audioMixer;
		}
	}

	void AudioManager::initialize(int sample_freq, int channels, int audioBufferSamples)
	{
		if (!FTX::System->initialize())
			return;

		// Reset instances
		mInstances.clear();
		mRootMixer.clearAudioInstances();

		if ((sample_freq % 11025) != 0)
		{
			if (sample_freq != 48000)
			{
				RMX_ASSERT(false, "Unsupported sample frequency: " << sample_freq << " Hz");
				int sf = sample_freq / 11025;
				sf = (sf == 3) ? 2 : clamp(sf, 1, 4);
				sample_freq = sf * 11025;
			}
		}
		channels = clamp(channels, 1, 2);

		mFormat.freq = sample_freq;
		mFormat.format = 0; // placeholder
		mFormat.channels = channels;
		mFormat.samples = audioBufferSamples;

		mPlayedSamples = 0;
		mAudioThreadRunning = true;
		mAudioThread = new std::thread([this]() {
			const int bytesPerSample = sizeof(short) * mFormat.channels;
			const int bufSamples = mFormat.samples;
			std::vector<uint8> outbuf(bufSamples * bytesPerSample);
			while (mAudioThreadRunning)
			{
				// Mix audio into outbuf
				mixAudio(outbuf.data(), (int)outbuf.size());
				// TODO: Send `outbuf` to WHB/WUT audio output for TV and GamePad here.
				// Example placeholders:
				// whbAudioWrite(outbuf.data(), outbuf.size());
				// whbGamePadAudioWrite(outbuf.data(), outbuf.size());

				// Simple scheduler: sleep for buffer duration
				double seconds = (double)bufSamples / (double)mFormat.freq;
				std::this_thread::sleep_for(std::chrono::duration<double>(seconds * 0.9));
			}
		});
	}

	void AudioManager::exit()
	{
		if (mAudioThreadRunning)
		{
			mAudioThreadRunning = false;
			if (mAudioThread && mAudioThread->joinable())
				mAudioThread->join();
			delete mAudioThread;
			mAudioThread = nullptr;
		}
	}

	void AudioManager::clear()
	{
		if (!mInstances.empty())
		{
			lockAudio();
			for (const auto& [key, audioMixer] : mAudioMixers)
			{
				audioMixer->clearAudioInstances();
			}
			unlockAudio();

			mInstances.clear();
			++mChangeCounter;
		}
	}

	void AudioManager::playAudio(bool onoff)
	{
		mAudioPlaying = onoff;
	}

	bool AudioManager::getAudioState()
	{
		return mAudioPlaying;
	}

	void AudioManager::lockAudio()
	{
		if (mAudioLocks == 0)
		{
			mAudioMutex.lock();
		}
		++mAudioLocks;
	}

	void AudioManager::unlockAudio()
	{
		RMX_ASSERT(mAudioLocks > 0, "Called 'AudioManager::unlockAudio' without locking");
		--mAudioLocks;
		if (mAudioLocks == 0)
		{
			mAudioMutex.unlock();
		}
	}

	void AudioManager::regularUpdate(float deltaSeconds)
	{
		mTimeSinceLastUpdate += deltaSeconds;
		if (mTimeSinceLastUpdate >= 0.5f)
		{
			mTimeSinceLastUpdate = 0.0f;
			lockAudio();

			static std::vector<std::pair<AudioBuffer*, int>> audioBufferPurgePositions;
			audioBufferPurgePositions.clear();
			for (const auto& instancePair : mInstances)
			{
				const AudioInstance& instance = instancePair.second;
				AudioBuffer* audioBuffer = instance.mAudioBuffer;
				if (nullptr != audioBuffer && !audioBuffer->isPersistent())
				{
					bool found = false;
					for (auto& bufferPair : audioBufferPurgePositions)
					{
						if (bufferPair.first == audioBuffer)
						{
							bufferPair.second = std::min(bufferPair.second, instance.mPosition);
							found = true;
							break;
						}
					}

					if (!found)
					{
						audioBufferPurgePositions.emplace_back(audioBuffer, instance.mPosition);
					}
				}
			}

			for (auto& bufferPair : audioBufferPurgePositions)
			{
				AudioBuffer* audioBuffer = bufferPair.first;
				audioBuffer->lock();
				audioBuffer->markPurgeableSamples(bufferPair.second);
				audioBuffer->unlock();
			}

			unlockAudio();
		}
	}

	void AudioManager::setGlobalVolume(float volume)
	{
		mRootMixer.setVolume(volume);
	}

	AudioMixer* AudioManager::getAudioMixerByID(int mixerId) const
	{
		const auto it = mAudioMixers.find(mixerId);
		return (it == mAudioMixers.end()) ? nullptr : it->second;
	}

	void AudioManager::deleteAudioMixerByID(int mixerId)
	{
		RMX_ASSERT(mixerId != 0, "Can't delete root audio mixer (with ID 0)");
		mAudioMixers.erase(mixerId);
	}

	float AudioManager::getAudioMixerVolumeByID(int mixerId) const
	{
		const AudioMixer* audioMixer = getAudioMixerByID(mixerId);
		return (nullptr != audioMixer) ? audioMixer->getVolume() : 0.0f;
	}

	void AudioManager::setAudioMixerVolumeByID(int mixerId, float relativeVolume)
	{
		AudioMixer* audioMixer = getAudioMixerByID(mixerId);
		if (nullptr != audioMixer)
		{
			audioMixer->setVolume(relativeVolume);
		}
	}

	bool AudioManager::addSound(const PlaybackOptions& playbackOptions, AudioReference& ref)
	{
		if (nullptr == playbackOptions.mAudioBuffer)
			return false;

		AudioMixer* audioMixer = getAudioMixerByID(playbackOptions.mAudioMixerId);
		if (nullptr == audioMixer)
			return false;

		AudioInstance& instance = mInstances[mNextFreeID];
		instance.mID = mNextFreeID;
		instance.mAudioBuffer = playbackOptions.mAudioBuffer;
		instance.mAudioMixer = audioMixer;
		instance.mVolume = playbackOptions.mVolume;
		instance.mVolumeChange = playbackOptions.mVolumeChange;
		instance.mSpeed = playbackOptions.mSpeed;
		instance.mPosition = roundToInt(playbackOptions.mPosition * (float)playbackOptions.mAudioBuffer->getFrequency());
		instance.mLoop = playbackOptions.mLoop;
		instance.mStreaming = playbackOptions.mStreaming;
		instance.mPaused = playbackOptions.mStartPaused;

		lockAudio();
		audioMixer->addAudioInstance(instance);
		unlockAudio();

		++mChangeCounter;
		++mNextFreeID;

		ref.setInstanceID(instance.mID);
		return (instance.mID != 0);
	}

	int AudioManager::addSound(AudioBuffer* audiobuffer, bool streaming)
	{
		PlaybackOptions playbackOptions;
		playbackOptions.mAudioBuffer = audiobuffer;
		playbackOptions.mStreaming = streaming;

		AudioReference ref;
		addSound(playbackOptions, ref);
		return ref.getInstanceID();
	}

	bool AudioManager::addSound(AudioBuffer* audiobuffer, AudioReference& ref, bool streaming)
	{
		PlaybackOptions playbackOptions;
		playbackOptions.mAudioBuffer = audiobuffer;
		playbackOptions.mStreaming = streaming;
		return addSound(playbackOptions, ref);
	}

	void AudioManager::removeSound(AudioReference& ref)
	{
		if (ref.valid())
		{
			removeInstance(ref.getInstanceID());
		}
	}

	void AudioManager::removeAllSounds()
	{
		clear();
	}

	AudioManager::AudioInstance* AudioManager::findInstance(int ID)
	{
		if (ID <= 0 || ID >= mNextFreeID)
			return nullptr;

		processRemoveIDs();

		const auto it = mInstances.find(ID);
		if (it == mInstances.end())
			return nullptr;

		return &it->second;
	}

	void AudioManager::removeInstance(int ID)
	{
		assert(ID > 0 && ID < mNextFreeID);
		const auto it = mInstances.find(ID);
		if (it != mInstances.end())
		{
			AudioInstance& audioInstance = it->second;
			if (nullptr != audioInstance.mAudioMixer)
			{
				lockAudio();
				audioInstance.mAudioMixer->removeAudioInstance(audioInstance);
				unlockAudio();
			}

			mInstances.erase(it);
			++mChangeCounter;
		}
	}

	void AudioManager::processRemoveIDs()
	{
		if (!mRemoveIDs.empty())
		{
			lockAudio();
			for (int ID : mRemoveIDs)
			{
				const auto it = mInstances.find(ID);
				if (it != mInstances.end())
				{
					AudioInstance& audioInstance = it->second;
					if (nullptr != audioInstance.mAudioMixer)
					{
						audioInstance.mAudioMixer->removeAudioInstance(audioInstance);
					}
				}
			}
			unlockAudio();

			for (int ID : mRemoveIDs)
			{
				mInstances.erase(ID);
			}
			mRemoveIDs.clear();
			++mChangeCounter;
		}
	}

	void AudioManager::registerAudioMixer(AudioMixer& audioMixer, int parentMixerId)
	{
		const auto it = mAudioMixers.find(audioMixer.mMixerId);
		if (it != mAudioMixers.end() && it->second != &audioMixer)
		{
			AudioMixer* oldMixer = it->second;

			std::swap(audioMixer.mParent, oldMixer->mParent);
			for (AudioMixer*& child : audioMixer.mParent->mChildren)
			{
				if (child == oldMixer)
					child = &audioMixer;
			}

			std::swap(audioMixer.mChildren, oldMixer->mChildren);
			for (AudioMixer* child : audioMixer.mChildren)
			{
				child->mParent = &audioMixer;
			}

			delete oldMixer;
		}

		mAudioMixers[audioMixer.mMixerId] = &audioMixer;

		AudioMixer* parent = getAudioMixerByID(parentMixerId);
		if (nullptr == parent)
			parent = &mRootMixer;
		parent->addChild(audioMixer);
	}

	void AudioManager::mixAudioStatic(void* _userdata, uint8* outputStream, int outputBytes)
	{
		FTX::Audio->mixAudio(outputStream, outputBytes);
	}

	void AudioManager::mixAudio(uint8* outputStream, int outputBytes)
	{
		const size_t outputSamples = outputBytes / (mFormat.channels * sizeof(short));
		const constexpr size_t MAX_SAMPLES = 2048;

		RMX_ASSERT(outputSamples <= MAX_SAMPLES, "Mixing more than " << MAX_SAMPLES << " samples at once is not supported");
		RMX_ASSERT(mFormat.channels <= 2, "More than 2 channels is not supported");

		static int32 fullOutputBuffer[MAX_SAMPLES * 2];
		memset(fullOutputBuffer, 0, sizeof(fullOutputBuffer));

		AudioMixer::MixerParameters parameters;
		parameters.mOutputBuffers[0] = &fullOutputBuffer[0];
		parameters.mOutputBuffers[1] = &fullOutputBuffer[outputSamples];
		parameters.mOutputSamples = outputSamples;
		parameters.mOutputFormat = &mFormat;
		parameters.mAccumulatedVolume = 1.0f;
		mRootMixer.performAudioMix(parameters);

		for (int channelIndex = 0; channelIndex < mFormat.channels; ++channelIndex)
		{
			const int32* RESTRICT src = parameters.mOutputBuffers[channelIndex];
			short* dst = ((short*)outputStream) + channelIndex;

			for (size_t i = 0; i < outputSamples; ++i)
			{
				if (*src >= 0x800000)
				{
					*dst = 0x7fff;
				}
				else if (*src < -0x800000)
				{
					*dst = -0x8000;
				}
				else
				{
					*dst = (*src >> 8);
				}
				++src;
				dst += 2;
			}
		}

		mPlayedSamples += (uint32)outputSamples;

		for (auto& [key, audioInstance] : mInstances)
		{
			if (audioInstance.mPlaybackDone)
			{
				if (!containsElement(mRemoveIDs, key))
				{
					mRemoveIDs.push_back(key);
				}
				++mChangeCounter;
			}
		}
	}

} // namespace rmx
