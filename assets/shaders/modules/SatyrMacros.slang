#ifndef SYRINX_MACROS_INCLUDED
#define SYRINX_MACROS_INCLUDED

import SatyrCore;
import SatyrFFT;

#define SYR_NOTES_DATA [[vk::binding(0, 0)]] cbuffer ShaderParams
#define SYR_INSTRUMENT_DATA [[vk::binding(1, 0)]] StructuredBuffer<Satyr.InstrumentData> _InstrumentData;
#define SYR_MASTER_BUFFER [[vk::binding(2, 0)]] RWStructuredBuffer<float> _MasterTime; [[vk::binding(3, 0)]] RWStructuredBuffer<Satyr.Complex> _MasterFrequency; 
#define SYR_INSTRUMENT_BUFFER(name, index) [[vk::binding(index * 2 + 4, 0)]] RWStructuredBuffer<float> name##Time; [[vk::binding(index * 2 + 4 + 1, 0)]] RWStructuredBuffer<Satyr.Complex> name##Frequency;
#define SYR_FFT_CONSTANTS [[vk::push_constant()]] Satyr.FFTConstants _STFTConstants;
#define SYR_FFT_SHARED(fftSize) groupshared Satyr.Complex[(uint)fftSize] _STFTShared;

#define SYR_TG_SIZE_S  [numthreads(8, 1, 1)]  [shader("compute")]
#define SYR_TG_SIZE_M  [numthreads(16, 1, 1)] [shader("compute")]
#define SYR_TG_SIZE_L  [numthreads(32, 1, 1)] [shader("compute")]
#define SYR_TG_SIZE_XL [numthreads(64, 1, 1)] [shader("compute")]

#endif