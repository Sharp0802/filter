#include "trie.h"

TRIENode::~TRIENode()
{
        for (auto& i : Children) delete i;
}

void
TRIENode::Record(const char* data, size_t size)
{
        if (size == 0)
        {
                Recorded = true;
                return;
        }

        auto& place = Children[data[0] - '!'];
        if (!place)
        {
                place = new TRIENode();
        }

        place->Record(data + 1, size - 1);
}

bool
TRIENode::Contains(const char* data, size_t size)
{
        if (size == 0) return Recorded;

        auto& rec = Children[data[0] - '!'];
        if (!rec) return false;

        return rec->Contains(data + 1, size - 1);
}

TRIE::TRIE(TRIENode* root) : _root(root)
{
}

TRIE::~TRIE()
{
        delete _root;
}

std::unique_ptr<TRIE, std::function<void(TRIE*)>>
TRIE::Compile(const DataSet& data)
{
        auto* root = new TRIENode();
        for (size_t i = 0; i < data.Size(); ++i)
        {
                auto rec = data[i];
                root->Record(rec.Ptr, rec.Size);
        }
        return {new TRIE(root), [](TRIE* p) { delete p; }};
}

bool
TRIE::Contains(const char* data, size_t size) const noexcept
{
        return _root->Contains(data, size);
}
