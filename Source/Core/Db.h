#pragma once
#include <sqlite3.h>
#include "UniqueHandle.h"
#include "DebugTools.h"

#define SQL(text)  #text

namespace Db
{
	enum class Type
	{
		Integer = SQLITE_INTEGER,
		Float = SQLITE_FLOAT,
		Blob = SQLITE_BLOB,
		Text = SQLITE_TEXT,
		Null = SQLITE_NULL
	};

	inline std::string GetLastError(sqlite3* const connection)
	{
		return std::string(sqlite3_errmsg(connection));
	}

	class Connection
	{
	public:
		struct ConnectionHandleTraits : HandleTraits<sqlite3*>
		{
			static auto Invalid() noexcept -> Traits
			{
				return nullptr;
			}

			static auto Close(sqlite3* value) noexcept -> void
			{
				ENSURE_(SQLITE_OK, sqlite3_close(value));
			}
		};

		using ConnectionHandle = UniqueHandle<ConnectionHandleTraits>;

	private:
		template <typename F, typename C>
		void _Open(F open, const C* const filename)
		{
			Connection temp;

			if (open(filename, temp.m_handle.Set()) != SQLITE_OK)
			{
				ExitWithError(GetLastError(temp.GetAbi()));
			}

			swap(m_handle, temp.m_handle);
		}
		ConnectionHandle m_handle;

	public:

		Connection() noexcept = default;
		template <typename C>
		explicit Connection(const C* const filename)
		{
			Open(filename);
		}

		static Connection Memory()
		{
			return Connection(":memory:");
		}

		static Connection WideMemory()
		{
			return Connection(L":memory:");
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_handle);
		}

		void Open(const char* const filename)
		{
			_Open(sqlite3_open, filename);
		}
		void Open(const wchar_t* const filename)
		{
			_Open(sqlite3_open16, filename);
		}

		sqlite3* GetAbi() const noexcept
		{
			return m_handle.Get();
		}

		long long RowId() const noexcept
		{
			return sqlite3_last_insert_rowid(GetAbi());
		}

		template <typename F>
		void Profile(F callback, void* const context = nullptr)
		{
			sqlite3_profile(GetAbi(), callback, context);
		}

		template <typename C, typename ... Values>
		void Execute(const C* const text, Values&& ... values)
		{
			Statement(*this, text, std::forward<Values>(values) ...).Execute();
		}

	};

	class Backup
	{
		struct BackupHandleTraits : HandleTraits<sqlite3_backup*>
		{

			static void Close(Traits value) noexcept
			{
				sqlite3_backup_finish(value);
			}
		};

		using BackupHandle = UniqueHandle<BackupHandleTraits>;
		BackupHandle m_handle;
		const Connection* m_destination = nullptr;

	public:
		Backup(const Connection& dest, const Connection& src,
			const char* const destName = "main", const char* const srcName = "main")
			: m_handle(sqlite3_backup_init(dest.GetAbi(), destName, src.GetAbi(), srcName))
			, m_destination(&dest)
		{
			if (!m_handle)
			{
				ExitWithError(GetLastError(dest.GetAbi()));
			}
		}

		sqlite3_backup* GetAbi() const noexcept
		{
			return m_handle.Get();
		}

		bool Step(const int pages = -1)
		{
			const int result = sqlite3_backup_step(GetAbi(), pages);

			if (result == SQLITE_OK)
				return true;
			if (result == SQLITE_DONE)
				return false;

			m_handle.Reset();
			ExitWithError(GetLastError(m_destination->GetAbi()));
			return false;
		}

	};

	template <typename T>
	struct Reader
	{
		const void* GetBlob(int& blob_size, const int column = 0) noexcept
		{
			blob_size = sqlite3_column_bytes(static_cast<const T*>(this)->GetAbi(), column);
			return sqlite3_column_blob(static_cast<const T*>(this)->GetAbi(), column);
		}

		int GetInt(const int column = 0) const noexcept
		{
			return sqlite3_column_int(static_cast<const T*>(this)->GetAbi(), column);
		}

		const char* GetString(const int column = 0) const noexcept
		{
			return reinterpret_cast<const char*>(
				sqlite3_column_text(static_cast<const T*>(this)->GetAbi(), column));
		}

		const wchar_t* GetWideString(const int column = 0) const noexcept
		{
			return static_cast<const wchar_t*>(sqlite3_column_text16(
				static_cast<const T*>(this)->GetAbi(), column));
		}

		int GetStringLength(const int column = 0) const noexcept
		{
			return sqlite3_column_bytes(static_cast<const T*>(this)->GetAbi(), column);
		}

		int GetWideStringLength(const int column = 0) const noexcept
		{
			return sqlite3_column_bytes16(static_cast<const T*>(this)->GetAbi(), column) / sizeof(wchar_t);
		}

		Type GetType(const int column = 0) const noexcept
		{
			return static_cast<Type>(sqlite3_column_type(static_cast<const T*>(this)->GetAbi(), column));
		}
	};

	class Row : public Reader<Row>
	{
		sqlite3_stmt* m_statement = nullptr;

	public:
		sqlite3_stmt* GetAbi() const noexcept
		{
			return m_statement;
		}
		Row(sqlite3_stmt* const statement)
			: m_statement(statement)
		{
		}


	};

	class Statement : public Reader<Statement>
	{
		struct StatementHandleTraits : HandleTraits<sqlite3_stmt*>
		{
			static void Close(Traits value) noexcept
			{
				sqlite3_finalize(value);
			}
		};

		using StatementHandle = UniqueHandle<StatementHandleTraits>;
		StatementHandle m_handle;

		template <typename F, typename C, typename ... Values>
		void _Prepare(const Connection& connection, F prepare, const C* const text, Values&& ... values)
		{
			ASSERT(connection);
			if (prepare(connection.GetAbi(), text, -1, m_handle.Set(), nullptr) != SQLITE_OK)
			{
				ExitWithError(GetLastError(connection.GetAbi()));
			}

			BindAll(std::forward<Values>(values)...);
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		void _Bind(int) const noexcept {}

		template <typename First, typename ... Rest>
		void _Bind(const int index, First&& first, Rest&& ... rest) const
		{
			Bind(index, std::forward<First>(first));
			_Bind(index + 1, std::forward<Rest>(rest) ...);

		}

	public:

		Statement() noexcept = default;

		template <typename C, typename ... Values>
		Statement(const Connection& connection, const C* const text, Values&& ... values)
		{
			Prepare(connection, text, std::forward<Values>(values) ...);
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_handle);
		}

		sqlite3_stmt* GetAbi() const noexcept
		{
			return m_handle.Get();
		}

		template<typename ... Values>
		void Prepare(const Connection& connection, const char* const text, Values&& ... values)
		{
			_Prepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values) ...);
		}

		template<typename ... Values>
		void Prepare(const Connection& connection, const wchar_t* const text, Values&& ... values)
		{
			_Prepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values) ...);
		}

		bool Step() const
		{
			const int result = sqlite3_step(GetAbi());

			if (result == SQLITE_ROW)
				return true;
			if (result == SQLITE_DONE)
				return false;

			ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			return false;
		}

		void Execute() const
		{
			ENSURE(!Step());
		}

		void Bind(const int index, const int value) const
		{
			if (sqlite3_bind_int(GetAbi(), index, value) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		void Bind(const int index, const void* value, const int size) const
		{
			if (sqlite3_bind_blob(GetAbi(), index, value, size, SQLITE_STATIC) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		void Bind(const int index, const char* const value, const int size = -1) const
		{
			if (sqlite3_bind_text(GetAbi(), index, value, size, SQLITE_STATIC) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		void Bind(const int index, const wchar_t* const value, const int size = -1) const
		{
			if (sqlite3_bind_text16(GetAbi(), index, value, size, SQLITE_STATIC) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		void Bind(const int index, const std::string& value) const
		{
			Bind(index, value.c_str(), value.size());
		}

		void Bind(const int index, const std::wstring& value) const
		{
			Bind(index, value.c_str(), value.size() * sizeof(wchar_t));
		}

		void Bind(const int index, std::string&& value) const
		{
			if (sqlite3_bind_text(GetAbi(), index, value.c_str(), value.size(), SQLITE_TRANSIENT) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		void Bind(const int index, std::wstring&& value) const
		{
			if (sqlite3_bind_text16(GetAbi(), index, value.c_str(), value.size() * sizeof(wchar_t), SQLITE_TRANSIENT) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}
		}

		template<typename ... Values>
		void BindAll(Values&& ... values) const
		{
			_Bind(1, std::forward<Values>(values) ...);
		}

		template <typename ... Values>
		void Reset(Values&& ... values) const
		{
			if (sqlite3_reset(GetAbi()) != SQLITE_OK)
			{
				ExitWithError(GetLastError(sqlite3_db_handle(GetAbi())));
			}

			BindAll(values ...);
		}
	};

	class RowIterator
	{
		const Statement* m_statement = nullptr;

	public:
		RowIterator() noexcept = default;
		RowIterator(const Statement& statement) noexcept
		{
			if (statement.Step())
			{
				m_statement = &statement;
			}
		}

		RowIterator& operator++() noexcept
		{
			if (!m_statement->Step())
			{
				m_statement = nullptr;
			}
			return *this;
		}

		bool operator !=(const RowIterator& other) const noexcept
		{
			return m_statement != other.m_statement;
		}

		Row operator *() const noexcept
		{
			return Row(m_statement->GetAbi());
		}
	};

	inline RowIterator begin(const Statement& statement) noexcept
	{
		return RowIterator(statement);
	}

	inline RowIterator end(const Statement& statement) noexcept
	{
		return RowIterator();
	}

}


