#pragma once

namespace system_info
{

#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
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
#ifdef _WIN32
            __cpuid(reinterpret_cast<int*>(_regs), leaf);
#else
            // see for example http://newbiz.github.io/cpp/2010/12/20/Playing-with-cpuid.html
            asm volatile (
                "cpuid"
                : "=a"(_regs[0]), "=b"(_regs[1]), "=c"(_regs[2]), "=d"(_regs[3])
                : "a"(leaf)
                : "b"
                );

            uint32_t eax, ebx, ecx, edx;
# if defined( __i386__ ) && defined ( __PIC__ )
            /* in case of PIC under 32-bit EBX cannot be clobbered */
            __asm__ volatile ("movl %%ebx, %%edi \n\t cpuid \n\t xchgl %%ebx, %%edi" : "=D" (ebx),
# else
            __asm__ volatile ("cpuid" : "+r" (ebx),
# endif
                "+a" (eax), "+c" (ecx), "=d" (edx));

            _regs[0] = eax; _regs[1] = ebx; _regs[2] = ecx; _regs[3] = edx;

#endif
            return *this;
        }

        void clear_regs()
        {
            memset(_regs, 0, sizeof(_regs));
        }

        enum class Register
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

        unsigned int reg(Register r) const
        {
            return _regs[static_cast<size_t>(r)];
        }

        // test register against bit mask
        // returns reg & mask
        bool bits_set(Register r, unsigned int mask) const
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
