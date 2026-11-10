module SatyrFFT;

#define TAU 6.28318530717

namespace Satyr
{
    public struct Complex
    {
        public float real;
        public float imaginary;

        public __init(float real, float imaginary)
        {
            this.real = real;
            this.imaginary = imaginary;
        }

        public static Complex FromReal(float real)
        {
            return Complex(real, 0.0f);
        }

        public static Complex Add(Complex a, Complex b)
        {
            return Complex(a.real + b.real,
                a.imaginary + b.imaginary);
        }

        public static Complex Subtract(Complex a, Complex b)
        {
            return Complex(a.real - b.real,
                a.imaginary - b.imaginary);
        }

        public static Complex Mul(Complex a, Complex b)
        {
            return Complex(a.real * b.real - a.imaginary * b.imaginary,
                a.real * b.imaginary + a.imaginary * b.real);
        }

        public static Complex Conjugate(Complex a)
        {
            return Complex(a.real, -a.imaginary);
        }
    }

    public enum FFTSize : uint
    {
        XS = 128,
        S = 256,
        M = 512,
        L = 1024,
        XL = 2048,
        XXL = 4096,
        XXXL = 8192
    }

    public enum WOLAMode : uint
    {
        None = 0,
        Hann = 1,
        Hamming = 2,
        Blackman = 3
    }

    public struct WOLAConstants
    {
        public float a0;
        public float a1;
        public float a2;

        public __init(float a0, float a1, float a2)
        {
            this.a0 = a0;
            this.a1 = a1;
            this.a2 = a2;
        }
    }

    #define SYR_WOLA_CONSTANTS_HANN WOLAConstants(0.5f, 0.5f, 0)
    #define SYR_WOLA_CONSTANTS_HAMMING WOLAConstants(0.54f, 0.46f, 0)
    #define SYR_WOLA_CONSTANTS_BLACKMAN WOLAConstants(0.42f, 0.50f, 0.08f)

    public struct FFTConstants
    {
        public FFTSize fftSize;
        public uint logSize;
        public uint threadGroupSize;
        public uint butterfliesPerThread;
        public WOLAMode wolaMode;
    }

    public struct FFTThread
    {
        public uint offset;
        public uint index;
    }

    public enum FFTDomain : bool
    {
        Time = false,
        Frequency = true
    }

    public enum WOLADirection : bool
    {
        Expand = false,
        Add = true
    }

    #define SYR_WOLA_HOP_SIZE 2

    public struct FFTThreadContainer
    {
        public uint threadID;
        public uint chunkStart;
        public uint threadGroupSize;
        public FFTSize fftSize;
        public uint hopSize;
        public uint itemsPerThread;
        public uint timeExpandBorder;

        public __init(uint threadID,
            uint groupID,
            uint threadGroupSize,
            FFTSize fftSize,
            uint totalSamples,
            uint paddedSamples)
        {
            this.threadID = threadID;
            this.chunkStart = groupID * (uint)fftSize;
            this.threadGroupSize = threadGroupSize;
            this.fftSize = fftSize;
            this.hopSize = fftSize / SYR_WOLA_HOP_SIZE;
            this.itemsPerThread = (uint)fftSize / threadGroupSize;
            this.timeExpandBorder = totalSamples + paddedSamples;
        }

        public FFTThread GetLinearThread(uint index)
        {
            FFTThread thread;
            thread.offset = threadID + (index * threadGroupSize);
            thread.index = thread.offset + chunkStart;

            return thread;
        }

        public FFTThread GetUnmixedLinearThread(uint index)
        {
            FFTThread thread = GetLinearThread(index);
            thread.index += timeExpandBorder;

            return thread;
        }

        public FFTThread GetUnmixedGatherThread(uint index, uint overlapLayer)
        {
            FFTThread thread;
            thread.offset = (index % hopSize) + (overlapLayer * hopSize);
            uint currentFrame = index / hopSize;
            uint frameIndex = (currentFrame > overlapLayer) ? (currentFrame - overlapLayer) : 0;
            thread.index = timeExpandBorder + (frameIndex * (uint)fftSize) + thread.offset;

            return thread;
        }
    }

    public enum FFTDirection : bool
    {
        Forward = false,
        Inverse = true
    }

    public interface IFFTHandle
    {
        public Complex ReadShared(uint index);
        public void WriteShared(uint index, Complex value);
        public float ReadTime(uint index);
        public void WriteTime(uint index, float value);
        public Complex ReadFrequency(uint index);
        public void WriteFrequency(uint index, Complex value);
    }

    #define SYR_WOLA_UNROLL_LOOP [unroll(SYR_WOLA_HOP_SIZE)] for (uint i = 0; i < SYR_WOLA_HOP_SIZE; i++)

    public struct WOLAMixer<TFFTHandle : IFFTHandle>
    {
        private TFFTHandle handle;
        private FFTConstants constants;
        private WOLADirection direction;
        private FFTThreadContainer threadContainer;

        public __init(TFFTHandle handle,
            FFTConstants constants,
            WOLADirection direction,
            FFTThreadContainer threadContainer)
        {
            this.handle = handle;
            this.constants = constants;
            this.direction = direction;
            this.threadContainer = threadContainer;
        }

        public void Mix()
        {
            for (uint i = 0; i < threadContainer.itemsPerThread; i++)
            {
                FFTThread mixedThread = threadContainer.GetLinearThread(i);

                if (mixedThread.index >= threadContainer.timeExpandBorder) continue;

                FFTThread[SYR_WOLA_HOP_SIZE] unmixedThreads = GetGatherThreads(mixedThread.index);

                if (direction == WOLADirection::Expand)
                {
                    Expand(mixedThread, unmixedThreads);
                }
                else // direction == WOLADirection::Add
                {
                    OverlapAdd(mixedThread, unmixedThreads);
                }
            }
        }

        private FFTThread[SYR_WOLA_HOP_SIZE] GetGatherThreads(uint offsetIndex)
        {
            FFTThread[SYR_WOLA_HOP_SIZE] gatherThreads;

            SYR_WOLA_UNROLL_LOOP
            {
                gatherThreads[i] = threadContainer.GetUnmixedGatherThread(offsetIndex, i);
            }

            return gatherThreads;
        }

        private void Expand(FFTThread mixedThread, FFTThread[SYR_WOLA_HOP_SIZE] unmixedThreads)
        {
            float originalSample = handle.ReadTime(mixedThread.index);

            SYR_WOLA_UNROLL_LOOP
            {
                FFTThread currentThread = unmixedThreads[i];

                float wolaWeight = Functions::GetWOLAMultiplier(currentThread.offset,
                    constants.fftSize,
                    constants.wolaMode);

                float weightedSample = originalSample * wolaWeight;
                handle.WriteTime(currentThread.index, weightedSample);
            }
        }

        private void OverlapAdd(FFTThread mixedThread, FFTThread[SYR_WOLA_HOP_SIZE] unmixedThreads)
        {
            float addedSample = 0;

            SYR_WOLA_UNROLL_LOOP
            {
                FFTThread currentThread = unmixedThreads[i];

                float wolaWeight = Functions::GetWOLAMultiplier(currentThread.offset,
                    constants.fftSize,
                    constants.wolaMode);

                float weightedSample = handle.ReadTime(currentThread.index) * wolaWeight;
                addedSample += weightedSample;
            }

            handle.WriteTime(mixedThread.index, addedSample);
        }
    }

    public struct FFTComputer<TFFTHandle : IFFTHandle>
    {
        private TFFTHandle handle;
        private FFTConstants constants;
        private FFTDirection direction;
        private FFTThreadContainer threadContainer;

        public __init(TFFTHandle handle,
            FFTConstants constants,
            FFTDirection direction,
            FFTThreadContainer threadContainer)
        {
            this.handle = handle;
            this.constants = constants;
            this.direction = direction;
            this.threadContainer = threadContainer;
        }

        public void Compute()
        {
            ReversalReorder();
            WindSignal();
            WriteFFT();
        }

        private void ReversalReorder()
        {
            uint shiftOffset = Functions.GetBitShift(constants.logSize);

            for (uint i = 0; i < threadContainer.itemsPerThread; i++)
            {
                FFTThread thread;
                Complex reversalSample;

                if (direction == FFTDirection::Forward)
                {
                    thread = threadContainer.GetUnmixedLinearThread(i);
                    float timeSample = handle.ReadTime(thread.index);
                    reversalSample = Complex.FromReal(timeSample);
                }
                else // direction == FFTDirection::Inverse
                {
                    thread = threadContainer.GetLinearThread(i);
                    reversalSample = handle.ReadFrequency(thread.index);
                }

                uint reversedIndex = Functions.BitReverse(thread.offset, shiftOffset);
                handle.WriteShared(reversedIndex, reversalSample);
            }

            GroupMemoryBarrierWithGroupSync();
        }

        private void WindSignal()
        {
            for (uint i = 1; i <= constants.logSize; i++)
            {
                uint chunkSize = 1u << i;
                uint halfChunk = chunkSize >> 1;

                for (uint j = 0; j < constants.butterfliesPerThread; j++)
                {
                    uint butterflyID = threadContainer.threadID + (j * threadContainer.threadGroupSize);
                    uint chunkID = butterflyID / halfChunk;
                    uint offset   = butterflyID % halfChunk;

                    uint topID = (chunkID * chunkSize) + offset;
                    uint bottomID = topID + halfChunk;

                    Complex topVal = handle.ReadShared(topID);
                    Complex bottomVal = handle.ReadShared(bottomID);
                    Complex twiddle = Functions.GetTwiddle((float)offset, (float)chunkSize, direction);
                    Complex rotatedBottom = Complex::Mul(bottomVal, twiddle);

                    handle.WriteShared(topID, Complex::Add(topVal, rotatedBottom));
                    handle.WriteShared(bottomID, Complex::Subtract(topVal, rotatedBottom));
                }

                GroupMemoryBarrierWithGroupSync();
            }
        }

        private void WriteFFT()
        {
            for (uint i = 0; i < threadContainer.itemsPerThread; i++)
            {
                FFTThread thread;
                Complex sharedSample;

                if (direction == FFTDirection::Forward)
                {
                    thread = threadContainer.GetLinearThread(i);
                    sharedSample = handle.ReadShared(thread.offset);
                    handle.WriteFrequency(thread.index, sharedSample);
                }
                else // direction == FFTDirection::Inverse
                {
                    thread = threadContainer.GetUnmixedLinearThread(i);
                    sharedSample = handle.ReadShared(thread.offset);
                    float scaledTime = sharedSample.real / (float)constants.fftSize;
                    handle.WriteTime(thread.index, scaledTime);
                }
            }
        }
    }

    namespace Functions
    {
        uint GetBitShift(uint logSize)
        {
            return 32 - logSize;
        }

        uint BitReverse(uint value, uint shift)
        {
            return reversebits(value) >> shift;
        }

        Complex GetTwiddle(float offset, float chunkSize, FFTDirection direction)
        {
            float sign = (bool)direction ? 1.0f : -1.0f;
            float angle = (sign * TAU * offset) / chunkSize;
            Complex twiddle;
            sincos(angle, twiddle.imaginary, twiddle.real);
            return twiddle;
        }

        float GetWOLAMultiplier(uint index,
            FFTSize fftSize,
            WOLAMode wolaMode)
        {
            if (fftSize <= 1) return 0.5f;

            WOLAConstants constants;
            switch (wolaMode)
            {
                case WOLAMode::Hann: constants = SYR_WOLA_CONSTANTS_HANN; break;
                case WOLAMode::Hamming: constants = SYR_WOLA_CONSTANTS_HAMMING; break;
                case WOLAMode::Blackman: constants = SYR_WOLA_CONSTANTS_BLACKMAN; break;

                case WOLAMode::None:
                default: return 1.0f;
            }

            // NOTE: Cosine Window Function (k = 2)
            float n = (float)index;
            float N = (float)(uint)fftSize;
            float shift = (TAU * n) / N;

            float cos1 = cos(shift);
            float cos2 = cos(2.0f * shift);

            return sqrt(max(0.0f, constants.a0
                - (constants.a1 * cos1)
                + (constants.a2 * cos2)));
        }
    }
}
