module SatyrPhaseVocoder;

import SatyrTypes;
using namespace Satyr;

namespace Satyr
{
    public interface IPVHandle
    {
        public InstrumentData GetInstrumentData();
        public Complex ReadFrequency(uint index);
        public void WriteFrequency(uint index, Complex value);

        public Complex PongReadFrequency(uint index)
        {
            uint pongIndex = GetFrequencyPongIndex<BufferAccessMode::Read>(index, GetInstrumentData());
            return ReadFrequency(pongIndex);
        }

        public void PongWriteFrequency(uint index, Complex value)
        {
            uint pongIndex = GetFrequencyPongIndex<BufferAccessMode::Write>(index, GetInstrumentData());
            WriteFrequency(pongIndex, value);
        }
    }

    public interface IPVEffect
    {
        public void Process<TPVHandle : IPVHandle>(TPVHandle handle,
            uint index,
            uint numBins);

        static Polar ReadPolar<TPVHandle : IPVHandle>(TPVHandle handle,
            uint index)
        {
            return Polar::FromComplex(handle.PongReadFrequency(index));
        }

        static void WritePolar<TPVHandle : IPVHandle>(TPVHandle handle,
            uint index,
            Polar value)
        {
            handle.PongWriteFrequency(index, Complex::FromPolar(value));
        }
    }

    public struct PhaseVocoder<TPVHandle : IPVHandle, TPVEffect : IPVEffect>
    {
        TPVHandle handle;
        TPVEffect effect;
        uint threadIndex;
        uint numBins;

        public  __init(TPVHandle handle,
            TPVEffect effect,
            uint threadIndex,
            uint numBins)
        {
            this.handle = handle;
            this.effect = effect;
            this.threadIndex = threadIndex;
            this.numBins = numBins;
        }

        public void ApplyEffect()
        {
            uint index = threadIndex * SYR_WOLA_HOP_SIZE;

            [unroll(SYR_WOLA_HOP_SIZE)]
                for (uint i = 0; i < SYR_WOLA_HOP_SIZE; i++)
            {
                effect.Process<TPVHandle>(handle, index + i, numBins);
            }
        }
    }

    public struct RobotizationEffect : IPVEffect
    {
        public void Process<TPVHandle : IPVHandle>(TPVHandle handle,
            uint index,
            uint numBins)
        {
            Polar polarBin = ReadPolar<TPVHandle>(handle, index);
            polarBin.phase = 0;
            WritePolar<TPVHandle>(handle, index, polarBin);
        }
    }


}
