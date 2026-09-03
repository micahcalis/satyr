module SatyrCore;

namespace Satyr
{
    public enum AudioSampleMode : uint
    {
        Mono = 1,
        Stereo = 2
    }

    public struct InstrumentData
    {
        public uint64_t totalSamples;
        public uint sampleRate;
        AudioSampleMode sampleMode;
    }
}
