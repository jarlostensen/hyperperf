#pragma once

namespace system_info
{

#ifdef _WIN32
#include <intrin.h>
#else
	static void __cpuid(int info[4], int infoType) {
		__asm__ __volatile__("cpuid" : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3]) : "0"(infoType));
	}

	/* Save %ebx in case it's the PIC register */
	static void __cpuidex(int info[4], int level, int count) {
		__asm__ __volatile__("xchg{l}\t{%%}ebx, %1\n\t"
							"cpuid\n\t"
							"xchg{l}\t{%%}ebx, %1\n\t"
							: "=a"(info[0]), "=r"(info[1]), "=c"(info[2]), "=d"(info[3])
							: "0"(level), "2"(count));
	}	
#endif

	
    // cross platform convenience wrapper for cpuid
    // usage:
    // cpuid cpu_id{1};
    // ...use cpu_id.eax()/ebx()/ecx() etc.
    // cpu_id = 2;
    // ...use cpu_id.eax()/ebx()/ecx() etc.
    // 
    //
    struct cpuid
    {		
        cpuid() = default;

        explicit cpuid(int leaf)
        {
            this->operator=(leaf);
        }

        cpuid& operator=(int leaf)
        {
            __cpuid(reinterpret_cast<int*>(_regs), leaf);
            return *this;
        }

		cpuid& operator=(std::pair<int,int>&& leafSubLeaf)
		{
            __cpuidex(reinterpret_cast<int*>(_regs), leafSubLeaf.first, leafSubLeaf.second);
			return *this;
		}

        void clear_regs()
        {
            memset(_regs, 0, sizeof(_regs));
        }

        enum class regs
        {
            eax = 0,
            ebx,
            ecx,
            edx
        };

        unsigned int& eax()
        {
            return _regs[0];
        }

        unsigned int& ebx()
        {
            return _regs[1];
        }

        unsigned int& ecx()
        {
            return _regs[2];
        }

        unsigned int& edx()
        {
            return _regs[3];
        }

        unsigned int reg(regs r) const
        {
            return _regs[static_cast<size_t>(r)];
        }
		
		// Intel notation; [MSB:LSB]
		unsigned int extract_reg_field(regs r, size_t startBitIndex, size_t endBitIndex) const
		{
			const auto rval = _regs[static_cast<size_t>(r)];
			return ((rval >> startBitIndex) & ((1<<(endBitIndex+1))-1));
		}

        // test register against bit mask
        // returns reg & mask
        bool bits_set(regs r, unsigned int mask) const
        {
            return (_regs[static_cast<size_t>(r)] & mask) != 0;
        }

        // returns true if not all of the registers are 0
        bool is_ok() const
        {
            return _regs[0] || _regs[1] || _regs[2] || _regs[3];
        }

        unsigned int _regs[4] = { 0 };
    };

}
