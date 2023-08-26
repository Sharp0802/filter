#include "bcsv.h"
#include <csignal>
#include <cstdio>

ArraySegment::ArraySegment(const char* ptr, const size_t size) : Ptr(ptr), Size(size)
{
}

DataSet::DataSet(const char* data, EntrySignature* sigv, size_t sigc) :
    _data(data), _sigv(sigv), _sigc(sigc)
{
}

DataSet::~DataSet()
{
        delete[] _data;
        delete[] _sigv;
}

size_t
DataSet::Size() const noexcept
{
        return _sigc;
}

ArraySegment
DataSet::operator[](size_t i) const noexcept
{
        auto& sig = _sigv[i];
        return {_data + sig.MOffset, sig.Size};
}

std::unique_ptr<DataSet, std::function<void(DataSet*)>>
DataSet::Load(const char* src, size_t len)
{
        size_t          sigc = 0;
        size_t          rn   = 0;
        EntrySignature* sigv;
        char*           buf;
        size_t          ti = 0;

        if (!src) goto EINVAL__;

        for (size_t i = 0; i < len; ++i)
                if (src[i] == '\r' || src[i] == '\n') sigc++;

        sigv = new EntrySignature[sigc];

        for (size_t i = 0; i < len; ++i)
        {
                size_t j;
                size_t size;

                if (src[i] != ',') continue;

                for (j = i + 1; j < len; ++j)
                {
                        if (src[j] == '\r' || src[j] == '\n') break;
                        if (src[j] == ',') goto EINVAL__;
                }

                size       = j - 1 - i;
                sigv[ti++] = {i + 1, rn, size};
                rn += size;
        }

        buf = new char[rn];
        for (size_t i = 0; i < sigc; ++i)
        {
                auto& sig = sigv[i];
                memcpy(buf + sig.MOffset, src + sig.ROffset, sig.Size);
        }

        errno = 0;
        return {
            new DataSet{buf, sigv, sigc},
             [](DataSet* p) { delete p; }
        };

EINVAL__:
        errno = EINVAL;
        return nullptr;
}
