#pragma once
#include "StdIncludes.h"
#include "UniqueHandle.h"

class ThreadpoolEnvironment
{
	TP_CALLBACK_ENVIRON m_handle;

public:
	ThreadpoolEnvironment(const ThreadpoolEnvironment&) = delete;
	ThreadpoolEnvironment(ThreadpoolEnvironment&&) = delete;
	ThreadpoolEnvironment& operator=(const ThreadpoolEnvironment&) = delete;
	ThreadpoolEnvironment& operator=(ThreadpoolEnvironment&&) = delete;

	ThreadpoolEnvironment()
	{
		InitializeThreadpoolEnvironment(&m_handle);
	}

	~ThreadpoolEnvironment()
	{
		DestroyThreadpoolEnvironment(&m_handle);
	}

	PTP_CALLBACK_ENVIRON Get()
	{
		return &m_handle;
	}
};

struct PoolTraits : HandleTraits<PTP_POOL>
{
	static Traits Invalid() noexcept
	{
		return nullptr;
	}

	static void Close(Traits value) noexcept
	{
		CloseThreadpool(value);
	}
};

using ThreadpoolHandle = UniqueHandle<PoolTraits>;

class SerializedPool
{
	ThreadpoolHandle m_pool;
	ThreadpoolEnvironment m_environment;

	using StatelessFunction = void(*)();

	static auto __stdcall Callback(PTP_CALLBACK_INSTANCE, void* args) -> void
	{
		auto w = static_cast<StatelessFunction>(args);
		w();
	}

public:
	SerializedPool(int min, int max)
		: m_pool(CreateThreadpool(nullptr))
	{
		SetThreadpoolThreadMinimum(m_pool.Get(), min);
		SetThreadpoolThreadMaximum(m_pool.Get(), max);
		SetThreadpoolCallbackPool(m_environment.Get(), m_pool.Get());
	}

	template <TP_CALLBACK_PRIORITY P = TP_CALLBACK_PRIORITY_NORMAL, typename W>
	void Submit(W work)
	{
		SetThreadpoolCallbackPriority(m_environment.Get(), P);
		if (!TrySubmitThreadpoolCallback(Callback, static_cast<StatelessFunction>(work), m_environment.Get()))
		{
			TRACE(L"Error submitting callback\n");
		}
	}

};

struct WorkTraits : HandleTraits<PTP_WORK>
{
	static Traits Invalid() noexcept
	{
		return nullptr;
	}

	static void Close(Traits value) noexcept
	{
		CloseThreadpoolWork(value);
	}
};

using WorkHandle = UniqueHandle<WorkTraits>;

struct WaitTraits : HandleTraits<PTP_WAIT>
{
	static Traits Invalid() noexcept
	{
		return nullptr;
	}

	static void Close(Traits value)
	{
		CloseThreadpoolWait(value);
	}
};

using WaitHandle = UniqueHandle<WaitTraits>;

class Win32EventObj
{
	UniqueHandle<NullHandleTraits> h;

public:
	enum class Type
	{
		AutoReset,
		ManualReset
	};

	Win32EventObj(const Win32EventObj&) = delete;
	Win32EventObj& operator=(const Win32EventObj&) = delete;
	~Win32EventObj() = default;

	explicit Win32EventObj(Type type)
		: h{ CreateEvent(nullptr, static_cast<BOOL>(type), false, nullptr) }
	{
		ASSERT(h);
	}

	Win32EventObj(Win32EventObj&& other) noexcept
		: h(other.h.Release()) {}
	Win32EventObj& operator=(Win32EventObj&& other) noexcept
	{
		h = std::move(other.h);
		return *this;
	}

	void Set()
	{
		ENSURE(SetEvent(h.Get()));
	}

	void Clear()
	{
		ENSURE(ResetEvent(h.Get()));
	}

	bool Wait(const DWORD milliseconds = INFINITE)
	{
		const auto result = WaitForSingleObject(h.Get(), milliseconds);
		ASSERT(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT);
		return result == WAIT_OBJECT_0;
	}

	auto Get() const noexcept -> HANDLE
	{
		return h.Get();
	}
};


struct TimerTraits : HandleTraits<PTP_TIMER>
{
	static Traits Invalid() noexcept
	{
		return nullptr;
	}

	static void Close(Traits value) noexcept
	{
		CloseThreadpoolTimer(value);
	}
};

using TimerHandle = UniqueHandle<TimerTraits>;

inline FILETIME RelativeTime(uint32_t milliseconds)
{
	union myTime
	{
		INT64 quad;
		FILETIME ft;
	};

	myTime t =
	{
		-static_cast<INT64>(milliseconds) * 10000
	};

	return t.ft;
}

struct CleanupGroupTraits : HandleTraits<PTP_CLEANUP_GROUP>
{
	static Traits Invalid() noexcept
	{
		return nullptr;
	}

	static void Close(Traits value)
	{
		CloseThreadpoolCleanupGroup(value);
	}
};

using CleanupGroupHandle = UniqueHandle<CleanupGroupTraits>;



class Threadpool
{
	ThreadpoolEnvironment m_environment;
	ThreadpoolHandle m_pool;
	CleanupGroupHandle m_cleanupGroup;
public:
	Threadpool(int minThreads)
		: m_pool
		(CreateThreadpool(nullptr))
		, m_cleanupGroup(CreateThreadpoolCleanupGroup())
	{
		InitializeThreadpoolEnvironment(m_environment.Get());
		ASSERT(m_pool);
		ASSERT(m_cleanupGroup);

		// associate callback environment with the thread pool 
		SetThreadpoolCallbackPool(m_environment.Get(), m_pool.Get());
		// and the cleanup group with the callback environment
		SetThreadpoolCallbackCleanupGroup(m_environment.Get(), m_cleanupGroup.Get(), NULL);

		// !!!!!!!!!!!!!!!!!!!!! 
		// Get logical processor count 
		SYSTEM_INFO sys_info;
		GetSystemInfo(&sys_info);
		// Set Thread pool limits 
		//sys_info.dwNumberOfProcessors instead of 1
		SetThreadpoolThreadMaximum(m_pool.Get(), sys_info.dwNumberOfProcessors);
		ENSURE(SetThreadpoolThreadMinimum(m_pool.Get(), minThreads));
	}
	~Threadpool()
	{
		CloseThreadpoolCleanupGroupMembers(m_cleanupGroup.Get(), TRUE, nullptr);
	}

	PTP_WORK SubmitWork(PTP_WORK_CALLBACK cb, void* params)
	{
		auto work = CreateThreadpoolWork(cb, params, m_environment.Get());
		SubmitThreadpoolWork(work);
		return work;
	}

	static void Wait(PTP_WORK work, BOOL cancelPending)
	{
		WaitForThreadpoolWorkCallbacks(work, cancelPending);
	}
};
