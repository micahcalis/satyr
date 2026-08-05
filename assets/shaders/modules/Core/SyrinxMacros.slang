#ifndef SYRINX_MACROS_INCLUDED
#define SYRINX_MACROS_INCLUDED

#define SYR_SHADER_PARAMS [[vk::binding(0, 0)]] cbuffer ShaderParams
#define SYR_RW_BUFFER(name, type, idx) [[vk::binding(idx, 0)]] RWStructuredBuffer<type> name;
#define SYR_COMPUTE_KERNEL(index, threadgroupsize) [numthreads(threadgroupsize, 1, 1)] [shader("compute")] void SyrKernel_##index(uint3 id : SV_DispatchThreadID)

#define SYR_TG_SIZE_S 8
#define SYR_TG_SIZE_M 16
#define SYR_TG_SIZE_L 32
#define SYR_TG_SIZE_XL 64

#endif