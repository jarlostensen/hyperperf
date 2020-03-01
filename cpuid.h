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
			__cpuid(_regs, leaf);
#else
			__cpuid(leaf, _regs[0], _regs[1], _regs[2], _regs[3]);
#endif
			return *this;
		}

		enum class Register
		{
			eax = 0,
			ebx,
			ecx,
			edx
		};

		int& eax()
		{
			return _regs[0];
		}

		int& ebx()
		{
			return _regs[1];
		}

		int& ecx()
		{
			return _regs[2];
		}

		int& edx()
		{
			return _regs[3];
		}

		int reg(Register r) const
		{
			return _regs[static_cast<size_t>(r)];
		}

		// test register against bit mask
		// returns reg & mask == mask
		bool bits_set(Register r, int mask) const
		{
			return (_regs[static_cast<size_t>(r)] & mask) == mask;
		}

		// returns true if not all of the registers are 0
		bool is_ok() const
		{
			return _regs[0] || _regs[1] || _regs[2] || _regs[3];
		}

		int _regs[4] = { 0 };
	};

}
