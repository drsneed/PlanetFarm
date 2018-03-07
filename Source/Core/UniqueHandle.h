#pragma once
#include "StdIncludes.h"
#include "DebugTools.h"

template<typename T>
class HandleTraits
{
public:
	using Traits = T;
	static auto Invalid() throw() -> Traits
	{
		return nullptr;
	}
};

template <typename Traits>
class UniqueHandle
{

	using Pointer = decltype(Traits::Invalid());
	Pointer m_value;

	auto _Close() noexcept -> void
	{
		if (*this)
		{
			Traits::Close(m_value);
		}
	}


public:
	UniqueHandle(const UniqueHandle&) = delete;
	UniqueHandle& operator=(const UniqueHandle&) = delete;

	explicit UniqueHandle(Pointer value = Traits::Invalid()) noexcept
		: m_value{value}
	{
		
	}

	UniqueHandle(UniqueHandle&& other) noexcept
		: m_value{ other.Release() }
	{
		
	}

	UniqueHandle& operator=(UniqueHandle&& other) noexcept
	{
		if (this != &other)
		{
			Reset(other.Release());
		}

		return *this;
	}

	auto Get() const noexcept -> Pointer
	{
		return m_value;
	}

	Pointer* Set() noexcept
	{
		ASSERT(!*this);
		return &m_value;
	}

	auto GetAddrOf() -> Pointer*
	{
		ASSERT(!*this);
		return &m_value;
	}

	~UniqueHandle() noexcept
	{
		_Close();
	}

	explicit operator bool() const noexcept
	{
		return m_value != Traits::Invalid();
	}

	auto Release() noexcept -> Pointer
	{
		auto value = m_value;
		m_value = Traits::Invalid();
		return value;
	}

	auto Reset(Pointer value = Traits::Invalid()) noexcept -> bool
	{
		if (m_value != value)
		{
			_Close();
			m_value = value;
		}

		return static_cast<bool>(*this);
	}

	auto Swap(UniqueHandle<Traits>& other) noexcept -> void
	{
		Pointer temp = m_value;
		m_value = other.m_value;
		other.m_value = temp;
	}
};

template < typename Traits >
auto swap(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) -> void
{
	left.Swap(right);
}

template < typename Traits >
auto operator==(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return left.Get() == right.Get();
}

template < typename Traits >
auto operator!=(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return !(left == right);
}

template < typename Traits >
auto operator<(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return left.Get() < right.Get();
}

template < typename Traits >
auto operator>(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return left.Get() > right.Get();
}

template < typename Traits >
auto operator>=(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return left.Get() >= right.Get();
}

template < typename Traits >
auto operator<=(UniqueHandle<Traits>& left, UniqueHandle<Traits>& right) noexcept -> bool
{
	return left.Get() <= right.Get();
}

struct LocalMessageTraits
{
	using Pointer = wchar_t*;
	static auto Invalid() noexcept -> Pointer
	{
		return nullptr;
	}

	static auto Close(Pointer value) noexcept -> void
	{
		ENSURE_(nullptr, LocalFree(value));
	}
};

struct NullHandleTraits
{
	typedef HANDLE Pointer;

	static auto Invalid() noexcept -> Pointer
	{
		return nullptr;
	}

	static auto Close(Pointer value) noexcept -> void
	{
		ENSURE(CloseHandle(value));
	}
};

struct InvalidHandleTraits
{
	typedef HANDLE Pointer;

	static auto Invalid() noexcept -> Pointer
	{
		return INVALID_HANDLE_VALUE;
	}

	static auto Close(Pointer value) noexcept -> void
	{
		ENSURE(CloseHandle(value));
	}
};

struct FindFileHandleTraits
{
	typedef HANDLE Pointer;

	static auto Invalid() noexcept -> Pointer
	{
		return INVALID_HANDLE_VALUE;
	}

	static auto Close(Pointer value) noexcept -> void
	{
		ENSURE(FindClose(value));
	}
};

struct MapViewDeleter
{
	typedef const char* Pointer;
	auto operator()(Pointer value) const noexcept -> void
	{
		ENSURE(UnmapViewOfFile(value));
	}
};

