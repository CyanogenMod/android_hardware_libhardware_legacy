/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <hardware_legacy/AudioPolicyInterface.h>


namespace android_audio_legacy {
    using android::KeyedVector;

// ----------------------------------------------------------------------------

#define MAX_DEVICE_ADDRESS_LEN 20
// Attenuation applied to STRATEGY_SONIFICATION streams when a headset is connected: 6dB
#define SONIFICATION_HEADSET_VOLUME_FACTOR 0.5
// Min volume for STRATEGY_SONIFICATION streams when limited by music volume: -36dB
#define SONIFICATION_HEADSET_VOLUME_MIN  0.016
// Time in milliseconds during which we consider that music is still active after a music
// track was stopped - see computeVolume()
#define SONIFICATION_HEADSET_MUSIC_DELAY  5000
// Time in milliseconds during witch some streams are muted while the audio path
// is switched
#define MUTE_TIME_MS 2000

#define NUM_TEST_OUTPUTS 5

#define NUM_VOL_CURVE_KNEES 2

// ----------------------------------------------------------------------------
// AudioPolicyManagerBase implements audio policy manager behavior common to all platforms.
// Each platform must implement an AudioPolicyManager class derived from AudioPolicyManagerBase
// and override methods for which the platform specific behavior differs from the implementation
// in AudioPolicyManagerBase. Even if no specific behavior is required, the AudioPolicyManager
// class must be implemented as well as the class factory function createAudioPolicyManager()
// and provided in a shared library libaudiopolicy.so.
// ----------------------------------------------------------------------------

class AudioPolicyManagerBase: public AudioPolicyInterface
#ifdef AUDIO_POLICY_TEST
    , public Thread
#endif //AUDIO_POLICY_TEST
{

public:
                AudioPolicyManagerBase(AudioPolicyClientInterface *clientInterface);
        virtual ~AudioPolicyManagerBase();

        // AudioPolicyInterface
        virtual status_t setDeviceConnectionState(AudioSystem::audio_devices device,
                                                          AudioSystem::device_connection_state state,
                                                          const char *device_address);
        virtual AudioSystem::device_connection_state getDeviceConnectionState(AudioSystem::audio_devices device,
                                                                              const char *device_address);
        virtual void setPhoneState(int state);
        virtual void setRingerMode(uint32_t mode, uint32_t mask);
        virtual void setForceUse(AudioSystem::force_use usage, AudioSystem::forced_config config);
        virtual AudioSystem::forced_config getForceUse(AudioSystem::force_use usage);
        virtual void setSystemProperty(const char* property, const char* value);
        virtual status_t initCheck();
        virtual audio_io_handle_t getOutput(AudioSystem::stream_type stream,
                                            uint32_t samplingRate = 0,
                                            uint32_t format = AudioSystem::FORMAT_DEFAULT,
                                            uint32_t channels = 0,
                                            AudioSystem::output_flags flags =
                                                    AudioSystem::OUTPUT_FLAG_INDIRECT);
        virtual status_t startOutput(audio_io_handle_t output,
                                     AudioSystem::stream_type stream,
                                     int session = 0);
        virtual status_t stopOutput(audio_io_handle_t output,
                                    AudioSystem::stream_type stream,
                                    int session = 0);
        virtual void releaseOutput(audio_io_handle_t output);
        virtual audio_io_handle_t getInput(int inputSource,
                                            uint32_t samplingRate,
                                            uint32_t format,
                                            uint32_t channels,
                                            AudioSystem::audio_in_acoustics acoustics);
        // indicates to the audio policy manager that the input starts being used.
        virtual status_t startInput(audio_io_handle_t input);
        // indicates to the audio policy manager that the input stops being used.
        virtual status_t stopInput(audio_io_handle_t input);
        virtual void releaseInput(audio_io_handle_t input);
        virtual void initStreamVolume(AudioSystem::stream_type stream,
                                                    int indexMin,
                                                    int indexMax);
        virtual status_t setStreamVolumeIndex(AudioSystem::stream_type stream, int index);
        virtual status_t getStreamVolumeIndex(AudioSystem::stream_type stream, int *index);

        // return the strategy corresponding to a given stream type
        virtual uint32_t getStrategyForStream(AudioSystem::stream_type stream);

        // return the enabled output devices for the given stream type
        virtual uint32_t getDevicesForStream(AudioSystem::stream_type stream);

        virtual audio_io_handle_t getOutputForEffect(effect_descriptor_t *desc);
        virtual status_t registerEffect(effect_descriptor_t *desc,
                                        audio_io_handle_t io,
                                        uint32_t strategy,
                                        int session,
                                        int id);
        virtual status_t unregisterEffect(int id);

        virtual bool isStreamActive(int stream, uint32_t inPastMs = 0) const;

        virtual status_t dump(int fd);

protected:

        enum routing_strategy {
            STRATEGY_MEDIA,
            STRATEGY_PHONE,
            STRATEGY_SONIFICATION,
            STRATEGY_DTMF,
            NUM_STRATEGIES
        };

        // descriptor for audio outputs. Used to maintain current configuration of each opened audio output
        // and keep track of the usage of this output by each audio stream type.
        class AudioOutputDescriptor
        {
        public:
            AudioOutputDescriptor();

            status_t    dump(int fd);

            uint32_t device();
            void changeRefCount(AudioSystem::stream_type, int delta);
            uint32_t refCount();
            uint32_t strategyRefCount(routing_strategy strategy);
            bool isUsedByStrategy(routing_strategy strategy) { return (strategyRefCount(strategy) != 0);}
            bool isDuplicated() { return (mOutput1 != NULL && mOutput2 != NULL); }

            audio_io_handle_t mId;              // output handle
            uint32_t mSamplingRate;             //
            uint32_t mFormat;                   //
            uint32_t mChannels;                 // output configuration
            uint32_t mLatency;                  //
            AudioSystem::output_flags mFlags;   //
            uint32_t mDevice;                   // current device this output is routed to
            uint32_t mRefCount[AudioSystem::NUM_STREAM_TYPES]; // number of streams of each type using this output
            nsecs_t mStopTime[AudioSystem::NUM_STREAM_TYPES];
            AudioOutputDescriptor *mOutput1;    // used by duplicated outputs: first output
            AudioOutputDescriptor *mOutput2;    // used by duplicated outputs: second output
            float mCurVolume[AudioSystem::NUM_STREAM_TYPES];   // current stream volume
            int mMuteCount[AudioSystem::NUM_STREAM_TYPES];     // mute request counter
        };

        // descriptor for audio inputs. Used to maintain current configuration of each opened audio input
        // and keep track of the usage of this input.
        class AudioInputDescriptor
        {
        public:
            AudioInputDescriptor();

            status_t    dump(int fd);

            uint32_t mSamplingRate;                     //
            uint32_t mFormat;                           // input configuration
            uint32_t mChannels;                         //
            AudioSystem::audio_in_acoustics mAcoustics; //
            uint32_t mDevice;                           // current device this input is routed to
            uint32_t mRefCount;                         // number of AudioRecord clients using this output
            int      mInputSource;                     // input source selected by application (mediarecorder.h)
        };

        // stream descriptor used for volume control
        class StreamDescriptor
        {
        public:
            StreamDescriptor()
            :   mIndexMin(0), mIndexMax(1), mIndexCur(1), mCanBeMuted(true) {}

            void dump(char* buffer, size_t size);

            int mIndexMin;      // min volume index
            int mIndexMax;      // max volume index
            int mIndexCur;      // current volume index
            bool mCanBeMuted;   // true is the stream can be muted

            // 4 points to define the volume attenuation curve, each characterized by the volume
            // index (from 0 to 100) at which they apply, and the attenuation in dB at that index.
            int mVolIndex[NUM_VOL_CURVE_KNEES+2];   // minimum index, index at knees, and max index
            float mVolDbAtt[NUM_VOL_CURVE_KNEES+2]; // largest attenuation, attenuation at knees,
                                                    //     and attenuation at max vol (usually 0dB)
            // indices in mVolIndex and mVolDbAtt respectively for points at lowest volume, knee 1,
            //    knee 2 and highest volume.
            enum { VOLMIN = 0, VOLKNEE1 = 1, VOLKNEE2 = 2, VOLMAX = 3 };
        };

        // stream descriptor used for volume control
        class EffectDescriptor
        {
        public:

            status_t dump(int fd);

            int mIo;                // io the effect is attached to
            routing_strategy mStrategy; // routing strategy the effect is associated to
            int mSession;               // audio session the effect is on
            effect_descriptor_t mDesc;  // effect descriptor
        };

        void addOutput(audio_io_handle_t id, AudioOutputDescriptor *outputDesc);

        // return the strategy corresponding to a given stream type
        static routing_strategy getStrategy(AudioSystem::stream_type stream);
        // return appropriate device for streams handled by the specified strategy according to current
        // phone state, connected devices...
        // if fromCache is true, the device is returned from mDeviceForStrategy[], otherwise it is determined
        // by current state (device connected, phone state, force use, a2dp output...)
        // This allows to:
        //  1 speed up process when the state is stable (when starting or stopping an output)
        //  2 access to either current device selection (fromCache == true) or
        // "future" device selection (fromCache == false) when called from a context
        //  where conditions are changing (setDeviceConnectionState(), setPhoneState()...) AND
        //  before updateDeviceForStrategy() is called.
        virtual uint32_t getDeviceForStrategy(routing_strategy strategy, bool fromCache = true);
        // change the route of the specified output
        void setOutputDevice(audio_io_handle_t output, uint32_t device, bool force = false, int delayMs = 0);
        // select input device corresponding to requested audio source
        virtual uint32_t getDeviceForInputSource(int inputSource);
        // return io handle of active input or 0 if no input is active
        audio_io_handle_t getActiveInput();
        virtual void initializeVolumeCurves();
        // compute the actual volume for a given stream according to the requested index and a particular
        // device
        virtual float computeVolume(int stream, int index, audio_io_handle_t output, uint32_t device);
        // check that volume change is permitted, compute and send new volume to audio hardware
        status_t checkAndSetVolume(int stream, int index, audio_io_handle_t output, uint32_t device, int delayMs = 0, bool force = false);
        // apply all stream volumes to the specified output and device
        void applyStreamVolumes(audio_io_handle_t output, uint32_t device, int delayMs = 0, bool force = false);
        // Mute or unmute all streams handled by the specified strategy on the specified output
        void setStrategyMute(routing_strategy strategy, bool on, audio_io_handle_t output, int delayMs = 0);
        // Mute or unmute the stream on the specified output
        void setStreamMute(int stream, bool on, audio_io_handle_t output, int delayMs = 0);
        // handle special cases for sonification strategy while in call: mute streams or replace by
        // a special tone in the device used for communication
        void handleIncallSonification(int stream, bool starting, bool stateChange);
        // true is current platform implements a back microphone
        virtual bool hasBackMicrophone() const { return false; }
        // true if device is in a telephony or VoIP call
        virtual bool isInCall();
        // true if given state represents a device in a telephony or VoIP call
        virtual bool isStateInCall(int state);

#ifdef WITH_A2DP
        // true is current platform supports suplication of notifications and ringtones over A2DP output
        virtual bool a2dpUsedForSonification() const { return true; }
        status_t handleA2dpConnection(AudioSystem::audio_devices device,
                                                            const char *device_address);
        status_t handleA2dpDisconnection(AudioSystem::audio_devices device,
                                                            const char *device_address);
        void closeA2dpOutputs();
        // checks and if necessary changes output (a2dp, duplicated or hardware) used for all strategies.
        // must be called every time a condition that affects the output choice for a given strategy is
        // changed: connected device, phone state, force use...
        // Must be called before updateDeviceForStrategy()
        void checkOutputForStrategy(routing_strategy strategy);
        // Same as checkOutputForStrategy() but for a all strategies in order of priority
        void checkOutputForAllStrategies();
        // manages A2DP output suspend/restore according to phone state and BT SCO usage
        void checkA2dpSuspend();
#endif
        // selects the most appropriate device on output for current state
        // must be called every time a condition that affects the device choice for a given output is
        // changed: connected device, phone state, force use, output start, output stop..
        // see getDeviceForStrategy() for the use of fromCache parameter
        uint32_t getNewDevice(audio_io_handle_t output, bool fromCache = true);
        // updates cache of device used by all strategies (mDeviceForStrategy[])
        // must be called every time a condition that affects the device choice for a given strategy is
        // changed: connected device, phone state, force use...
        // cached values are used by getDeviceForStrategy() if parameter fromCache is true.
         // Must be called after checkOutputForAllStrategies()
        void updateDeviceForStrategy();
        // true if current platform requires a specific output to be opened for this particular
        // set of parameters. This function is called by getOutput() and is implemented by platform
        // specific audio policy manager.
        virtual bool needsDirectOuput(AudioSystem::stream_type stream,
                                    uint32_t samplingRate,
                                    uint32_t format,
                                    uint32_t channels,
                                    AudioSystem::output_flags flags,
                                    uint32_t device);
        virtual uint32_t getMaxEffectsCpuLoad();
        virtual uint32_t getMaxEffectsMemory();
#ifdef AUDIO_POLICY_TEST
        virtual     bool        threadLoop();
                    void        exit();
        int testOutputIndex(audio_io_handle_t output);
#endif //AUDIO_POLICY_TEST

        AudioPolicyClientInterface *mpClientInterface;  // audio policy client interface
        audio_io_handle_t mHardwareOutput;              // hardware output handler
        audio_io_handle_t mA2dpOutput;                  // A2DP output handler
        audio_io_handle_t mDuplicatedOutput;            // duplicated output handler: outputs to hardware and A2DP.

        KeyedVector<audio_io_handle_t, AudioOutputDescriptor *> mOutputs;   // list of output descriptors
        KeyedVector<audio_io_handle_t, AudioInputDescriptor *> mInputs;     // list of input descriptors
        uint32_t mAvailableOutputDevices;                                   // bit field of all available output devices
        uint32_t mAvailableInputDevices;                                    // bit field of all available input devices
        int mPhoneState;                                                    // current phone state
        uint32_t                 mRingerMode;                               // current ringer mode
        AudioSystem::forced_config mForceUse[AudioSystem::NUM_FORCE_USE];   // current forced use configuration

        StreamDescriptor mStreams[AudioSystem::NUM_STREAM_TYPES];           // stream descriptors for volume control
        String8 mA2dpDeviceAddress;                                         // A2DP device MAC address
        String8 mScoDeviceAddress;                                          // SCO device MAC address
        bool    mLimitRingtoneVolume;                                       // limit ringtone volume to music volume if headset connected
        uint32_t mDeviceForStrategy[NUM_STRATEGIES];
        float   mLastVoiceVolume;                                           // last voice volume value sent to audio HAL

        // Maximum CPU load allocated to audio effects in 0.1 MIPS (ARMv5TE, 0 WS memory) units
        static const uint32_t MAX_EFFECTS_CPU_LOAD = 1000;
        // Maximum memory allocated to audio effects in KB
        static const uint32_t MAX_EFFECTS_MEMORY = 512;
        uint32_t mTotalEffectsCpuLoad; // current CPU load used by effects
        uint32_t mTotalEffectsMemory;  // current memory used by effects
        KeyedVector<int, EffectDescriptor *> mEffects;  // list of registered audio effects
        bool    mA2dpSuspended;  // true if A2DP output is suspended

#ifdef AUDIO_POLICY_TEST
        Mutex   mLock;
        Condition mWaitWorkCV;

        int             mCurOutput;
        bool            mDirectOutput;
        audio_io_handle_t mTestOutputs[NUM_TEST_OUTPUTS];
        int             mTestInput;
        uint32_t        mTestDevice;
        uint32_t        mTestSamplingRate;
        uint32_t        mTestFormat;
        uint32_t        mTestChannels;
        uint32_t        mTestLatencyMs;
#endif //AUDIO_POLICY_TEST

private:
        static float volIndexToAmpl(uint32_t device, const StreamDescriptor& streamDesc,
                int indexInUi);
};

};
