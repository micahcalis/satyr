module SatyrTypes;

namespace Satyr
{
    public static const uint SYR_WOLA_HOP_SIZE = 2;

    public enum PongState : bool
    {
        Forwards = false,
        Backwards = true
    }

    public enum BufferAccessMode : bool
    {
        Read = false,
        Write = true
    }

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

        public static Complex FromPolar(Polar p)
        {
            Complex c;
            sincos(p.phase, c.imaginary, c.real);
            c.real *= p.magnitude;
            c.imaginary *= p.magnitude;

            return c;
        }
    }

    public struct Polar
    {
        public float magnitude;
        public float phase;

        public __init(float magnitude, float phase)
        {
            this.magnitude = magnitude;
            this.phase = phase;
        }

        public static Polar FromComplex(Complex c)
        {
            return Polar(length(float2(c.real, c.imaginary)),
                atan2(c.imaginary, c.real));
        }
    }

    public enum AudioSampleMode : uint
    {
        Mono = 1,
        Stereo = 2
    }

    public struct InstrumentData
    {
        public uint64_t totalSamples;
        public uint sampleRate;
        public AudioSampleMode sampleMode;
        public uint paddingSamples;
        public PongState frequencyPongState;
    }

    public inline uint GetFrequencyPongIndex<let accessMode : BufferAccessMode>(uint index,
        InstrumentData instrumentData)
    {
        uint pongBorder = (uint(instrumentData.totalSamples) + instrumentData.paddingSamples);
        pongBorder *= SYR_WOLA_HOP_SIZE;
        uint wrappedIndex = index % pongBorder;

        bool useSecondBuffer = (instrumentData.frequencyPongState == PongState::Backwards)
                            != (accessMode == BufferAccessMode::Write);

        return wrappedIndex + (useSecondBuffer ? pongBorder : 0);
    }
}
