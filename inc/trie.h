#ifndef FILTER_TRIE_H
#define FILTER_TRIE_H

#include "bcsv.h"

struct TRIENode final
{
        bool      Recorded;
        TRIENode* Children[94];
        
        ~TRIENode();
        
        void Record(const char* data, size_t size);
        bool Contains(const char* data, size_t size);
};

class TRIE final
{
      private:
        TRIENode* _root;

      public:
        TRIE(TRIENode* root);

        ~TRIE();

      public:
        static std::unique_ptr<TRIE, std::function<void(TRIE*)>>
        Compile(const DataSet& data);

        bool
        Contains(const char* data, size_t size) const noexcept;
};

#endif // FILTER_TRIE_H
