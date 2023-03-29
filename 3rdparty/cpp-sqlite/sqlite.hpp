/**
    Copyright (C) 2023 Toni Lipponen

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
 */

#pragma once
#include <sqlite3.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

#if __cplusplus > 201402L
    #define CPP_SQLITE_NODISCARD [[nodiscard]]
#else
    #define CPP_SQLITE_NODISCARD
#endif

#if defined(CPP_SQLITE_NOTHROW)
    #define CPP_SQLITE_THROW(...) return false
#else
    #define CPP_SQLITE_THROW(...) throw sqlite::Error(__VA_ARGS__)
#endif

namespace sqlite
{
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const char* message, int errorCode = SQLITE_ERROR)
        : std::runtime_error(message), m_errorCode(errorCode)
        {

        }

        explicit Error(const std::string& message, int errorCode = SQLITE_ERROR)
        : std::runtime_error(message), m_errorCode(errorCode)
        {

        }

        CPP_SQLITE_NODISCARD
        int GetCode() const
        {
            return m_errorCode;
        }

    private:
        int m_errorCode;
    };

    namespace Priv
    {
        inline bool CheckError(sqlite3* db, int code)
        {
            if(code != SQLITE_OK && code != SQLITE_DONE)
            {
                const int extendedCode = sqlite3_extended_errcode(db);
                std::string errstr = sqlite3_errstr(extendedCode);
                std::string errmsg = sqlite3_errmsg(db);

                CPP_SQLITE_THROW(errstr + ": " + errmsg, extendedCode);
            }

            return true;
        }

        inline bool CheckError(int code)
        {
            if(code != SQLITE_OK && code != SQLITE_DONE)
            {
                std::string errstr = std::string("SQL error: ") + sqlite3_errstr(code);
                CPP_SQLITE_THROW(errstr, code);
            }

            return true;
        }
    }

    class Connection
    {
    public:
        Connection() : m_connection(nullptr) {}

        explicit Connection(const std::string& filename)
        {
            this->Open(filename);
        }

        Connection(const Connection&) = delete;

        Connection(Connection&& other) noexcept
        {
            this->m_connection = other.m_connection;
            other.m_connection = nullptr;
        }

        virtual ~Connection() noexcept
        {
            try
            {
                this->Close();
            }
            catch(...)
            {

            }
        }

        Connection& operator=(const Connection&) = delete;

        Connection& operator=(Connection&& other) noexcept
        {
            if(&other != this)
            {
                this->m_connection = other.m_connection;
                other.m_connection = nullptr;
            }

            return *this;
        }

        bool Open(const std::string& filename)
        {
            return sqlite::Priv::CheckError(sqlite3_open(filename.data(), &m_connection));
        }

        bool Close()
        {
            const auto result = Priv::CheckError(sqlite3_close(m_connection));
            m_connection = nullptr;

            return result;
        }

        CPP_SQLITE_NODISCARD
        int GetExtendedResult() const
        {
            return sqlite3_extended_errcode(m_connection);
        }

        CPP_SQLITE_NODISCARD
        sqlite3* GetPtr()
        {
            return m_connection;
        }

    private:
        sqlite3* m_connection = nullptr;
    };

    class Blob
    {
    public:
        Blob(const void* data, int32_t bytes)
        {
            m_data.resize(bytes);
            std::memcpy(&m_data.at(0), data, bytes);
        }

        explicit Blob(std::vector<unsigned char> data)
        : m_data(std::move(data))
        {

        }

        CPP_SQLITE_NODISCARD
        uint32_t GetSize() const
        {
            return m_data.size();
        }

        CPP_SQLITE_NODISCARD
        unsigned char* GetData()
        {
            return m_data.data();
        }

        CPP_SQLITE_NODISCARD
        const unsigned char* GetData() const
        {
            return m_data.data();
        }

    private:
        std::vector<unsigned char> m_data;
    };

    /** Non-owning blob*/
    class NOBlob
    {
    public:
        NOBlob(const void* ptr, uint32_t bytes)
        : m_ptr(ptr), m_bytes(bytes)
        {

        }

        CPP_SQLITE_NODISCARD
        uint32_t GetSize() const
        {
            return m_bytes;
        }

        const void* GetData()
        {
            return m_ptr;
        }

        CPP_SQLITE_NODISCARD
        const void* GetData() const
        {
            return m_ptr;
        }

    private:
        const void* m_ptr;
        uint32_t m_bytes;
    };

    namespace Priv
    {
        inline void Append(sqlite3_stmt* statement, int index, const int32_t& data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_int(statement, index, data));
        }

        inline void Append(sqlite3_stmt* statement, int index, const int64_t& data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_int64(statement, index, data));
        }

        inline void Append(sqlite3_stmt* statement, int index, const float& data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_double(statement, index, static_cast<double>(data)));
        }

        inline void Append(sqlite3_stmt* statement, int index, const double& data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_double(statement, index, data));
        }

        inline void Append(sqlite3_stmt* statement, int index, const std::string& data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_text(statement, index, data.data(), static_cast<int>(data.size()), nullptr));
        }

        inline void Append(sqlite3_stmt* statement, int index, const char* data)
        {
            sqlite::Priv::CheckError(sqlite3_bind_text(statement, index, data, static_cast<int>(std::strlen(data)), nullptr));
        }

        inline void Append(sqlite3_stmt* statement, int index, const sqlite::Blob& blob)
        {
            sqlite::Priv::CheckError(sqlite3_bind_blob(statement, index, blob.GetData(), static_cast<int>(blob.GetSize()), nullptr));
        }

        inline void Append(sqlite3_stmt* statement, int index, const sqlite::NOBlob& blob)
        {
            sqlite::Priv::CheckError(sqlite3_bind_blob(statement, index, blob.GetData(), static_cast<int>(blob.GetSize()), nullptr));
        }

        template<typename Arg>
        inline void AppendToQuery(sqlite3_stmt* statement, int index, const Arg& arg)
        {
            sqlite::Priv::Append(statement, index, arg);
        }

        template<typename First, typename ... Args>
        inline void AppendToQuery(sqlite3_stmt* statement, int index, const First& first, const Args&... args)
        {
            sqlite::Priv::Append(statement, index, first);
            sqlite::Priv::AppendToQuery(statement, ++index, args...);
        }

        struct Statement
        {
            Statement() : handle(nullptr) {}

            Statement(sqlite::Connection& connection, const std::string& command)
            {
                auto* db = connection.GetPtr();

                const int code = sqlite3_prepare_v2(
                        db,
                        command.data(),
                        static_cast<int>(command.size()),
                        &handle,
                        nullptr);

                Priv::CheckError(db, code);
            }

            Statement(Statement&& other) noexcept
            {
                std::swap(handle, other.handle);
            }

            ~Statement()
            {
                sqlite::Priv::CheckError(sqlite3_finalize(handle));
            }

            Statement& operator=(Statement&& other) noexcept
            {
                handle = other.handle;
                other.handle = nullptr;

                return *this;
            }

            CPP_SQLITE_NODISCARD
            bool Advance() const
            {
                const int code = sqlite3_step(handle);

                if(code == SQLITE_ROW)
                {
                    return true;
                }

                sqlite::Priv::CheckError(code);
                Reset();

                return false;
            }

            bool Reset() const
            {
                return sqlite::Priv::CheckError(sqlite3_reset(handle));
            }

            CPP_SQLITE_NODISCARD
            int ColumnCount() const
            {
                Reset();

                if(!Advance())
                {
                    return 0;
                }

                const int count = sqlite3_column_count(handle);
                Reset();

                return count;
            }

            CPP_SQLITE_NODISCARD
            std::string GetColumnName(int columnIndex) const
            {
                Reset();

                if(!Advance())
                {
#ifndef CPP_SQLITE_NOTHROW
                    throw sqlite::Error("SQL error: invalid column index");
#endif
                }

                std::string name = sqlite3_column_name(handle, columnIndex);

                if(name.empty())
                {
#ifndef CPP_SQLITE_NOTHROW
                    throw sqlite::Error("SQL error: failed to get column name at index " + std::to_string(columnIndex));
#endif
                }

                Reset();

                return name;
            }

            template<typename T>
            CPP_SQLITE_NODISCARD
            T Get(int) const
            {
                static_assert(sizeof(T) == -1, "SQL error: invalid column data type");
            }

            sqlite3_stmt* handle = nullptr;
        };

        template<>
        inline float Statement::Get(int col) const
        {
            return static_cast<float>(sqlite3_column_double(handle, col));
        }

        template<>
        inline double Statement::Get(int col) const
        {
            return sqlite3_column_double(handle, col);
        }

        template<>
        inline int32_t Statement::Get(int col) const
        {
            return sqlite3_column_int(handle, col);
        }

        template<>
        inline int64_t Statement::Get(int col) const
        {
            return sqlite3_column_int64(handle, col);
        }

        template<>
        inline std::string Statement::Get(int col) const
        {
            const unsigned char* bytes = sqlite3_column_text(handle, col);
            const int size = sqlite3_column_bytes(handle, col);

            if(size == 0)
            {
                return "";
            }

            return {reinterpret_cast<const char*>(bytes), static_cast<std::string::size_type>(size)};
        }

        template<>
        inline sqlite::Blob Statement::Get(int col) const
        {
            const void* bytes = sqlite3_column_blob(handle, col);
            const int size = sqlite3_column_bytes(handle, col);

            return {bytes, size};
        }
    }

    class Type
    {
    private:
        Type(const sqlite::Priv::Statement& statement, int col)
        : m_statement(statement), m_columnIndex(col)
        {

        }
    public:
        template<typename T>
        operator T() const
        {
            return m_statement.Get<T>(m_columnIndex);
        }

        friend class Result;

    private:
        const sqlite::Priv::Statement& m_statement;
        const int m_columnIndex;
    };

    class Result
    {
        explicit Result(sqlite::Priv::Statement&& statement)
        : m_statement(std::move(statement))
        {

        }

    public:
        Result() = default;

        Result(Result&& other) noexcept
        {
            m_statement = std::move(other.m_statement);
        }

        Result& operator=(Result&& other) noexcept
        {
            m_statement = std::move(other.m_statement);

            return *this;
        }

        CPP_SQLITE_NODISCARD
        bool HasData() const
        {
            return ColumnCount() > 0;
        }

        CPP_SQLITE_NODISCARD
        int ColumnCount() const
        {
            return m_statement.ColumnCount();
        }

        bool Reset() const
        {
            return m_statement.Reset();
        }

        CPP_SQLITE_NODISCARD
        bool Next() const
        {
            return m_statement.Advance();
        }

        CPP_SQLITE_NODISCARD
        Type Get(int columnIndex) const
        {
            return {m_statement, columnIndex};
        }

        CPP_SQLITE_NODISCARD
        std::string GetColumnName(int columnIndex) const
        {
            return m_statement.GetColumnName(columnIndex);
        }

        friend void Statement(sqlite::Connection&, const std::string&);

        template<typename First, typename ... Args>
        friend Result Query(sqlite::Connection& connection, const std::string& command, const First& first, const Args... args);
        friend Result Query(sqlite::Connection& connection, const std::string& command);

    private:
        sqlite::Priv::Statement m_statement;
    };

    template<typename First, typename ... Args>
    inline void Statement(sqlite::Connection& connection, const std::string& command, const First& first, const Args... args)
    {
        sqlite::Priv::Statement statement(connection, command);
        sqlite::Priv::AppendToQuery<First, Args...>(statement.handle, 1, first, args...);

        (void)statement.Advance();
    }

    inline void Statement(sqlite::Connection& connection, const std::string& command)
    {
        sqlite::Priv::Statement statement(connection, command);

        (void)statement.Advance();
    }

    template<typename First, typename ... Args>
    CPP_SQLITE_NODISCARD
    inline Result Query(sqlite::Connection& connection, const std::string& command, const First& first, const Args... args)
    {
        sqlite::Priv::Statement statement(connection, command);
        sqlite::Priv::AppendToQuery<First, Args...>(statement.handle, 1, first, args...);

        return Result(std::move(statement));
    }

    CPP_SQLITE_NODISCARD
    inline Result Query(sqlite::Connection& connection, const std::string& command)
    {
        sqlite::Priv::Statement statement(connection, command);

        return Result(std::move(statement));
    }

    inline bool Backup(sqlite::Connection& from, sqlite::Connection& to)
    {
        sqlite3_backup* backup = sqlite3_backup_init(to.GetPtr(), "main", from.GetPtr(), "main");

        if(!backup)
        {
            CPP_SQLITE_THROW("SQL error: failed to initialize backup");
        }

        if(!Priv::CheckError(sqlite3_backup_step(backup, -1)))
        {
            return false;
        }

        if(!Priv::CheckError(sqlite3_backup_finish(backup)))
        {
            return false;
        }

        return true;
    }

    inline bool Backup(sqlite::Connection& from, const std::string& filename)
    {
        sqlite::Connection to(filename);

        return sqlite::Backup(from, to);
    }
}
