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
    
    public struct FFTConstants
    {
        public uint fftSize;
        public uint logSize;
        public uint threadGroupSize;
        public uint butterfliesPerThread;
    }

    public struct FFTThread
    {
        public uint offset;
        public uint index;
    }

    public struct FFTThreadContainer
    {
        public uint threadID;
        public uint chunkStart;
        public uint threadGroupSize;
        public uint itemsPerThread;

        public __init(uint threadID, 
            uint groupID,
            uint threadGroupSize,
            uint fftSize)
        {
            this.threadID = threadID;
            this.chunkStart = groupID * fftSize;
            this.threadGroupSize = threadGroupSize;
            this.itemsPerThread = fftSize / threadGroupSize;
        }

        public FFTThread GetThread(uint index)
        {
            FFTThread thread;
            thread.offset = threadID + (index * threadGroupSize);
            thread.index = thread.offset + chunkStart;
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
                FFTThread thread = threadContainer.GetThread(i);
                Complex reversalSample;

                if (direction == FFTDirection::Forward)
                {
                    float timeSample = handle.ReadTime(thread.index);
                    reversalSample = Complex.FromReal(timeSample);
                }
                else // direction == FFTDirection::Inverse
                {
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
                FFTThread thread = threadContainer.GetThread(i);
                Complex sharedSample = handle.ReadShared(thread.offset);

                if (direction == FFTDirection::Forward)
                {
                    handle.WriteFrequency(thread.index, sharedSample);
                }
                else // direction == FFTDirection::Inverse
                {
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
    }
}