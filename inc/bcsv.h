#ifndef FILTER_BCSV_H
#define FILTER_BCSV_H

#include <sys/types.h>
#include <vector>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <memory>
#include <functional>


struct EntrySignature final
{
	size_t ROffset;
	size_t MOffset;
	size_t Size;
};

struct ArraySegment final
{
	const char* Ptr;
	const size_t Size;

	ArraySegment(const char* ptr, size_t size);
};

class DataSet final
{
private:
	const char* _data;
	const EntrySignature* _sigv;
    const size_t _sigc;

private:
	DataSet(const char* data, EntrySignature* sigv, size_t sigc);
    
    ~DataSet();

public:
	DataSet(const DataSet&) = delete;

	[[nodiscard]]
	size_t Size() const noexcept;

	[[nodiscard]]
	ArraySegment operator[](size_t i) const noexcept;

public:
	static std::unique_ptr<DataSet, std::function<void(DataSet*)>> Load(const char* src, size_t len);
};


#endif //FILTER_BCSV_H
