module SatyrCore;

import SatyrTypes;
import SatyrFFT;

namespace Satyr
{
    public enum InstrumentDataSlot : uint
    {
        Master = 0,
        Instrument0 = 1,
        Instrument1 = 2,
        Instrument2 = 3,
        Instrument3 = 4,
        Instrument4 = 5,
        Instrument5 = 6,
        Instrument6 = 7,
        Instrument7 = 8,
        Instrument8 = 9,
        Instrument9 = 12,
        Instrument10 = 11,
        Instrument11 = 12,
        Instrument12 = 13
    }

    public enum FFTKernelPhase : uint
    {
        ExpandTime = 0,
        ForwardFFT = 1,
        InverseFFT = 2,
        OverlapAddTime = 3
    }

    public struct DSPEngineData
    {
        public uint localID;
        public uint groupID;
        public FFTConstants stftConstants;
        public InstrumentData instrumentData;
    }

    public struct DSPEngine<TFFTHandle : IFFTHandle,
        let kernelPhase : FFTKernelPhase>
    {
        private TFFTHandle handle;
        private FFTThreadContainer threadContainer;
        private WOLAMixer<TFFTHandle> wolaMixer;
        private FFTComputer<TFFTHandle> fftComputer;

        public __init(TFFTHandle handle, DSPEngineData data)
        {
            this.handle = handle;
            InitializeThreadContainer(data);

            if (NeedsMixer(kernelPhase))
            {
                InitializeMixer(data);
            }

            if (NeedsComputer(kernelPhase))
            {
                InitializeComputer(data);
            }
        }

        public void ExecutePhase()
        {
            if (NeedsMixer(kernelPhase))
            {
                wolaMixer.Mix();
            }

            if (NeedsComputer(kernelPhase))
            {
                fftComputer.Compute();
            }
        }

        [mutating] private void InitializeThreadContainer(DSPEngineData data)
        {
            threadContainer = FFTThreadContainer(data.localID,
                data.groupID,
                data.stftConstants.threadGroupSize,
                data.stftConstants.fftSize,
                data.instrumentData);
        }

        [mutating] private void InitializeMixer(DSPEngineData data)
        {
            WOLADirection mixerDirection = kernelPhase == FFTKernelPhase::ExpandTime
                ? WOLADirection::Expand : WOLADirection::Add;

            wolaMixer = WOLAMixer<TFFTHandle>(handle,
                data.stftConstants,
                mixerDirection,
                threadContainer);
        }

        [mutating] private void InitializeComputer(DSPEngineData data)
        {
            FFTDirection fftDirection = kernelPhase == FFTKernelPhase::ForwardFFT
                ? FFTDirection::Forward : FFTDirection::Inverse;

            fftComputer = FFTComputer<TFFTHandle>(handle,
                data.stftConstants,
                fftDirection,
                threadContainer);
        }

        private inline static bool NeedsMixer(FFTKernelPhase kernelPhase)
        {
            return kernelPhase == FFTKernelPhase::ExpandTime
                || kernelPhase == FFTKernelPhase::OverlapAddTime;
        }

        private inline static bool NeedsComputer(FFTKernelPhase kernelPhase)
        {
            return kernelPhase == FFTKernelPhase::ForwardFFT
                || kernelPhase == FFTKernelPhase::InverseFFT;
        }
    }
}
